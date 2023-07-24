#ifndef _ENDPOINT_PACKET_H_
#define _ENDPOINT_PACKET_H_

#include <ArduinoJson.h>
#include "endpoint_types.h"

#define HTTP_PORT 80
#define DEFAULT_BUFFER_SIZE 4096

extern bool nowRebooting;

long packet_initialize(void);
void packet_appendEntry(EndpointEntry *tables, int num_of_entry);
long packet_execute(const char *endpoint, JsonObject &params, JsonObject &responseResult);

#endif
