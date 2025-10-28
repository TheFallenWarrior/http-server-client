#ifndef SRV_HANDLECLIENT_H
#define SRV_HANDLECLIENT_H

#include <stdint.h>
typedef struct {
	int sockfd;
	uint8_t listDir;
	char * baseDir;
} ThreadArgs;

ThreadArgs *createThreadArgs(uint8_t listDir, char *baseDir);
void freeThreadArgs(ThreadArgs *targ);
void *handleClient(void *arg);

#endif