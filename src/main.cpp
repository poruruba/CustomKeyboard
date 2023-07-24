#include <Arduino.h>
#include <SPIFFS.h>
#include <M5Unified.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>
#include "Esp32BleKeyboard.h"

#include "endpoint_packet.h"
#include "file_utils.h"
#include "wifi_utils.h"
#include "main_conf.h"

#define BLE_PASSKEY 123456

#define LED_PIN 10
#define IR_SEND_PIN  32
#define IR_RECV_PIN  33

IRsend irsend(IR_SEND_PIN);
IRrecv irrecv(IR_RECV_PIN);
decode_results results;
uint16_t *p_ir_send_data = NULL;
int ir_send_data_length = 0;
bool irStartReceive = false;
bool irReceiving = false;
bool irReceived = false;
bool irStartSend = false;
static bool nowBleConnected = false;

void ir_dump(decode_results *results);

enum OPERATION_MODE {
  MODE_HID = 0,
  MODE_EDIT
};
enum OPERATION_MODE mode = MODE_HID;

static JsonObject find_ir_value(uint64_t ir_value){
  long ret = file_load_json(DATA_JSON_FNAME);
  if( ret != 0 )
    return JsonVariant();

  JsonArray list = jsonDoc.as<JsonArray>();
  for( int i = 0 ; i < list.size() ; i++ ){
    JsonObject item = list[i];
    uint32_t item_ir_value = item["ir_value"];
    if( item_ir_value == ir_value )
      return item;
  }

  return JsonVariant();
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Lcd.setFont(&fonts::lgfxJapanGothic_12);
  M5.Lcd.setRotation(1);
  M5.Lcd.setTextSize(2);
  
  Serial.println("setup start");
  M5.Lcd.clear();
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("Setup starting");

  long ret = file_init();
  if( ret != 0 ){
    Serial.println("file_init failed");
    while(1);
  }

  if( !SPIFFS.exists(DATA_JSON_FNAME) ){
    File fp = SPIFFS.open(DATA_JSON_FNAME, FILE_WRITE);
    if( !fp ){
        Serial.printf("SPIFFS.open failed\n");
        while(1);
    }else{
      fp.print("[]");
      fp.close();
    }
  }

  pinMode(LED_PIN, OUTPUT);

  if( M5.BtnA.isPressed() ){
    mode = MODE_EDIT;
    Serial.println("MODE_EDIT");
    digitalWrite(LED_PIN, LOW);

    wifi_try_connect(true);

    long ret = packet_initialize();
    if( ret != 0 ){
      Serial.println("packet_initialize failed");
      while(1);
    }
  }else{
    mode = MODE_HID;
    Serial.println("MODE_HID");
    digitalWrite(LED_PIN, HIGH);

    long ret = file_load_json(DATA_JSON_FNAME);
    if( ret != 0 ){
      Serial.println("file_load_json failed");
      while(1);
    }
    irsend.begin();
    EBK_begin();
  }

  irrecv.enableIRIn();

  Serial.println("setup finished");
  M5.Lcd.clear();
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("Setup Finished");

  if( mode == MODE_EDIT ){
    uint32_t ipaddress = get_ip_address();
    M5.Lcd.clear();
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("IP:%d.%d.%d.%d\n", (ipaddress >> 24) & 0xff, (ipaddress >> 16) & 0xff, (ipaddress >> 8) & 0xff, (ipaddress >> 0) & 0xff);
  }
}

void process_edit(void){
  static unsigned long start_tim;

  if( irStartReceive ){
    irStartReceive = false;
    irReceived = false;
    irReceiving = true;
    start_tim = millis();
    irrecv.resume();
  }else if( irReceiving ){
    if (irrecv.decode(&results)) {
      Serial.println("Received");
      irReceived = true;
      ir_dump(&results);
      irReceiving = false;
    }else{
      unsigned long end_tim = millis();
      if( end_tim - start_tim >= 10000){
        irReceiving = false;
        Serial.println("ir timeout");
      }
    }
  }else if( irStartSend ){
    irStartSend = false;
    irsend.sendRaw(p_ir_send_data, ir_send_data_length, 38);
    delay(100);
    irrecv.resume();
  }
}

void process_hid(void){
  if (irrecv.decode(&results)) {
    Serial.println("irReceived");

    if( results.decode_type == decode_type_t::NEC && !results.repeat){
      Serial.printf("value=%d\n", results.value);
      JsonObject obj = find_ir_value(results.value);
      if( !obj.isNull() ){
        const char *p_key_type = obj["key_type"];
        const char *p_key_name = obj["key_name"];
        if( strcmp(p_key_type, "text") == 0 ){
          M5.Lcd.clear();
          M5.Lcd.setCursor(0, 0);
          M5.Lcd.printf("%s Pushed\n", p_key_name);
          const char *p_text = obj["text"];
          uint8_t key_lang = obj["key_lang"];
          Serial.printf("text(%d)=%s\n", key_lang, p_text);
          EBK_sendString(p_text, key_lang);
          delay(100);
        }else
        if( strcmp(p_key_type, "key") == 0 ){
          M5.Lcd.clear();
          M5.Lcd.setCursor(0, 0);
          M5.Lcd.printf("%s Pushed\n", p_key_name);
          uint8_t code = obj["code"];
          uint8_t mod = obj["mod"];
          Serial.printf("mod=%d code=%d\n", mod, code);
          EBK_sendKeys(mod, &code, 1);
          delay(100);
        }else
        if( strcmp(p_key_type, "key_mac") == 0 ){
          M5.Lcd.clear();
          M5.Lcd.setCursor(0, 0);
          M5.Lcd.printf("%s Pushed\n", p_key_name);
          uint8_t code = obj["code"];
          uint8_t mod = obj["mod"];
          Serial.printf("mod=%d code=%d\n", mod, code);
          if( (mod & EBK_KEY_MODIFIER_CAPS_LOCK) && (mod & EBK_KEY_MODIFIER_TAB) ){
            uint8_t codes[3] = { 0x39 /* CapsLock */, 0x2b /* TAB */, code };
            EBK_sendSequenceKeys(EBK_KEY_MODIFIER_MASK_TRIM(mod), codes, sizeof(codes));
          }else
          if( mod & EBK_KEY_MODIFIER_TAB ){
            uint8_t codes[2] = { 0x2b /* TAB */, code };
            EBK_sendSequenceKeys(EBK_KEY_MODIFIER_MASK_TRIM(mod), codes, sizeof(codes));
          }else
          if( mod & EBK_KEY_MODIFIER_CAPS_LOCK ){
            uint8_t codes[2] = { 0x39 /* CapsLock */, code };
            EBK_sendSequenceKeys(EBK_KEY_MODIFIER_MASK_TRIM(mod), codes, sizeof(codes));
          }else{
            EBK_sendKeys(EBK_KEY_MODIFIER_MASK_TRIM(mod), &code, 1);
          }
          delay(100);
        }else{
          Serial.println("Unkown key_type");
        }
      }else{
        Serial.println("Not found");
      }
    }else{
      Serial.println("unknown decode_type_t");
    }
    irrecv.resume();
  }
}

uint16_t current_index = 0;

void loop() {
  M5.update();

  if( mode == MODE_EDIT ){

    process_edit();

  }else if( mode == MODE_HID ){
    if(EBK_isConnected) {
      if( !nowBleConnected ){
        nowBleConnected = true;
        Serial.println("Ble Connected");
        M5.Lcd.clear();
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.println("Ble Connected");
        irrecv.resume();
      }

      process_hid();

    }else{
      if( nowBleConnected ){
        nowBleConnected = false;
        Serial.println("Ble Disonnected");
        M5.Lcd.clear();
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.println("Ble Disconnected");
        irrecv.pause();
      }
    }      

    if( M5.BtnB.wasPressed() ){
      M5.Lcd.clear();
      M5.Lcd.setCursor(0, 0);
      JsonArray list = jsonDoc.as<JsonArray>();
      if( list.size() > 0 ){
        current_index++;
        if( current_index >= list.size() )
          current_index = 0;
        JsonObject item = list[current_index];
        const char *p_ir_name = item["ir_name"];
        const char *p_key_name = item["key_name"];
        M5.Lcd.println(p_ir_name);
        if( p_key_name != NULL )
          M5.Lcd.println(p_key_name);
      }
    }
  }

  if( M5.BtnA.pressedFor(2000) ){
    Serial.println("Now Rebooting");
    digitalWrite(LED_PIN, HIGH);
    M5.Lcd.clear();
    M5.Lcd.setCursor(0, 0);    
    M5.Lcd.println("Now Rebooting");
    delay(2000);
    ESP.restart();
  }

  delay(1);
}

void ir_dump(decode_results *results) {
  // Dumps out the decode_results structure.
  // Call this after IRrecv::decode()
  uint16_t count = results->rawlen;
  if (results->decode_type == UNKNOWN) {
    Serial.print("Unknown encoding: ");
  } else if (results->decode_type == NEC) {
    Serial.print("Decoded NEC: ");
  } else if (results->decode_type == SONY) {
    Serial.print("Decoded SONY: ");
  } else if (results->decode_type == RC5) {
    Serial.print("Decoded RC5: ");
  } else if (results->decode_type == RC5X) {
    Serial.print("Decoded RC5X: ");
  } else if (results->decode_type == RC6) {
    Serial.print("Decoded RC6: ");
  } else if (results->decode_type == RCMM) {
    Serial.print("Decoded RCMM: ");
  } else if (results->decode_type == PANASONIC) {
    Serial.print("Decoded PANASONIC - Address: ");
    Serial.print(results->address, HEX);
    Serial.print(" Value: ");
  } else if (results->decode_type == LG) {
    Serial.print("Decoded LG: ");
  } else if (results->decode_type == JVC) {
    Serial.print("Decoded JVC: ");
  } else if (results->decode_type == AIWA_RC_T501) {
    Serial.print("Decoded AIWA RC T501: ");
  } else if (results->decode_type == WHYNTER) {
    Serial.print("Decoded Whynter: ");
  } else if (results->decode_type == NIKAI) {
    Serial.print("Decoded Nikai: ");
  }
  serialPrintUint64(results->value, 16);
  Serial.print(" (");
  Serial.print(results->bits, DEC);
  Serial.println(" bits)");
  Serial.print("Raw (");
  Serial.print(count, DEC);
  Serial.print("): {");

  for (uint16_t i = 1; i < count; i++) {
    if (i % 100 == 0)
      yield();  // Preemptive yield every 100th entry to feed the WDT.
    if (i & 1) {
      Serial.print(results->rawbuf[i] * kRawTick, DEC);
    } else {
      Serial.print(", ");
      Serial.print((uint32_t) results->rawbuf[i] * kRawTick, DEC);
    }
  }
  Serial.println("};");
}
