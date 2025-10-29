#include <string.h>

#include "mimetype.h"

// Tipos de mídia comuns para formatos Web
const MimeTypeMap mimeTypes[] = {
	// Web
	{".htm",  "text/html"},
	{".html", "text/html"},
	{".css",  "text/css"},
	{".js",   "text/javascript"},
	{".json", "application/json"},
	{".xml",  "application/xml"},

	// Fonte
	{".ttf",  "font/ttf"},
	{".otf",  "font/otf"},
	{".woff", "font/woff"},
	{".woff2","font/woff2"},

	// Texto
	{".pdf",  "application/pdf"},
	{".txt",  "text/plain"},
	{".cfg",  "text/plain"},
	{".conf", "text/plain"},
	{".log",  "text/plain"},
	{".csv",  "application/csv"},
	{".php",  "application/x-httpd-php"},

	// Áudio e vídeo
	{".mp3",  "audio/mpeg"},
	{".ogg",  "audio/ogg"},
	{".oga",  "audio/ogg"},
	{".wav",  "audio/wav"},
	{".weba", "audio/webm"},
	{".mp4",  "video/mp4"},
	{".mpeg", "video/mpeg"},
	{".ogv",  "video/ogg"},
	{".avi",  "video/x-msvideo"},
	{".webm", "video/webm"},

	// Imagem
	{".png",  "image/png"},
	{".jpg",  "image/jpeg"},
	{".jpeg", "image/jpeg"},
	{".webp", "image/webp"},
	{".bmp",  "image/bmp"},
	{".gif",  "image/gif"},
	{".svg",  "image/svg+xml"},
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