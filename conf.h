#ifndef CONF_H
#define CONF_H

#include "indigo_bus.h"

#define CONFIG_FILENAME ".indigo_panel.conf"

typedef struct {
	bool blobs_enabled;
	bool auto_connect;
	bool indigo_use_host_suffix;
	indigo_log_levels indigo_log_level;
	char unused[1000];
} conf_t;

extern conf_t conf;

#endif // CONF_H
