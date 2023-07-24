#include <Arduino.h>

#include "endpoint_types.h"
#include "endpoint_irhid.h"
#include "main_conf.h"
#include "file_utils.h"
#include "endpoint_packet.h"

long endp_irhid_list(JsonObject &request, JsonObject &response, int magic)
{
  long ret = file_load_json(DATA_JSON_FNAME);
  if( ret != 0 )
    return ret;

  serializeJson(jsonDoc, Serial);
  response["result"]["ir_list"] = jsonDoc;

  // JsonArray ir_list = response["result"]["ir_list"].to<JsonArray>();
  // JsonArray list = jsonDoc.as<JsonArray>();
  // for( int i = 0 ; i < list.size() ; i++ ){
  //   JsonObject item = list[i];
  //   const char *p_ir_name = item["ir_name"];
  //   ir_list[i]["ir_value"] = (uint64_t)item["ir_value"];
  //   ir_list[i]["ir_name"] = (char*)p_ir_name;
  //   const char *p_key_type = list[i]["key_type"];
  //   if( p_key_type == NULL )
  //     continue;
  //   ir_list[i]["key_type"] = (char*)p_key_type;
  //   const char *p_key_name = list[i]["key_name"];
  //   ir_list[i]["key_name"] = (char*)p_key_name;
  //   uint8_t key_lang = item["key_lang"];
  //   ir_list[i]["key_lang"] = key_lang;
  //   if( strcmp( p_key_type, "text") == 0 ){
  //     const char *p_text = item["text"];
  //     ir_list[i]["text"] = (char*)p_text;
  //   }else if( strcmp(p_key_type, "key") == 0 || strcmp(p_key_type, "key_mac") == 0 ){
  //     ir_list[i]["code"] = (uint8_t)item["code"];
  //     ir_list[i]["mod"] = (uint8_t)item["mod"];
  //   }
  // }

  return 0;
}

long endp_irhid_ir_register(JsonObject &request, JsonObject &response, int magic)
{
  if( irReceiving || !irReceived )
    return -1;
  
  long ret = file_load_json(DATA_JSON_FNAME);
  if( ret != 0 )
    return ret;

  uint64_t ir_value = request["ir_value"];
  JsonArray list = jsonDoc.as<JsonArray>();
  for( int i = 0 ; i < list.size() ; i++ ){
    JsonObject item = list[i];
    uint64_t item_ir_value = item["ir_value"];
    if( item_ir_value == ir_value )
      return -1;
  }

  const char *p_ir_name = request["ir_name"];
  JsonObject obj = list.createNestedObject();
  obj["ir_value"] = ir_value;
  obj["ir_name"] = (char*)p_ir_name;

  ret = file_save_json(DATA_JSON_FNAME);
  if( ret != 0 )
    return ret;

  return 0;
}

long endp_irhid_ir_update(JsonObject& request, JsonObject& response, int magic)
{
  long ret = file_load_json(DATA_JSON_FNAME);
  if( ret != 0 )
    return ret;

  uint64_t ir_value = request["ir_value"];
  JsonArray list = jsonDoc.as<JsonArray>();
  for( int i = 0 ; i < list.size() ; i++ ){
    JsonObject item = list[i];
    uint32_t item_ir_value = item["ir_value"];
    if( item_ir_value == ir_value ){
      const char *p_ir_name = request["ir_name"];
      item["ir_name"] = (char*)p_ir_name;

      ret = file_save_json(DATA_JSON_FNAME);
      if( ret != 0 )
        return ret;
      
      return 0;
    }
  }
  return -1;
}

long endp_irhid_key_update(JsonObject& request, JsonObject& response, int magic)
{
  long ret = file_load_json(DATA_JSON_FNAME);
  if( ret != 0 )
    return ret;

  uint64_t ir_value = request["ir_value"];
  JsonArray list = jsonDoc.as<JsonArray>();
  for( int i = 0 ; i < list.size() ; i++ ){
    JsonObject item = list[i];
    uint64_t item_ir_value = item["ir_value"];
    if( item_ir_value == ir_value ){
      const char *p_key_type = request["key_type"];
      const char *p_key_name = request["key_name"];
      uint8_t key_lang = request["key_lang"];
      item["key_type"] = (char*)p_key_type;
      item["key_name"] = (char*)p_key_name;
      item["key_lang"] = key_lang;
      if( strcmp(p_key_type, "text") == 0 ){
        item.remove("mod");
        item.remove("code");
        const char *p_text = request["text"];
        item["text"] = (char*)p_text;
      }else
      if( strcmp(p_key_type, "key") == 0 || strcmp(p_key_type, "key_mac") == 0 ){
        item.remove("text");
        uint32_t mod = request["mod"];
        uint32_t code = request["code"];
        item["mod"] = mod;
        item["code"] = code;
      }

      ret = file_save_json(DATA_JSON_FNAME);
      if( ret != 0 )
        return ret;
      
      return 0;
    }
  }
  return -1;
}

long endp_irhid_ir_send(JsonObject& request, JsonObject& response, int magic)
{
  const char *p_rawbuf = request["rawbuf"];

  if( p_ir_send_data != NULL ){
    free(p_ir_send_data);
    p_ir_send_data = NULL;
  }

  int buflen = b64_decode_length(p_rawbuf);
  uint8_t *p_buf = (uint8_t*)malloc(buflen);
  if( p_buf == NULL )
    return -1;

  b64_decode(p_rawbuf, p_buf);

  p_ir_send_data = (uint16_t*)p_buf;
  ir_send_data_length = buflen / sizeof(uint16_t);

  irStartSend = true;

  return 0;
}

long endp_irhid_ir_check(JsonObject& request, JsonObject& response, int magic)
{
  irReceived = false;
  irStartReceive = true;

  return 0;
}

long endp_irhid_ir_status(JsonObject& request, JsonObject& response, int magic)
{
  response["result"]["irReceiving"] = irReceiving;
  response["result"]["irReceived"] = irReceived;
  if( irReceiving )
    return 0;

  if( !irReceived )
    return 0;

  long buflen = b64_encode_length(results.rawlen * sizeof(uint16_t));
  char *buf = (char*)malloc(buflen + 1);
  b64_encode((uint8_t*)results.rawbuf, results.rawlen * sizeof(uint16_t), buf);
  Serial.printf("%02x %02x %02x\n", buf[buflen - 2], buf[buflen - 1], buf[buflen]);
  buf[buflen] = '\0';

  response["result"]["decode_type"] = results.decode_type;
  response["result"]["address"] = results.address;
  response["result"]["command"] = results.command;
  response["result"]["value"] = results.value;
  response["result"]["rawbuf"] = buf;

  free(buf);

  return 0;
}

long endp_irhid_ir_delete(JsonObject& request, JsonObject &response, int magic)
{
  long ret = file_load_json(DATA_JSON_FNAME);
  if( ret != 0 )
    return ret;

  uint32_t ir_value = request["ir_value"];
  JsonArray list = jsonDoc.as<JsonArray>();
  int list_size = list.size();
  bool found = false;
  for( int i = 0 ; i < list_size ; i++ ){
    JsonObject item = list[i];
    uint32_t item_ir_value = item["ir_value"];
    if( item_ir_value == ir_value ){
      list.remove(i);

      ret = file_save_json(DATA_JSON_FNAME);
      if( ret != 0 )
        return ret;

      found = true;
      break;
    }
  }
  if( !found )
    return -1;

  return 0;
}

long endp_irhid_reboot(JsonObject& request, JsonObject& response, int magic)
{
  nowRebooting = true;
  return 0;
}

EndpointEntry irhid_table[] = {
  EndpointEntry{ endp_irhid_list, "/irhid-ir-list", -1 },
  EndpointEntry{ endp_irhid_ir_register, "/irhid-ir-register", -1 },
  EndpointEntry{ endp_irhid_ir_update, "/irhid-ir-update", -1 },
  EndpointEntry{ endp_irhid_key_update, "/irhid-key-update", -1 },
  EndpointEntry{ endp_irhid_ir_delete, "/irhid-ir-delete", -1 },
  EndpointEntry{ endp_irhid_ir_check, "/irhid-ir-check", -1 },
  EndpointEntry{ endp_irhid_ir_status, "/irhid-ir-status", -1 },
  EndpointEntry{ endp_irhid_ir_send, "/irhid-ir-send", -1 },
  EndpointEntry{ endp_irhid_reboot, "/irhid-reboot", -1 },
};

const int num_of_irhid_entry = sizeof(irhid_table) / sizeof(EndpointEntry);
