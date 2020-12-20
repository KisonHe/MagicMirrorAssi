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
#include "esp_task_wdt.h"
#include "driver/gpio.h"


#include "main.h"

#define TAG "app_main"

#define IR_GPIO_NUM ((gpio_num_t)32)
#define TOUCH_GPIO_NUM ((gpio_num_t)33)
#define ESP_INTR_FLAG_DEFAULT 0

// keyPressEmulator PowerKey(18, 0);
SemaphoreHandle_t KeepScreenOn = nullptr;

TaskHandle_t MainTaskHandle = nullptr;

void ScreenCtrTask(void *pvParameters)
{
   while (1)
   {
      if (xSemaphoreTake(KeepScreenOn, pdMS_TO_TICKS(45000)) != pdTRUE) //try 45s if can recieve Semaphore
      {
         ESP_LOGD(TAG,"Semaphore Time Out!");
         //time to close the screen!
         ESP_LOGD(TAG,"Closing!");
         for (int tryNum = 0; tryNum < 25; tryNum++) //try 25 times see if we can close screen nicely :)
         {
            if (ScreenChecker::SCStatus.isTurnedOn)
            {
               ESP_LOGD(TAG,"Doing Press!");
               // PowerKey.doPress();
               vTaskDelay(pdMS_TO_TICKS(500));  //wait 500ms before nextCheck
            }
            else
            {
               ESP_LOGD(TAG,"Already Closed!");
               break;
            }
         }
      }
      else
      {
         ESP_LOGD(TAG,"Got Semaphore!");
         //try to open the monitor!
         for (int tryNum = 0; tryNum < 25; tryNum++) //try 25 times see if we can open screen nicely :)
         {
            if (!ScreenChecker::SCStatus.isTurnedOn)
            {
               // if (!ScreenChecker::SCStatus.isPowered){
               //    ESP_LOGW(TAG,"Screen is not powered!");
               //    break;
               //    //[todo] Remind User to turn the power on
               // } else
               // {
                  ESP_LOGD(TAG,"Doing Press!");
                  // PowerKey.doPress();
                  vTaskDelay(pdMS_TO_TICKS(500));  //wait 500ms before nextCheck
               // }       
            }
            else
            {
               ESP_LOGW(TAG,"Already On!");
               break;
            } 
         }
      }
      

      vTaskDelay(pdMS_TO_TICKS(50));
   }
}

void MainTask(void *pvParameters)
{
   for (;;)
   {
      //if ir gives high, someone is moving
      if (gpio_get_level(IR_GPIO_NUM))
         xSemaphoreGive(KeepScreenOn);
      vTaskDelay(pdMS_TO_TICKS(10));
   }
}

void app_main(void)
{
   //init
   gpio_config_t io_conf;
   //no interrupt
   io_conf.intr_type = (gpio_int_type_t)GPIO_PIN_INTR_DISABLE;
   //bit mask of the pins
   io_conf.pin_bit_mask = (1ULL << IR_GPIO_NUM);
   //set as input mode
   io_conf.mode = (gpio_mode_t)GPIO_MODE_INPUT;
   //enable pull-down mode
   io_conf.pull_down_en = (gpio_pulldown_t)1;
   gpio_config(&io_conf);

   //interrupt of rising edge
   io_conf.intr_type = (gpio_int_type_t)GPIO_PIN_INTR_POSEDGE;
   //bit mask of the pins
   io_conf.pin_bit_mask = (1ULL << TOUCH_GPIO_NUM);
   //set as input mode
   io_conf.mode = (gpio_mode_t)GPIO_MODE_INPUT;
   //enable pull-down mode
   io_conf.pull_down_en = (gpio_pulldown_t)1;
   gpio_config(&io_conf);

   gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
   gpio_isr_handler_add(TOUCH_GPIO_NUM, [](void *){
      static BaseType_t xHigherPriorityTaskWoken = pdFALSE; 
      xSemaphoreGiveFromISR(KeepScreenOn,&xHigherPriorityTaskWoken);
      //ESP_LOGI(TAG,"Touch signal got!"); //Use ESP_LOG in isr will make you boom!!
      }, nullptr);

   KeepScreenOn = xSemaphoreCreateBinary();
   if (KeepScreenOn == nullptr){
      ESP_LOGE(TAG,"Failed to create Semaphore!");
   }
   ScreenChecker::init();
   // example of rtos used this api not xTaskCreate
   // maybe we should use this too?
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
}
