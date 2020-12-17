/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "keyPressEmulator.h"
#include "driver/gpio.h"

#include "screenChecker.h"
#include "main.h"

#define TAG "app_main"

#define IR_GPIO_NUM ((gpio_num_t)10) //[todo] change this later

keyPressEmulator PowerKey(18, 0);
SemaphoreHandle_t KeepScreenOn = NULL;

TaskHandle_t MainTaskHandle = nullptr;

void ScreenCtrTask(void *pvParameters)
{
   configASSERT(((uint32_t)pvParameters) == 1);
   while (1)
   {
      if (xSemaphoreTake(KeepScreenOn, pdMS_TO_TICKS(45000)) != pdTRUE) //try 45s if can recieve Semaphore
      {
         //time to close the screen!
         for (int tryNum = 0; tryNum < 25; tryNum++) //try 25 times see if we can close screen nicely :)
         {
            if (ScreenChecker::SCStatus.isTurnedOn)
            {
               PowerKey.doPress();
               vTaskDelay(pdMS_TO_TICKS(500));  //wait 500ms before nextCheck
            }
            else
            {
               break;
            }
            if (tryNum == 24)
            ESP_LOGW(TAG,"Failed to Close the Screen!!!");
         }
      }
      else
      {
         //try to open the monitor!
         for (int tryNum = 0; tryNum < 25; tryNum++) //try 25 times see if we can open screen nicely :)
         {
            if (!ScreenChecker::SCStatus.isTurnedOn)
            {
               if (!ScreenChecker::SCStatus.isPowered){
                  ESP_LOGI(TAG,"Screen is not powered!");
                  //[todo] Remind User to turn the power on
               } else
               {
                  PowerKey.doPress();
                  vTaskDelay(pdMS_TO_TICKS(500));  //wait 500ms before nextCheck
               }
               
               
            }
            else
            {
               break;
            }
            if (tryNum == 24)
            ESP_LOGW(TAG,"Failed to Open the Screen!!!");
         }
      }
      

      vTaskDelay(pdMS_TO_TICKS(5000));
   }
}

void MainTask(void *pvParameters)
{
   configASSERT(((uint32_t)pvParameters) == 1);

   for (;;)
   {
      //if ir gives high, someone is moving
      if (gpio_get_level(IR_GPIO_NUM))
         xSemaphoreGive(KeepScreenOn);
      vTaskDelay(pdMS_TO_TICKS(5));
   }
}

void app_main(void)
{
   //init
   gpio_config_t io_conf;
   //interrupt of rising edge
   io_conf.intr_type = (gpio_int_type_t)GPIO_PIN_INTR_DISABLE;
   //bit mask of the pins, use GPIO4/5 here
   io_conf.pin_bit_mask = (1ULL << IR_GPIO_NUM);
   //set as input mode
   io_conf.mode = (gpio_mode_t)GPIO_MODE_INPUT;
   //enable pull-down mode
   io_conf.pull_down_en = (gpio_pulldown_t)1;
   gpio_config(&io_conf);

   KeepScreenOn = xSemaphoreCreateBinary();
   //example of rtos used this api not xTaskCreate
   //maybe we should use this too?
   xTaskCreatePinnedToCore(MainTask,
                           "MainTaskName",
                           4096,
                           nullptr,
                           5,
                           &MainTaskHandle,
                           tskNO_AFFINITY);

   xTaskCreatePinnedToCore(ScreenCtrTask,
                           "ScreenCtrTaskName",
                           4096,
                           nullptr,
                           6,
                           nullptr,
                           tskNO_AFFINITY);
   // while (1)
   // {
   //    PowerKey.doPress();
   //    ESP_LOGI(TAG, "suka!");
   //    vTaskDelay(pdMS_TO_TICKS(5000));
   // }
}
