#include <arpa/inet.h>
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "httpget.h"
#include "parseurl.h"

uint8_t outFlag = 0;
char outFilename[BUFMAX];
int16_t maxRedirs = 10;

// Busca case-insensitive por um campo no cabeçalho HTTP
char *findHeaderField(char *header, char *headerEnd, char *fieldName){
	while(header < headerEnd){
		char *lineEnd = memchr(header, '\n', headerEnd - header);
		if(!lineEnd) break;

		int len = strlen(fieldName);
		if(
			lineEnd - header >= len &&
			strncasecmp(header, fieldName, len) == 0
		){
			char *val = header + len;
			while(val < lineEnd && (*val == ' ' || *val == '\t')) val++;
			return val;
		}
		header = lineEnd + 1;
	}
	return NULL;
}

// Decodificar respostas com chunked encoding, retorna o tamanho do buffer decodificado
ssize_t decodeChunks(char *response, size_t responseLen, char **decoded){
	char *bodyPtr = strstr(response, "\r\n\r\n");
	size_t responseBodyStart = 4 +(bodyPtr - response);

	char *body = response + responseBodyStart;
	size_t bodyLen = responseLen - responseBodyStart;
	ssize_t decodedLen = 0;

	while(bodyLen > 0){
		// Encontrar fim do cabeçalho do chunk
		char *lineEnd = memchr(body, '\n', bodyLen);
		if(!lineEnd || lineEnd == body || *(lineEnd - 1) != '\r'){
			fprintf(stderr, "Error: malformed chunk body: missing CRLF\n");
			free(*decoded);
			return -1;
		}
		// Ler tamanho do chunk em hexadecimal
		*(lineEnd - 1) = '\x00'; // Temporariamente substituir '\r' por '\0'
		char *endptr;
		unsigned long chunkSize = strtoul(body, &endptr, 16);
		*(lineEnd - 1) = '\r'; // Restaurar '\r'

		if(endptr == body || chunkSize == 0) break; // Último chunk

		// Pular cabeçalho do chunk
		size_t lineSize =(lineEnd - body) + 1;
		body += lineSize;
		bodyLen -= lineSize;

		if(bodyLen < chunkSize){
			fprintf(stderr, "Error: malformed chunk body: incomplete chunk\n");
			free(*decoded);
			return -1;
		}

		// Concatenar dados do chunk ao buffer decodificado
		char *tmp = realloc(*decoded, decodedLen + chunkSize);
		if(!tmp){
			perror("realloc failed");
			free(*decoded);
			return -1;
		}
		*decoded = tmp;
		memcpy(*decoded + decodedLen, body, chunkSize);
		decodedLen += chunkSize;

		// Pular dados do chunk e o \r\n no final
		body += chunkSize;
		bodyLen -= chunkSize;
		if(bodyLen < 2 || body[0] != '\r' || body[1] != '\n'){
			fprintf(stderr, "Error: malformed chunk body: missing CRLF after chunk\n");
			free(*decoded);
			return -1;
		}
		body += 2;
		bodyLen -= 2;
	}

	return decodedLen;
}

// Receber um host em IPv4 ou nome de domínio, retorna um struct sockaddr
int resolveHost(char *host, uint16_t port, struct sockaddr *addr){
	struct addrinfo hints, *result;
	if(!host || !addr) return 1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	char portStr[6];
	snprintf(portStr, 6, "%d", port);

	int s = getaddrinfo(host, portStr, &hints, &result);
	if(!s){
		memcpy(addr, result->ai_addr, result->ai_addrlen);
		freeaddrinfo(result);
		return 0;
	}

	struct sockaddr_in servaddr;

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	if(!inet_aton(host, &servaddr.sin_addr)){
		return 1;
	}

	memcpy(addr, &servaddr, sizeof(servaddr));
	return 0;
}

// Fazer requisição HTTP e salvar resposta localmente
int httpGet(int sockfd, const char *host, const char *path, FILE *out, int16_t redirs){
	char request[BUFMAX];
	bzero(request, sizeof(request));

	snprintf(request, sizeof(request),
		"GET %s HTTP/1.1\r\n"
		"Host: %s\r\n" // Hostname do servidor
		"Connection: close\r\n" // Pedir para o servidor encerrar a conexão após enviar a resposta
		"\r\n",
		path, host
	);

	ssize_t sent = write(sockfd, request, strlen(request));
	if(sent < 0){
		perror("write failed");
		return 1;
	} else if((size_t)sent < strlen(request)){
		fprintf(stderr, "Error: partial request write to server: %zd of %zu bytes\n", sent, strlen(request));
		return 1;
	}

	char responseBuf[BUFMAX];
	char *response = malloc(1);
	size_t responseLen = 0;
	if(!response){
		perror("malloc failed");
		return 1;
	}

	// Ler pedaços da resposta em loop até que EOF seja encontrado
	while(1){
		bzero(responseBuf, sizeof(responseBuf));
		ssize_t received = read(sockfd,	responseBuf, sizeof(responseBuf)-1); // Deixar espaço para o \0
		if(received <= 0) break; // EOF ou erro (-1)
		responseBuf[received] = '\x00';

		// Concatenar com o que já foi recebido
		char *tmp = realloc(response, responseLen + received+1);
		if(!tmp){
			perror("realloc failed");
			free(response);
			return 1;
		} response = tmp;

		memcpy(response + responseLen, responseBuf, received);
		responseLen += received;
	}

	char *bodyPtr = strstr(response, "\r\n\r\n");
	if(!bodyPtr){
		fprintf(stderr, "Error: malformed HTTP response.\n");
		free(response);
		return 1;
	}

	// Verificar resposta do servidor
	if(!strncmp(response, "HTTP/1.1 200 OK", 15)){
		size_t responseBodyStart = 4 + (bodyPtr - response);

		// Checar se a tranferência é em chunks
		char *header = response;
		char *headerEnd = bodyPtr;
		char *chunkedPtr = findHeaderField(header, headerEnd, "Transfer-Encoding:");

		int chunked = (chunkedPtr && !strncasecmp(chunkedPtr, "chunked", 7));

		if(chunked){
			char *decoded = malloc(1);
			if(!decoded){
				perror("malloc failed");
				free(response);
				return 1;
			}
			ssize_t decodedLen = decodeChunks(response, responseLen, &decoded);
			if(decodedLen == -1){
				free(response);
				return 1;
			}

			// Escrever body decodificado no arquivo
			size_t written = fwrite(decoded, 1, decodedLen, out);
			free(decoded);
			if((ssize_t)written != decodedLen){
				fprintf(stderr, "Error: partial write to output file: %zu of %zu bytes\n",
						written, decodedLen);
				free(response);
				return 1;
			}
		} else{
			// Escrever body no arquivo
			size_t written = fwrite(response + responseBodyStart, 1, responseLen - responseBodyStart, out);
			if(written != responseLen - responseBodyStart){
				fprintf(stderr,
					"Error: partial write to output file: %zu of %zu bytes\n",
					written, responseLen - responseBodyStart
				);
				free(response);
				return 1;
			}
		}
	} else{
		// Verificar se a resposta é um redirecionamento
		if(!strncmp(response, "HTTP/1.1 3", 10)){
			// Encontrar a URL do redirecionamento
			char *location = findHeaderField(response, bodyPtr, "Location:");
			if(!location){
				fprintf(stderr, "Error: Could not find redirect target.\n");
				fprintf(stderr, "Response:\n%s\n", response);
				free(response);
				return 1;
			}
			char *locationEnd = strstr(location, "\r\n");
			if(!locationEnd){
				fprintf(stderr, "Error: malformed HTTP response.\n");
				free(response);
				return 1;
			}
			*locationEnd = '\x00'; // Terminar string

			// Chamada recursiva para resolver redirecionamento
			int x = httpConnect(location, redirs-1);

			free(response);
			return x;
		}
		fprintf(stderr, "Error: server did not respond with HTTP status 200 OK\n");
		fprintf(stderr, "Response:\n%s\n", response);
		free(response);
		return 1;
	}
	free(response);
	return 0;
}

// Criar socket, conectar com o servidor e fazer download do recurso
int httpConnect(char *url, int16_t redirs){
	if(redirs < 0){
		fprintf(stderr, "Error: redirection limit reached.\n");
		return 1;
	}
	if(redirs != maxRedirs) fprintf(stderr, "Info: redirecting to '%s'\n", url);

	ParsedUrl *purl = createParsedUrl();
	if(parseUrl(url, purl)){
		fprintf(stderr, "Error: failed to parse URL.\n");
		return 1;
	}

	if(!outFlag){
		if(purl->path[strlen(purl->path)-1] == '/'){
			strcpy(outFilename, "index.html");
		} else{
			strncpy(outFilename, 1+strrchr(purl->path, '/'), BUFMAX);
		}
	}

	int sockfd;
	struct sockaddr sa;

	// Criar o socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1){
		fprintf(stderr, "Error: socket creation failed.\n");
		return 1;
	}

	if(resolveHost(purl->host, purl->port, &sa)){
		fprintf(stderr, "Error: could not determine host's IP address.\n");
		freeParsedUrl(purl);
		return 1;
	}

	// Conectar socket
	if(connect(sockfd, &sa, sizeof(sa)) != 0){
		fprintf(stderr, "Error: connection with the server failed.\n");
		freeParsedUrl(purl);
		return 1;
	}

	int x;

	// O campo "Host" no cabeçalho HTTP também contém a porta do servidor
	char hostPort[BUFMAX];
	if(purl->port == 80){
		snprintf(hostPort, BUFMAX, "%s", purl->host);
	} else{
		snprintf(hostPort, BUFMAX, "%s:%d", purl->host, purl->port);
	}

	// Abrir arquivo de saída e fazer a requisição
	if(strcmp(outFilename, "stdout")){
		FILE *f =fopen(outFilename, "wb");
		x = httpGet(sockfd, hostPort, purl->path, f, redirs);
		fclose(f);
	} else{
		x = httpGet(sockfd, hostPort, purl->path, stdout, redirs);
	}

	close(sockfd);
	freeParsedUrl(purl);

	return x;
}