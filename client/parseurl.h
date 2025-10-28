#ifndef CLI_PARSEURL_H
#define CLI_PARSEURL_H

#include <stdint.h>

typedef struct{
	char *host;
	uint16_t port;
	char *path;
} ParsedUrl;

ParsedUrl *createParsedUrl();
void freeParsedUrl(ParsedUrl *purl);
int parseUrl(const char *url, ParsedUrl *purl);

#endif