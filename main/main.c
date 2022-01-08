#include<string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/ledc.h"


#include "esp_log.h"
#include "./ssd1306.h"
#include "./font8x8_basic.h"


#define CONFIG_INTERFACE  "I2C_INTERFACE"
#define CONFIG_PANEL  "SSD1306_128x6"

#define CONFIG_FLIP  false
#define CONFIG_SDA_GPIO 21
#define CONFIG_SCL_GPIO 22
#define CONFIG_RESET_GPIO -1
#define CONFIG_I2C_INTERFACE true
#define CONFIG_SSD1306_128x64 true
/*CONFIG_MOSI_GPIO
CONFIG_SCLK_GPIO
CONFIG_CS_GPIO
CONFIG_DC_GPIO*/

#define LED_PIN 2
#define ANLOG_PIN 4
#define tag "OLED-TEST"
void oled_led();
void oled_i2c_init();
void log_status(esp_err_t status ,char *tagname, char *success_msg, char*error_msg);

void task_blink(void *params);
void task_analog_measurements(void *params);
void task_display_data(void *params);
void task_pwm_led(void *params);
// OLED Variable
SSD1306_t dev;

//FeeRtos variable
SemaphoreHandle_t  bin_sem;
int adc_reading ;
esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}

void app_main(void)
{
	oled_i2c_init();
	gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
	// Init analog
    gpio_set_direction(ANLOG_PIN, GPIO_MODE_INPUT);

    bin_sem = xSemaphoreCreateBinary();

    xTaskCreate(task_blink,"Blink",512, NULL, 15, NULL);
    xTaskCreate(task_analog_measurements,"Analog",2048, NULL, 7, NULL);
    xTaskCreate(task_display_data,"Display",1023, NULL, 15, NULL);
    xTaskCreate(task_pwm_led,"Led PWM",1023, NULL, 15, NULL);
}

/*@brief Blink task**/
void task_blink(void *params){

   while(1){
       gpio_set_level(LED_PIN, 1);
       vTaskDelay(1000/portTICK_PERIOD_MS);
       gpio_set_level(LED_PIN, 0);
       vTaskDelay(1000/portTICK_PERIOD_MS);

   }
}

esp_err_t status ;
void task_analog_measurements(void *params){
	status = adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
	status = adc1_config_width(ADC_WIDTH_BIT_12);


  while(1){
     vTaskDelay(1000/portTICK_PERIOD_MS);
     adc_reading = adc1_get_raw(ADC1_CHANNEL_6);
     printf("Value : %d \n\r", adc_reading);
     xSemaphoreGive(bin_sem);

  }

}


void task_pwm_led(void *params){
    ledc_timer_config_t tconf ={
       .timer_num = LEDC_TIMER_0,
	   .duty_resolution = LEDC_TIMER_12_BIT,
	   .freq_hz = 1000 ,
	   .speed_mode = LEDC_HIGH_SPEED_MODE,
    };
    ledc_timer_config(&tconf);
    ledc_channel_config_t ledc_channel_conf ={
        .channel = LEDC_CHANNEL_0,
		.duty = 100,
		.gpio_num = 4,
		.intr_type = LEDC_INTR_DISABLE,
		.timer_sel = LEDC_TIMER_0,
		.speed_mode = LEDC_HIGH_SPEED_MODE
    };


   ledc_channel_config(&ledc_channel_conf);
   int num = 0 ;

	while(1){

		ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0,num);
		ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
		num+=50 ;
		if (num>=4095){
			num = 0 ;
		}

		vTaskDelay(25/portTICK_PERIOD_MS);
	}
}

void task_display_data(void *params){

	while(1){
		xSemaphoreTake(bin_sem, portMAX_DELAY);
        char msg[20];
        memset(msg, 0, sizeof(msg));
        sprintf(msg, "ADC : %d",adc_reading);
		ssd1306_display_text(&dev, 3, msg, sizeof(msg), false);



	}
}

void oled_i2c_init(){

     int center=0, top=0, bottom=0;
	 char lineChar[20];

	 ESP_LOGI(tag, "INTERFACE is i2c");
	 ESP_LOGI(tag, "CONFIG_SDA_GPIO=%d",CONFIG_SDA_GPIO);
	 ESP_LOGI(tag, "CONFIG_SCL_GPIO=%d",CONFIG_SCL_GPIO);
	 ESP_LOGI(tag, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
	 i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);

     dev._flip = false;
     ESP_LOGW(tag, "No flip");

		ESP_LOGI(tag, "Panel is 128x64");
	    ssd1306_init(&dev, 128, 64);

	    ssd1306_clear_screen(&dev, false);
	    ssd1306_contrast(&dev, 0xff);

        char title[] = "Analog Reading";
	    ssd1306_display_text(&dev, 1, title, sizeof(title), false);



}












void oled_led(){

	    SSD1306_t dev;
		int center=0, top=0, bottom=0;
		char lineChar[20];

	#if CONFIG_I2C_INTERFACE
		ESP_LOGI(tag, "INTERFACE is i2c");
		ESP_LOGI(tag, "CONFIG_SDA_GPIO=%d",CONFIG_SDA_GPIO);
		ESP_LOGI(tag, "CONFIG_SCL_GPIO=%d",CONFIG_SCL_GPIO);
		ESP_LOGI(tag, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
		i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
	#endif // CONFIG_I2C_INTERFACE

	#if CONFIG_SPI_INTERFACE
		ESP_LOGI(tag, "INTERFACE is SPI");
		ESP_LOGI(tag, "CONFIG_MOSI_GPIO=%d",CONFIG_MOSI_GPIO);
		ESP_LOGI(tag, "CONFIG_SCLK_GPIO=%d",CONFIG_SCLK_GPIO);
		ESP_LOGI(tag, "CONFIG_CS_GPIO=%d",CONFIG_CS_GPIO);
		ESP_LOGI(tag, "CONFIG_DC_GPIO=%d",CONFIG_DC_GPIO);
		ESP_LOGI(tag, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
		spi_master_init(&dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO);
	#endif // CONFIG_SPI_INTERFACE

	#if CONFIG_FLIP
		dev._flip = true;
		ESP_LOGW(tag, "Flip upside down");
	#endif

	#if CONFIG_SSD1306_128x64
		ESP_LOGI(tag, "Panel is 128x64");
		ssd1306_init(&dev, 128, 64);
	#endif // CONFIG_SSD1306_128x64
	#if CONFIG_SSD1306_128x32
		ESP_LOGI(tag, "Panel is 128x32");
		ssd1306_init(&dev, 128, 32);
	#endif // CONFIG_SSD1306_128x32

		ssd1306_clear_screen(&dev, false);
		ssd1306_contrast(&dev, 0xff);

	#if CONFIG_SSD1306_128x64
		top = 2;
		center = 3;
		bottom = 8;
		ssd1306_display_text(&dev, 0, "SSD1306 128x64", 14, false);
		ssd1306_display_text(&dev, 1, "ABCDEFGHIJKLMNOP", 16, false);
		ssd1306_display_text(&dev, 2, "abcdefghijklmnop",16, false);
		ssd1306_display_text(&dev, 3, "Hello World!!", 13, false);
		ssd1306_clear_line(&dev, 4, true);
		ssd1306_clear_line(&dev, 5, true);
		ssd1306_clear_line(&dev, 6, true);
		ssd1306_clear_line(&dev, 7, true);
		ssd1306_display_text(&dev, 4, "SSD1306 128x64", 14, true);
		ssd1306_display_text(&dev, 5, "ABCDEFGHIJKLMNOP", 16, true);
		ssd1306_display_text(&dev, 6, "abcdefghijklmnop",16, true);
		ssd1306_display_text(&dev, 7, "Hello World!!", 13, true);
	#endif // CONFIG_SSD1306_128x64

	#if CONFIG_SSD1306_128x32
		top = 1;
		center = 1;
		bottom = 4;
		ssd1306_display_text(&dev, 0, "SSD1306 128x32", 14, false);
		ssd1306_display_text(&dev, 1, "Hello World!!", 13, false);
		ssd1306_clear_line(&dev, 2, true);
		ssd1306_clear_line(&dev, 3, true);
		ssd1306_display_text(&dev, 2, "SSD1306 128x32", 14, true);
		ssd1306_display_text(&dev, 3, "Hello World!!", 13, true);
	#endif // CONFIG_SSD1306_128x32
		vTaskDelay(3000 / portTICK_PERIOD_MS);

		// Display Count Down
		uint8_t image[24];
		memset(image, 0, sizeof(image));
		ssd1306_display_image(&dev, top, (6*8-1), image, sizeof(image));
		ssd1306_display_image(&dev, top+1, (6*8-1), image, sizeof(image));
		ssd1306_display_image(&dev, top+2, (6*8-1), image, sizeof(image));
		for(int font=0x39;font>0x30;font--) {
			memset(image, 0, sizeof(image));
			ssd1306_display_image(&dev, top+1, (7*8-1), image, 8);
			memcpy(image, font8x8_basic_tr[font], 8);
			if (dev._flip) ssd1306_flip(image, 8);
			ssd1306_display_image(&dev, top+1, (7*8-1), image, 8);
			vTaskDelay(1000 / portTICK_PERIOD_MS);
		}

		// Scroll Up
		ssd1306_clear_screen(&dev, false);
		ssd1306_contrast(&dev, 0xff);
		ssd1306_display_text(&dev, 0, "---Scroll  UP---", 16, true);
		//ssd1306_software_scroll(&dev, 7, 1);
		ssd1306_software_scroll(&dev, (dev._pages - 1), 1);
		for (int line=0;line<bottom+10;line++) {
			lineChar[0] = 0x01;
			sprintf(&lineChar[1], " Line %02d", line);
			ssd1306_scroll_text(&dev, lineChar, strlen(lineChar), false);
			vTaskDelay(500 / portTICK_PERIOD_MS);
		}
		vTaskDelay(3000 / portTICK_PERIOD_MS);

		// Scroll Down
		ssd1306_clear_screen(&dev, false);
		ssd1306_contrast(&dev, 0xff);
		ssd1306_display_text(&dev, 0, "--Scroll  DOWN--", 16, true);
		//ssd1306_software_scroll(&dev, 1, 7);
		ssd1306_software_scroll(&dev, 1, (dev._pages - 1) );
		for (int line=0;line<bottom+10;line++) {
			lineChar[0] = 0x02;
			sprintf(&lineChar[1], " Line %02d", line);
			ssd1306_scroll_text(&dev, lineChar, strlen(lineChar), false);
			vTaskDelay(500 / portTICK_PERIOD_MS);
		}
		vTaskDelay(3000 / portTICK_PERIOD_MS);

		// Page Down
		ssd1306_clear_screen(&dev, false);
		ssd1306_contrast(&dev, 0xff);
		ssd1306_display_text(&dev, 0, "---Page	DOWN---", 16, true);
		ssd1306_software_scroll(&dev, 1, (dev._pages-1) );
		for (int line=0;line<bottom+10;line++) {
			//if ( (line % 7) == 0) ssd1306_scroll_clear(&dev);
			if ( (line % (dev._pages-1)) == 0) ssd1306_scroll_clear(&dev);
			lineChar[0] = 0x02;
			sprintf(&lineChar[1], " Line %02d", line);
			ssd1306_scroll_text(&dev, lineChar, strlen(lineChar), false);
			vTaskDelay(500 / portTICK_PERIOD_MS);
		}
		vTaskDelay(3000 / portTICK_PERIOD_MS);

		// Horizontal Scroll
		ssd1306_clear_screen(&dev, false);
		ssd1306_contrast(&dev, 0xff);
		ssd1306_display_text(&dev, center, "Horizontal", 10, false);
		ssd1306_hardware_scroll(&dev, SCROLL_RIGHT);
		vTaskDelay(5000 / portTICK_PERIOD_MS);
		ssd1306_hardware_scroll(&dev, SCROLL_LEFT);
		vTaskDelay(5000 / portTICK_PERIOD_MS);
		ssd1306_hardware_scroll(&dev, SCROLL_STOP);

		// Vertical Scroll
		ssd1306_clear_screen(&dev, false);
		ssd1306_contrast(&dev, 0xff);
		ssd1306_display_text(&dev, center, "Ousseni", 8, false);
		ssd1306_hardware_scroll(&dev, SCROLL_DOWN);
		vTaskDelay(5000 / portTICK_PERIOD_MS);
		ssd1306_hardware_scroll(&dev, SCROLL_UP);
		vTaskDelay(5000 / portTICK_PERIOD_MS);
		ssd1306_hardware_scroll(&dev, SCROLL_STOP);

		// Invert
		ssd1306_clear_screen(&dev, true);
		ssd1306_contrast(&dev, 0xff);
		ssd1306_display_text(&dev, center, "  Good Bye!!", 12, true);
		vTaskDelay(5000 / portTICK_PERIOD_MS);


		// Fade Out
		ssd1306_fadeout(&dev);

	#if 0
		// Fade Out
		for(int contrast=0xff;contrast>0;contrast=contrast-0x20) {
			ssd1306_contrast(&dev, contrast);
			vTaskDelay(40);
		}
	#endif

		// Restart module
		esp_restart();

}

void log_status(esp_err_t status ,char *tagname, char *success_msg, char*error_msg){
 if(status==ESP_OK){
	 printf(" \033[0;32m");
	 printf(" %s : %s ",tagname, success_msg);
 }else{
	 printf("\033[0;31m");
	 printf(" %s : %s ",tagname, error_msg);
 }
 printf("\033[0m");
}
