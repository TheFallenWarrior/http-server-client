#include <string.h>

#include "mimetype.h"

// Tipos de mídia comuns para formatos de texto e imagem
const MimeTypeMap mimeTypes[] = {
	{".htm",  "text/html"},
	{".html", "text/html"},
	{".css",  "text/css"},
	{".js",   "text/javascript"},
	{".json", "application/json"},
	{".xml",  "application/xml"},
	{".txt",  "text/plain"},
	{".png",  "image/png"},
	{".jpg",  "image/jpeg"},
	{".jpeg", "image/jpeg"},
	{".webp", "image/webp"},
	{".bmp",  "image/bmp"},
	{".gif",  "image/gif"},
	{".ico",  "image/vnd.microsoft.icon"}
};

const char *getMimeType(char *extension){
	if(!extension) return "application/octet-stream";

	for(int i=0;i<(int)(sizeof(mimeTypes)/sizeof(MimeTypeMap));i++){
		if(!strcasecmp(extension, mimeTypes[i].extension)){
			return mimeTypes[i].type;
		}
	}
	return "application/octet-stream";
}