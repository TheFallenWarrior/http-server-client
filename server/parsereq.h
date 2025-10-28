#ifndef SRV_PARSEREQ_H
#define SRV_PARSEREQ_H

typedef struct{
	char *host;
	char *path;
} ParsedRequest;

ParsedRequest *createParsedRequest();
void freeParsedRequest(ParsedRequest *preq);
int parseRequest(char *req, ParsedRequest *preq);

#endif