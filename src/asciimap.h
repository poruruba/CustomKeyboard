#ifndef _ASCIIMAP_H_
#define _ASCIIMAP_H_

typedef struct {
	unsigned char usage;
	unsigned char modifier;
} ASCIIMAP;

enum ASCIIMAP_MODIFIER_MASK{
  ASCIIMAP_MODIFIER_CTRL = 0x01,
  ASCIIMAP_MODIFIER_SHIFT = 0x02,
  ASCIIMAP_MODIFIER_ALT = 0x04,
  ASCIIMAP_MODIFIER_WIN = 0x08
};

#define ASCIIMAP_SIZE (127)
extern const ASCIIMAP asciimap_us[ASCIIMAP_SIZE];
extern const ASCIIMAP asciimap_jp[ASCIIMAP_SIZE];

#endif
