/* cmd_display.c
*/
#include <stdio.h>
#include <string.h>
#include "argtable3/argtable3.h"
#include "driver/i2c.h"
#include "esp_console.h"
#include "esp_log.h"
#include "i2c.h"
#include "cmd_display.h"
#include "7segment.c"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

static const char *TAG = "cmd_display";

DisplayDriver displays[MAX_DISPLAYS];

static int do_display_init_cmd(int argc, char **argv)
{
	esp_err_t ret;
	
	for (int index = 0; index < MAX_DISPLAYS; index++)
	{
		int chip_addr = displays[index].i2cAddress;
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();
		
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, chip_addr << 1 | WRITE_BIT, ACK_CHECK_EN);
		i2c_master_write_byte(cmd, 0x21, ACK_CHECK_EN);
		i2c_master_stop(cmd);
		
		ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
		
		i2c_cmd_link_delete(cmd);
		
		cmd = i2c_cmd_link_create();
		
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, chip_addr << 1 | WRITE_BIT, ACK_CHECK_EN);
		for (int i = 0; i < 17 ; i++)
		{
			i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN);
		}
		i2c_master_stop(cmd);
		
		ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
		
		i2c_cmd_link_delete(cmd);

		cmd = i2c_cmd_link_create();
		
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, chip_addr << 1 | WRITE_BIT, ACK_CHECK_EN);
		i2c_master_write_byte(cmd, 0x81, ACK_CHECK_EN);
		i2c_master_stop(cmd);
		
		ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
		
		i2c_cmd_link_delete(cmd);
	}
	
   if (ret == ESP_OK)
	{
       ESP_LOGI(TAG, "Write OK");
   }
	else if (ret == ESP_ERR_TIMEOUT)
	{
       ESP_LOGW(TAG, "Bus is busy");
   }
	else
	{
       ESP_LOGW(TAG, "Write Failed");
   }

	return 0;
}

static void register_display_init(void)
{
    const esp_console_cmd_t display_init_cmd = {
        .command = "dinit",
        .help = "Init display",
        .hint = NULL,
        .func = &do_display_init_cmd,
        .argtable = NULL
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&display_init_cmd));
}

static struct {
	struct arg_str *text;
	struct arg_end *end;
} show_args;

static int do_display_show_cmd(int argc, char **argv)
{
	uint8_t i = 0;

	int nerrors = arg_parse(argc, argv, (void **)&show_args);
	if (nerrors != 0)
	{
		arg_print_errors(stderr, show_args.end, argv[0]);
		return 1;
	}

	if (show_args.text->sval[0])
	{
		for (i = 0; i < MAX_DISPLAYS; i++)
		{
			if (displays[i].active && strlen(show_args.text->sval[0]) > i * 4)
			{
				uint8_t d = 0;
				for (uint8_t c = i * 4; c < MIN((size_t)i * 4 + 4, strlen(show_args.text->sval[0])); c++)
				{
					displays[i].digits[d++] = show_args.text->sval[0][c];
				}
			}
		}
		updateDisplay();
	}
/*	else if (argc == 2)
	{
		if (strncmp(argv[0], "br", 2) == 0)
		{
			for (i = 0; i < MAX_DISPLAYS; i++)
			{
				if (displays[i].active)
				{
					displays[i].brightness = strtol(argv[1], NULL, 10) & 0xF;
				}
			}
			updateDisplay();
		}
	}*/
	else
	{
		for (i = 0; i < MAX_DISPLAYS; i++)
		{
			if (displays[i].active)
			{
				printf("%d: %c%c%c%c\n\r", i, displays[i].digits[0] & 0x7f,
					displays[i].digits[1] & 0x7f,
					displays[i].digits[2] & 0x7f,
					displays[i].digits[3] & 0x7f);
			}
		}
	}


	return 0;
}

static void register_display_show(void)
{
	show_args.text = arg_str1(NULL, NULL, "<text>", "Text to show");
	show_args.end = arg_end(1);

    const esp_console_cmd_t display_show_cmd = {
        .command = "dshow",
        .help = "Write string to display",
        .hint = NULL,
        .func = &do_display_show_cmd,
        .argtable = &show_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&display_show_cmd));
}

void updateDisplay(void)
{
   uint8_t txbuf[17] = {0};
 
	esp_err_t ret;

	for (int index=0; index < MAX_DISPLAYS; index++)
	{
		if (displays[index].active)
		{
			for (int digit=0 ; digit<4 ; digit++)
			{
				uint8_t c = segval[displays[index].digits[digit] & 0x7F] | (displays[index].digits[digit] & 0x80);
				txbuf[0x01 + digit*2] = c;
				txbuf[0x09 + digit*2] = c;
			}

			int chip_addr = displays[index].i2cAddress;
			
			i2c_cmd_handle_t cmd = i2c_cmd_link_create();
			
			i2c_master_start(cmd);
			i2c_master_write_byte(cmd, chip_addr << 1 | WRITE_BIT, ACK_CHECK_EN);
			for (int i = 0; i < 17 ; i++)
			{
				i2c_master_write_byte(cmd, txbuf[i], ACK_CHECK_EN);
			}
			i2c_master_stop(cmd);
			
			ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
			
			i2c_cmd_link_delete(cmd);

			cmd = i2c_cmd_link_create();
			
			i2c_master_start(cmd);
			i2c_master_write_byte(cmd, chip_addr << 1 | WRITE_BIT, ACK_CHECK_EN);
			i2c_master_write_byte(cmd, 0x81 |  (displays[index].blink << 1), ACK_CHECK_EN);
			i2c_master_stop(cmd);
			
			ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
			
			i2c_cmd_link_delete(cmd);

			cmd = i2c_cmd_link_create();
			
			i2c_master_start(cmd);
			i2c_master_write_byte(cmd, chip_addr << 1 | WRITE_BIT, ACK_CHECK_EN);
			i2c_master_write_byte(cmd, 0xE0 | (displays[index].brightness & 0x0F), ACK_CHECK_EN);
			i2c_master_stop(cmd);
			
			ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
			
			i2c_cmd_link_delete(cmd);
		}
	}

	if (ret == ESP_OK)
	{
		ESP_LOGI(TAG, "Write OK");
	}
	else if (ret == ESP_ERR_TIMEOUT)
	{
		ESP_LOGW(TAG, "Bus is busy");
	}
	else
	{
		ESP_LOGW(TAG, "Write Failed");
	}
}

void register_display(void)
{
	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
	i2c_master_driver_initialize();

	register_display_init();
	register_display_show();
	
	displays[0].active = true;
	displays[0].i2cAddress = 0x70;
	displays[0].digits[0] = 0x20;
	displays[0].digits[1] = 0x20;
	displays[0].digits[2] = 0x20;
	displays[0].digits[3] = 0x20;
	displays[0].blink = 0;
	displays[0].brightness = 0x08;

	displays[1].active = true;
	displays[1].i2cAddress = 0x71;
	displays[1].digits[0] = 0x20;
	displays[1].digits[1] = 0x20;
	displays[1].digits[2] = 0x20;
	displays[1].digits[3] = 0x20;
	displays[1].blink = 0;
	displays[1].brightness = 0x08;
}
