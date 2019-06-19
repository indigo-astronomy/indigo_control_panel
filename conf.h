#ifndef CONF_H
#define CONF_H

typedef struct {
	bool blobs_enabled;
	bool auto_connect;
} conf_t;

extern conf_t conf;

#endif // CONF_H
