#ifndef SRV_MIMETYPE_H
#define SRV_MIMETYPE_H

typedef struct {
	const char *extension;
	const char *type;
}MimeTypeMap;

const char *getMimeType(char *extension);

#endif