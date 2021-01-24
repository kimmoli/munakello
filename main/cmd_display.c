/* cmd_i2ctools.c

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "argtable3/argtable3.h"
#include "driver/i2c.h"
#include "esp_console.h"
#include "esp_log.h"
#include "i2c.h"

static const char *TAG = "cmd_display";

static int do_display_init_cmd(int argc, char **argv)
{
   i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
	i2c_master_driver_initialize();
	
	int chip_addr = 0x70;
	
   i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	
   i2c_master_start(cmd);
	i2c_master_write_byte(cmd, chip_addr << 1 | WRITE_BIT, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, 0x21, ACK_CHECK_EN);
	i2c_master_stop(cmd);
	
	esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
	
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
	
   if (ret == ESP_OK)
	{
       ESP_LOGI(TAG, "Write OK");
   } else if (ret == ESP_ERR_TIMEOUT)
	{
       ESP_LOGW(TAG, "Bus is busy");
   } else
	{
       ESP_LOGW(TAG, "Write Failed");
   }
   i2c_driver_delete(I2C_NUM_0);

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


void register_display(void)
{
	register_display_init();
}
