#ifndef _FILE_UTILS_H_
#define _FILE_UTILS_H_

#include <ArduinoJson.h>

#define JSON_CAPACITY 4096
extern StaticJsonDocument<JSON_CAPACITY> jsonDoc;

#define DUMMY_FNAME "/dummy"
#define CONFIG_FNAME "/config.ini"

long file_save_json(const char *p_fname);
long file_load_json(const char *p_fname);

long file_init(void);
long file_save(const char *p_fname, const char *p_data);
char* file_load(const char *p_fname);
long file_size(const char *p_fname);

long config_read_long(unsigned short index);
long config_write_long(unsigned short index, long value);
String config_read_string(const char *fname);
long config_write_string(const char *fname, const char *text);

unsigned long b64_encode_length(unsigned long input_length);
unsigned long b64_encode(const unsigned char input[], unsigned long input_length, char output[]);
unsigned long b64_decode_length(const char input[]);
unsigned long b64_decode(const char input[], unsigned char output[]);

#endif