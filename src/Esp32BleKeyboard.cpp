#include <Arduino.h>

#include <BleKeyboard.h>
#include "Esp32BleKeyboard.h"
#include "asciimap.h"

BleKeyboard bleKeyboard("CustomKeyboard");

void EBK_begin(void){
  bleKeyboard.begin();
}

bool EBK_isConnected(void){
  return bleKeyboard.isConnected();
}

void EBK_sendSequenceKeys(uint8_t mod, const uint8_t *keys, uint8_t num_key){
  KeyReport keyReport = { 0 };
  keyReport.modifiers = mod;
  for( int i = 0 ; i < num_key && i < 6; i++ ){
    keyReport.keys[i] = keys[i];
    bleKeyboard.sendReport(&keyReport);
    delay(EBK_KEY_WAIT);
  }
  bleKeyboard.releaseAll();
  delay(EBK_KEY_WAIT);
}

void EBK_sendKeysLong(uint8_t mod, const uint8_t *keys, uint8_t num_key, uint32_t wait_ms){
  KeyReport keyReport = { 0 };
  keyReport.modifiers = mod;
  for( int i = 0 ; i < num_key && i < 6 ; i++ )
    keyReport.keys[i] = keys[i];
  bleKeyboard.sendReport(&keyReport);
  delay(wait_ms);
  bleKeyboard.releaseAll();
  delay(EBK_KEY_WAIT);
}

void EBK_sendKeys(uint8_t mod, const uint8_t *keys, uint8_t num_key){
  KeyReport keyReport = { 0 };
  keyReport.modifiers = mod;
  for( int i = 0 ; i < num_key && i < 6 ; i++ )
    keyReport.keys[i] = keys[i];
  bleKeyboard.sendReport(&keyReport);
  delay(EBK_KEY_WAIT);
  bleKeyboard.releaseAll();
  delay(EBK_KEY_WAIT);
}

void EBK_sendString(const char* ptr, uint8_t lang){
  KeyReport keyReport = { 0 };
  while(*ptr != '\0'){
    if( *ptr >= ASCIIMAP_SIZE ){
      ptr++;
      continue;
    }
    ASCIIMAP map;
    if( lang == EBK_KEYMAP_LANG_JP )
      map = asciimap_jp[(uint8_t)*ptr];
    else
      map = asciimap_us[(uint8_t)*ptr];
    keyReport.modifiers = map.modifier;
    keyReport.keys[0] = map.usage;
    bleKeyboard.sendReport(&keyReport);
    delay(EBK_KEY_WAIT);
    bleKeyboard.releaseAll();
    delay(EBK_KEY_WAIT);
    
    ptr++;
  }
}
