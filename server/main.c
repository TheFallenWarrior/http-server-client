#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "handleclient.h"

void printUsage(){
	fprintf(stderr,
		"Usage: http-server [options] <dir>\n"
		"Options:\n"
		"\t-d\t\tDisable directory listing\n"
		"\t-p <port>\tSpecify port to run the server on (default: 8080)\n"
	);
}

int main(int argc, char **argv){
	int sockfd, opt;
	struct sockaddr_in serverAddr;
	uint8_t listDir = 1;
	uint16_t port = 8080;

	if(argc == 1){
		printUsage();
		return 1;
	}

	while((opt = getopt(argc, argv, "dhp:")) != -1){
		switch(opt){
			case 'h':
			printUsage();
			return 0;
			break;

			case 'd':
			listDir = 0;
			break;

			case 'p':
			port = atoi(optarg);
			if(!port){
				fprintf(stderr, "Error: invalid port.\n");
				return 1;
			}
			break;

			case '?':
			printUsage();
			return 1;
		}
	}

	if(optind >= argc){
		printUsage();
		return 1;
	}

	// Create socket
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Socket creation failed");
		return 1;
	}

	// Configurar reutilização da porta
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
		perror("setsockopt(SO_REUSEADDR) failed");

	// Initializar estrutura serverAddr
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(port);

	// Bind
	if(bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0){
		perror("Bind failed");
		close(sockfd);
		return 1;
	}

	// Listen
	if(listen(sockfd, 20) < 0){
		perror("Listen failed");
		close(sockfd);
		return 1;
	}

	printf("Info: Server listening on port %d...\n", port);

	while(1){
		ThreadArgs *targ = createThreadArgs(listDir, argv[optind]);
		if(!targ){
			fprintf(stderr, "Error: could not create ThreadArgs object.\n");
			continue;
		}

		targ->sockfd = accept(sockfd, NULL, NULL);
		if(targ->sockfd < 0){
			perror("accept failed");
			freeThreadArgs(targ);
			continue;
		}

		printf("Info: Client connected — spawning thread\n");

		pthread_t tid;
		if (pthread_create(&tid, NULL, handleClient, targ) != 0){
			perror("pthread_create failed");
			close(targ->sockfd);
			freeThreadArgs(targ);
		}
		else{
			pthread_detach(tid);
		}
	}

	close(sockfd);
	return 0;
}