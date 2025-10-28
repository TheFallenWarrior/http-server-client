#ifndef CLI_HTTPGET_H
#define CLI_HTTPGET_H

#include <stdint.h>

#define BUFMAX 4096

int httpConnect(char *url, int16_t redirs);

extern uint8_t outFlag;
extern char outFilename[];
extern int16_t maxRedirs;

#endif