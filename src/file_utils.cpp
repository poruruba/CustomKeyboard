#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <base64.hpp>

#include "file_utils.h"

StaticJsonDocument<JSON_CAPACITY> jsonDoc;
static char *p_jsonData = NULL;

long file_load_json(const char *p_fname){
  if( p_jsonData != NULL )
    free(p_jsonData);
  p_jsonData = file_load(p_fname);
  if( p_jsonData == NULL )
    return -1;
  jsonDoc.clear();
  DeserializationError er = deserializeJson(jsonDoc, p_jsonData);
  if( er ){
    Serial.printf("deserializeJson Error: %s\n", er.c_str());
    return -1;
  }

  return 0;
}

long file_save_json(const char *p_fname){
  int size = measureJson(jsonDoc);
  char *p_data = (char*)malloc(size + 1);
  if( p_data == NULL )
    return -1;
  int size2 = serializeJson(jsonDoc, p_data, size);
  if( size2 <= 0 ){
    free(p_data);
    return -1;
  }
  p_data[size2] = '\0';
  Serial.println(p_data);
  long ret = file_save(p_fname, p_data);
  if( ret != 0 ){
    free(p_data);
    return -1;
  }
  free(p_data);

  return 0;
}

long file_init(void){
    if( !SPIFFS.begin() )
    Serial.println("SPIFFS begin failed");
  delay(100);

  File fp = SPIFFS.open(DUMMY_FNAME, FILE_READ);
  if( !fp ){
    Serial.println("SPIFFS FORMAT START");
    if( !SPIFFS.format() )
      Serial.println("SPIFFS format failed");
    Serial.println("SPIFFS FORMAT END");

    if( !SPIFFS.begin() )
      Serial.println("SPIFFS begin failed");
    delay(100);

    fp = SPIFFS.open(DUMMY_FNAME, FILE_WRITE);
    if( !fp ){
      Serial.printf("SPIFFS.open failed\n");
      return -1;
    }else{
      fp.print("{}");
      fp.close();
    }
  }else{
    fp.close();
  }

  return 0;
}

long file_save(const char *p_fname, const char *p_data)
{
  File fp = SPIFFS.open(p_fname, FILE_WRITE);
  if( !fp )
    return -1;
  fp.write((uint8_t*)p_data, strlen(p_data));
  fp.close();

  return 0;
}

long file_size(const char *p_fname){
  if( !SPIFFS.exists(p_fname) )
    return -1;
  File fp = SPIFFS.open(p_fname, FILE_READ);
  if( !fp )
    return -1;

  size_t size = fp.size();
  fp.close();

  return size;
}

char* file_load(const char *p_fname)
{
  if( !SPIFFS.exists(p_fname) )
    return NULL;
  File fp = SPIFFS.open(p_fname, FILE_READ);
  if( !fp )
    return NULL;

  size_t size = fp.size();
  char* data = (char*)malloc(size + 1);
  if( data == NULL ){
    fp.close();
    return NULL;
  }
  fp.readBytes(data, size);
  fp.close();
  data[size] = '\0';

  return data;
}

long config_read_long(unsigned short index)
{
  File fp = SPIFFS.open(CONFIG_FNAME, FILE_READ);
  if( !fp )
    return 0;
  
  size_t fsize = fp.size();
  if( fsize < (index + 1) * sizeof(long) )
    return 0;

  bool ret = fp.seek(index * sizeof(long));
  if( !ret ){
    fp.close();
    return 0;
  }

  long value;
  if( fp.read((uint8_t*)&value, sizeof(long)) != sizeof(long) ){
    fp.close();
    return 0;
  }
  fp.close();

  return value;
}

long config_write_long(unsigned short index, long value)
{
  File fp = SPIFFS.open(CONFIG_FNAME, FILE_WRITE);
  if( !fp )
    return -1;
  
  size_t fsize = fp.size();
  const long temp = 0;
  if( fsize < index * sizeof(long) ){
    fp.seek(fsize / sizeof(long) * sizeof(long));
    for( int i = fsize / sizeof(long) ; i < index ; i++ )
      fp.write((uint8_t*)&temp, sizeof(long));
  }

  if( fp.write((uint8_t*)&value, sizeof(long)) != sizeof(long) ){
    fp.close();
    return -1;
  }
  fp.close();

  return 0;
}

String config_read_string(const char *fname)
{
  File fp = SPIFFS.open(fname, FILE_READ);
  if( !fp )
    return String("");

  String text = fp.readString();
  fp.close();

  return text;
}

long config_write_string(const char *fname, const char *text)
{
  File fp = SPIFFS.open(fname, FILE_WRITE);
  if( !fp )
    return -1;

  long ret = fp.write((uint8_t*)text, strlen(text));
  fp.close();
  if( ret != strlen(text) )
    return -1;

  return 0;
}

unsigned long b64_encode_length(unsigned long input_length)
{
  return encode_base64_length(input_length);
}

unsigned long b64_encode(const unsigned char input[], unsigned long input_length, char output[])
{
  return encode_base64((unsigned char*)input, input_length, (unsigned char*)output);
}

unsigned long b64_decode_length(const char input[])
{
  return decode_base64_length((unsigned char*)input);
}

unsigned long b64_decode(const char input[], unsigned char output[])
{
  return decode_base64((unsigned char*)input, output);
}