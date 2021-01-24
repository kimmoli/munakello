#include "hal.h"
#include "display.h"
#include "i2c.h"
#include "helpers.h"
#include "7segment.c"

DisplayDriver displays[8];

static void initializeDisplay(DisplayDriver *display)
{
    uint8_t txbuf[17] = {0};

    if (display->active)
    {
        DEBUG("init display\n\r");
        i2cAcquireBus(&I2CD1);

        /* enable oscillator */
        txbuf[0] = 0x21;
        i2cMasterTransmit(&I2CD1, display->i2cAddress, txbuf, 1, NULL, 0);

        /* clear display memory */
        txbuf[0] = 0x00;
        i2cMasterTransmit(&I2CD1, display->i2cAddress, txbuf, 17, NULL, 0);

        /* enable display */
        txbuf[0] = 0x81;
        i2cMasterTransmit(&I2CD1, display->i2cAddress, txbuf, 1, NULL, 0);

        i2cReleaseBus(&I2CD1);
    }
}

static void detectDisplays(void)
{
    msg_t ret;
    uint8_t rxBuf[2];
    uint8_t addr;
    uint8_t index;
    uint8_t instance = 0;

    for (index = 0; index < 8 ; index++)
    {
        displays[index].active = false;
        addr = 0x70 + index;

        i2cAcquireBus(&I2CD1);
        i2cMasterTransmit(&I2CD1, addr, (uint8_t[]){ 0 }, 1, NULL, 0);
        ret = i2cMasterReceive(&I2CD1, addr, rxBuf, 2);
        i2cReleaseBus(&I2CD1);

        if (MSG_OK == ret)
        {
            PRINT("Detected display %d at 0x%02x\n\r", instance, addr);

            displays[instance].active = true;
            displays[instance].i2cAddress = addr;
            displays[instance].digits[0] = 0x20;
            displays[instance].digits[1] = 0x20;
            displays[instance].digits[2] = 0x20;
            displays[instance].digits[3] = 0x20;
            displays[instance].blink = 0;

            initializeDisplay(&displays[instance]);

            instance++;
        }
    }
}

void updateDisplay(void)
{
    uint8_t txbuf[17] = {0};

    for (int index=0; index<8 ; index++)
    {
        if (displays[index].active)
        {
            for (int digit=0 ; digit<4 ; digit++)
            {
                uint8_t c = segval[displays[index].digits[digit] & 0x7F] | (displays[index].digits[digit] & 0x80);
                txbuf[0x01 + digit*2] = c;
                txbuf[0x09 + digit*2] = c;
            }

            i2cAcquireBus(&I2CD1);
            i2cMasterTransmit(&I2CD1, displays[index].i2cAddress,(uint8_t[]){ 0x81 | (displays[index].blink << 1) } , 1, NULL, 0);
            i2cMasterTransmit(&I2CD1, displays[index].i2cAddress,(uint8_t[]){ 0xE0 | (displays[index].brightness & 0x0F) } , 1, NULL, 0);
            i2cMasterTransmit(&I2CD1, displays[index].i2cAddress, txbuf, 17, NULL, 0);
            i2cReleaseBus(&I2CD1);
        }
    }
}

void initDisplay(void)
{
    detectDisplays();
}
