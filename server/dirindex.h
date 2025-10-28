#ifndef SRV_DIRINDEX_H
#define SRV_DIRINDEX_H

#include <stdint.h>

#include "parsereq.h"

int serveDirIndex(int sockfd, char *baseDir, ParsedRequest *preq, uint8_t listDir);

#endif