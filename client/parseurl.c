#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parseurl.h"

ParsedUrl *createParsedUrl(){
	ParsedUrl *purl = malloc(sizeof(ParsedUrl));
	if(!purl){
		perror("malloc failed");
		return NULL;
	};
	purl->host = NULL;
	purl->port = 0;
	purl->path = NULL;

	return purl;
}

void freeParsedUrl(ParsedUrl *purl){
	if(!purl) return;
	free(purl->host);
	free(purl->path);
	free(purl);
}

int parseUrl(const char *url, ParsedUrl *purl){
	uint64_t hostStart = 0, pathStart = 0;
	if(!purl){
		fprintf(stderr, "Error: argument purl is NULL.\n");
		return 1;
	}

	if(strstr(url, "://")){
		if(!strncmp(url, "http://", 7)) hostStart = 7;
		//else if(!strncmp(url, "https://", 8)) hostStart = 8;
		else{
			fprintf(stderr, "Error: unsupported protocol or malformed URL.\n");
			freeParsedUrl(purl);
			return 1;
		}
	}

	if(hostStart == strlen(url)){
		fprintf(stderr, "Error: malformed URL.\n");
			freeParsedUrl(purl);

		return 1;
	}

	char *tmp;
	// Extrair path
	if(!(tmp = strchr(url+hostStart, '/'))){
		purl->path = malloc(2);
		if(!purl->path){
			perror("malloc failed");
			freeParsedUrl(purl);

			return 1;
		}
		purl->path[0] = '/';
		purl->path[1] = '\x00';
		pathStart = strlen(url);
	} else{
		int pathLen = strlen(tmp);
		purl->path = malloc(1+pathLen);
		if(!purl->path){
			perror("malloc failed");
			freeParsedUrl(purl);

			return 1;
		}
		strncpy(purl->path, tmp, pathLen);
		purl->path[pathLen] = '\x00';
		pathStart = tmp-url;
	}

	// Extrair port e host
	if(!(tmp = strchr(url+hostStart, ':'))){
		purl->port = 80;
		purl->host = malloc(1+pathStart-hostStart);
		if(!purl->host){
			perror("malloc failed");
			freeParsedUrl(purl);
			return 1;
		}
		strncpy(purl->host, url+hostStart, pathStart-hostStart);
		purl->host[pathStart-hostStart] = '\x00';
	} else{
		char *portTmp = malloc(1+pathStart-(tmp-url));
		if(!portTmp){
			perror("malloc failed");
			freeParsedUrl(purl);
			return 1;
		}
		strncpy(portTmp, tmp+1, pathStart-(tmp-url)-1);
		portTmp[pathStart-(tmp-url)-1] = '\x00';
		purl->port = atoi(portTmp);
		free(portTmp);
		if(!purl->port){
			fprintf(stderr, "Error: invalid port number.\n");
			freeParsedUrl(purl);
			return  1;
		}

		purl->host = malloc(1+(tmp-url)-hostStart);
		if(!purl->host){
			perror("malloc failed");
			freeParsedUrl(purl);
			return  1;
		}
		strncpy(purl->host, url+hostStart, (tmp-url)-hostStart);
		purl->host[(tmp-url)-hostStart] = '\x00';
	}

	return 0;
}