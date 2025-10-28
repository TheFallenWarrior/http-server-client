#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "httpget.h"
#include "parseurl.h"

void printUsage(){
	fprintf(stderr,
		"Usage: http-client [options] <url>\n"
		"Options:\n"
		"\t-r <redirs>\tSets redirect limit to <redirs>\n"
		"\t-o <file>\tWrite to output to <file>\n"
	);
}

int main(int argc, char **argv){
	int opt;

	if(argc == 1){
		printUsage();
		return 1;
	}

	while((opt = getopt(argc, argv, "ho:r:")) != -1){
		switch(opt){
			case 'h':
			printUsage();
			return 0;
			break;

			case 'o':
			outFlag = 1;
			strncpy(outFilename, optarg, BUFMAX);
			break;

			case 'r':
			maxRedirs = atoi(optarg);
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

	return httpConnect(argv[optind], maxRedirs);
}