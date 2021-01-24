#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DISPLAYS 2

struct DisplayDriver
{
    bool active;
    uint8_t i2cAddress;
    uint8_t digits[4];
    uint8_t blink;
    uint8_t brightness;
};

typedef struct DisplayDriver DisplayDriver;

extern DisplayDriver displays[MAX_DISPLAYS];

void register_display(void);
extern void updateDisplay(void);


#ifdef __cplusplus
}
#endif
