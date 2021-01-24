#ifndef _DISPLAY_H
#define _DISPLAY_H

struct DisplayDriver
{
    bool active;
    uint8_t i2cAddress;
    uint8_t digits[4];
    uint8_t blink;
    uint8_t brightness;
};

typedef struct DisplayDriver DisplayDriver;

extern DisplayDriver displays[8];

extern void initDisplay(void);
extern void updateDisplay(void);

#endif
