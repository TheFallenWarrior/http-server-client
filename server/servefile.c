#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "mimetype.h"

#define BUFMAX 4096

// Servir um arquivo via HTTP
int serveFile(int sockfd, char *filename){
	struct stat statbuf;
	if(stat(filename, &statbuf) == -1){
		perror(filename);
		close(sockfd);
		return 1;
	}

	// Gerar cabeçalho HTTP
	char header[BUFMAX];
	int headerLen = snprintf(header, BUFMAX,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: %s\r\n"
		"Content-Length: %ld\r\n"
		"\r\n",
		getMimeType(strrchr(filename, '.')),
		statbuf.st_size
	);

	ssize_t hwritten = write(sockfd, header,(size_t)headerLen);
	if(hwritten != headerLen){
		// Envio parcial do cabeçalho
		fprintf(stderr, "Error: partial write of HTTP header.\n");
		close(sockfd);
		return 1;
	}

	// Abrir arquivo
	FILE *in = fopen(filename, "rb");
	if(!in){
		perror("fopen");
		close(sockfd);
		return 1;
	}

	// Loop para fazer o streaming do arquivo na rede
	while(1){
		char filebuf[BUFMAX];
		size_t nread = fread(filebuf, 1, BUFMAX, in);
		if(nread == 0){
			if(feof(in)) break;
			if(ferror(in)){
				perror("fread");
				fclose(in);
				close(sockfd);
				return 1;
			}
		}

		// Loop interno para resolver envio parcial do buffer
		size_t totalWritten = 0;
		while(totalWritten < nread){
			ssize_t written = write(sockfd, filebuf + totalWritten, nread - totalWritten);
			if(written == -1){
				perror("write");
				fclose(in);
				close(sockfd);
				return 1;
			}
			totalWritten += (size_t)written;
		}
	}

	fclose(in);
	close(sockfd);
	return 0;
}