/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
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


#include "app_uart.h"
#include "main.h"

#define TAG "app_main"

#define IR_GPIO_NUM ((gpio_num_t)32)
#define TOUCH_GPIO_NUM ((gpio_num_t)33)
#define ESP_INTR_FLAG_DEFAULT 0

// keyPressEmulator PowerKey(18, 0);
comm::app_uart mainUart(UART_NUM_1, 10, 9);
SemaphoreHandle_t KeepScreenOn = nullptr;
SemaphoreHandle_t GotOpenRespond = nullptr;
SemaphoreHandle_t GotCloseRespond = nullptr;

TaskHandle_t MainTaskHandle = nullptr;


int handlerOne(comm::app_uart::FW_ FW, comm::app_uart::SFW_ SFW, uint8_t* data, comm::app_uart* pt) {
   switch (FW)
   {
   case comm::app_uart::FW_HB:
      if (SFW == comm::app_uart::SFW_RESQUEST)
      {
         uint8_t tmpd[16];
         bzero(tmpd,16);
         pt->send(comm::app_uart::FW_HB,comm::app_uart::SFW_RESPOND,tmpd);
      }
      
      break;
   case comm::app_uart::FW_OPEN:
   if (SFW == comm::app_uart::SFW_RESPOND){
      xSemaphoreGive(GotOpenRespond);
   }
   break;

   case comm::app_uart::FW_CLOSE:
   if (SFW == comm::app_uart::SFW_RESPOND){
      xSemaphoreGive(GotCloseRespond);
   }
   break;
   
   default:
   return -1;//FW not found
      break;
   }
   return 0;
}

void ScreenCtrTask(void *pvParameters)
{
   while (1)
   {
      if (xSemaphoreTake(KeepScreenOn, pdMS_TO_TICKS(45000)) != pdTRUE) //try 45s if can recieve Semaphore
      {
         ESP_LOGD(TAG,"Semaphore Time Out!,Closing!");
         //time to close the screen!
         for (int tryNum = 0; tryNum < 25; tryNum++) //try 25 times see if we can close screen nicely :)
         {
               uint8_t tmp[18];
               bzero(tmp,18);
               mainUart.send(comm::app_uart::FW_CLOSE,comm::app_uart::SFW_RESQUEST,tmp);
               if (xSemaphoreTake(GotCloseRespond,pdMS_TO_TICKS(500)) == pdTRUE){
                  break;
               }
               if (tryNum>=24) ESP_LOGW(TAG,"Fail to get any respond from pi!");
         }
      }
      else
      {
         ESP_LOGD(TAG,"Got Open Semaphore!");
         //try to open the monitor!
         for (int tryNum = 0; tryNum < 25; tryNum++) //try 25 times see if we can close screen nicely :)
         {
               uint8_t tmp[18];
               bzero(tmp,18);
               mainUart.send(comm::app_uart::FW_OPEN,comm::app_uart::SFW_RESQUEST,tmp);
               if (xSemaphoreTake(GotOpenRespond,pdMS_TO_TICKS(500)) == pdTRUE){
                  break;
               }
               if (tryNum>=24) ESP_LOGW(TAG,"Fail to get any respond from pi!");
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

   KeepScreenOn = xSemaphoreCreateBinary();
   GotOpenRespond = xSemaphoreCreateBinary();
   GotCloseRespond = xSemaphoreCreateBinary();

   gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
   gpio_isr_handler_add(TOUCH_GPIO_NUM, [](void *){
      static BaseType_t xHigherPriorityTaskWoken = pdFALSE; 
      xSemaphoreGiveFromISR(KeepScreenOn,&xHigherPriorityTaskWoken);
      //ESP_LOGI(TAG,"Touch signal got!"); //Use ESP_LOG in isr will make you boom!!
      }, nullptr);

   if (KeepScreenOn == nullptr || GotOpenRespond == nullptr || GotCloseRespond == nullptr){
      ESP_LOGE(TAG,"Failed to create Semaphore!");
   }

   mainUart.handler=handlerOne;
   mainUart.init();

   // ScreenChecker::init();
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
   // while (1)
   // {
   //    uint8_t tmp[18];
   //    bzero(tmp,18);
   //    mainUart.send(comm::app_uart::FW_CLOSE,comm::app_uart::SFW_RESQUEST,tmp);
   //    vTaskDelay(10);
   // }
   
}
