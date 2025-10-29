#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "dirindex.h"
#include "handleclient.h"
#include "parsereq.h"
#include "servefile.h"

#define BUFMAX 4096

ThreadArgs *createThreadArgs(uint8_t listDir, char *baseDir){
	if(!baseDir){
		fprintf(stderr, "Error: invalid baseDir.\n");
		return NULL;
	}
	ThreadArgs *targ = malloc(sizeof(ThreadArgs));
	if(!targ){
		perror("malloc failed");
		return NULL;
	}

	targ->baseDir = malloc(1+strlen(baseDir));
	if(!targ->baseDir){
		perror("malloc failed");
		free(targ);
		return NULL;
	}
	strcpy(targ->baseDir, baseDir);
	targ->baseDir[strlen(baseDir)] = '\x00';

	targ->listDir = listDir;
	return targ;
}

void freeThreadArgs(ThreadArgs *targ){
	if(!targ) return;
	free(targ->baseDir);
	free(targ);
}

void httpErrorCode(int socket, uint16_t code){
	const char *const codes[] = {
		"400 Bad Request",
		"403 Forbidden",
		"404 Not Found",
		"500 Internal Server Error",
		"501 Not Implemented"
	};
	const char *tmp;
	switch(code){
		case 400:
		tmp = codes[0];
		break;

		case 403:
		tmp = codes[1];
		break;

		case 404:
		tmp = codes[2];
		break;

		case 500:
		tmp = codes[3];
		break;

		case 501:
		tmp = codes[4];
		break;

		default:
		tmp = codes[3];
	}

	char buffer[BUFMAX];
	snprintf(buffer, BUFMAX,
		"HTTP/1.1 %s\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: %ld\r\n"
		"\r\n"
		"<!DOCTYPE html><body><h1>%s</h1></body>",
		tmp,
		37+strlen(tmp),
		tmp
	);

	write(socket, buffer, strlen(buffer));
	close(socket);
}

// Função de thread para lidar com cada cliente
void *handleClient(void *arg){
	ThreadArgs *targ = arg;

	char request[BUFMAX];

	ssize_t nread, totalNread = 0;

	// Ler requisição do socket
	while(1){
		nread = read(targ->sockfd, request+totalNread, BUFMAX-totalNread);

		totalNread += nread;
		if(nread <= 0 || totalNread >= BUFMAX) break;

		request[totalNread] = '\x00';
		if(strstr(request, "\r\n\r\n")) break;
	};

	ParsedRequest *preq = createParsedRequest();
	if(!preq){
		httpErrorCode(targ->sockfd, 500);
		freeThreadArgs(targ);
		freeParsedRequest(preq);
		pthread_exit(NULL);
	}
	int x = parseRequest(request, preq);
	if(x > 0){
		switch(x){
			case 1: // Requisição malformada
			httpErrorCode(targ->sockfd, 400);
			break;

			case 2: // Método não suportado
			httpErrorCode(targ->sockfd, 501);
			break;

			case 3: // Falha na análise da requisição
			httpErrorCode(targ->sockfd, 500);
		}
		freeThreadArgs(targ);
		pthread_exit(NULL);
	}

	char filename[BUFMAX];
	snprintf(filename, BUFMAX, "%s/%s", targ->baseDir, preq->path);

	if(strstr(preq->path, "..")){
		fprintf(stderr, "Possible traversal attack: client tried to access %s.\n", filename);
		httpErrorCode(targ->sockfd, 404);
		freeThreadArgs(targ);
		freeParsedRequest(preq);
		pthread_exit(NULL);
	}

	struct stat statbuf;
	if(stat(filename, &statbuf) == -1){
		perror(filename);
		httpErrorCode(targ->sockfd, 404);
		freeThreadArgs(targ);
		freeParsedRequest(preq);
		pthread_exit(NULL);
	} else if(S_ISDIR(statbuf.st_mode)){
		if(serveDirIndex(targ->sockfd, targ->baseDir, preq, targ->listDir) == 2){
			httpErrorCode(targ->sockfd, 403);
		}

		freeThreadArgs(targ);
		freeParsedRequest(preq);
		pthread_exit(NULL);
	}

	serveFile(targ->sockfd, filename);

	freeThreadArgs(targ);
	freeParsedRequest(preq);
	pthread_exit(NULL);
}