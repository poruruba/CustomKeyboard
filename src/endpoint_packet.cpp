#include <Arduino.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <unordered_map>

#include "endpoint_types.h"
#include "endpoint_packet.h"
#include "endpoint_irhid.h"

#define PACKET_JSON_DOCUMENT_SIZE  DEFAULT_BUFFER_SIZE

static AsyncWebServer server(HTTP_PORT);
static std::unordered_map<std::string, EndpointEntry*> endpoint_list;
bool nowRebooting = false;

void packet_appendEntry(EndpointEntry *tables, int num_of_entry)
{
  for(int i = 0 ; i < num_of_entry ; i++ )
    endpoint_list[tables[i].name] = &tables[i];
}

long packet_execute(const char *endpoint, JsonObject &params, JsonObject &responseResult)
{
  std::unordered_map<std::string, EndpointEntry*>::iterator itr = endpoint_list.find(endpoint);
  if( itr != endpoint_list.end() ){
//    Serial.printf("endpoint:%s called\n", endpoint);
    EndpointEntry *entry = itr->second;
    long ret = entry->impl(params, responseResult, entry->magic);
    return ret;
  }

  Serial.println("endpoint not found");
  return -1;
}

static void notFound(AsyncWebServerRequest *request)
{
  if (request->method() == HTTP_OPTIONS){
    request->send(200);
  }else{
    request->send(404);
  }
}

long packet_initialize(void)
{
  packet_appendEntry(irhid_table, num_of_irhid_entry);

  AsyncCallbackJsonWebHandler *handler = new AsyncCallbackJsonWebHandler("/endpoint", [](AsyncWebServerRequest *request, JsonVariant &json) {
    JsonObject jsonObj = json.as<JsonObject>();
    const char *endpoint = jsonObj["endpoint"];
    AsyncJsonResponse *response = new AsyncJsonResponse(false, PACKET_JSON_DOCUMENT_SIZE);
    JsonObject responseResult = response->getRoot();
    responseResult["status"] = "OK";
    responseResult["endpoint"] = (char*)endpoint;
    JsonObject params = jsonObj["params"];
    long ret = packet_execute(endpoint, params, responseResult);
    if( ret != 0 ){
      responseResult.clear();
      responseResult["status"] = "NG";
      responseResult["endpoint"] = (char*)endpoint;
      responseResult["message"] = "unknown";
    }
    response->setLength();
    request->send(response);

    if( nowRebooting ){
      delay(2000);
      ESP.restart();
    }
  });
  server.addHandler(handler);

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "*");
  server.serveStatic("/", SPIFFS, "/html/").setDefaultFile("index.html");
  server.onNotFound(notFound);
  server.begin();

  return 0;
}
