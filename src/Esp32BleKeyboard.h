#ifndef _ESP32_BLE_KEYBOARD_H_
#define _ESP32_BLE_KEYBOARD_H_

#include <Arduino.h>

#define EBK_KEYMAP_LANG_US    0
#define EBK_KEYMAP_LANG_JP    1

#define EBK_KEY_WAIT  10

enum KEY_MODIFIER_MASK {
  EBK_KEY_MODIFIER_CTRL = 0x01,
  EBK_KEY_MODIFIER_SHIFT = 0x02,
  EBK_KEY_MODIFIER_ALT = 0x04,
  EBK_KEY_MODIFIER_WIN = 0x08
};
#define EBK_KEY_MODIFIER_MASK_TRIM(mods) ((mods) & 0x0f)

enum EBK_KEY_MODIFIER_MASK_MAC {
  EBK_KEY_MODIFIER_CAPS_LOCK = 0x10,
  EBK_KEY_MODIFIER_TAB = 0x20
};

void EBK_begin(void);
bool EBK_isConnected(void);
void EBK_sendKeysLong(uint8_t mod, const uint8_t *keys, uint8_t num_key, uint32_t wait_ms);
void EBK_sendKeys(uint8_t mod, const uint8_t *keys, uint8_t num_key);
void EBK_sendSequenceKeys(uint8_t mod, const uint8_t *keys, uint8_t num_key);
void EBK_sendString(const char* ptr, uint8_t lang);

#endif
