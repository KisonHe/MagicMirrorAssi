/**
 * @file app_uart.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2020-12-20
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "app_uart.h"

#include "esp_log.h"

namespace comm
{
    app_uart::app_uart(int UART_NUM_,int txPin_, int rxPin_,uint8_t FRAME_HEAD_, uint8_t FRAME_END_)
    {
        FRAME_HEAD = FRAME_HEAD_;
        FRAME_END = FRAME_END_;
        UART_NUM = UART_NUM;
        txPin = txPin_;
        rxPin = rxPin_;
    }
    app_uart::~app_uart(){
        vTaskDelete(&xHandle);
        uart_driver_delete(UART_NUM);
    }

    void app_uart::uart_event_task(void *pvParameters)
    {
        app_uart* thispt =  static_cast<app_uart *>(pvParameters);
        uart_event_t event;
        uint8_t dtmp[BUF_SIZE];
        for (;;)
        {
            //Waiting for UART event.
            if (xQueueReceive(thispt->uart_queue, (void *)&event, (portTickType)portMAX_DELAY))
            {
                bzero(dtmp, thispt->BUF_SIZE);
                switch (event.type)
                {
                //Event of UART receving data
                /*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.*/
                case UART_DATA:
                // ESP_LOGD("app_uart","Got sth from uart");
                    uart_read_bytes(thispt->UART_NUM, dtmp, event.size, portMAX_DELAY);
                    if (is_frame_right(dtmp, thispt) == 0)
                    {
                        thispt->handler(static_cast<FW_>(dtmp[1]),static_cast<SFW_>(dtmp[2]),&dtmp[3],thispt);
                        //[todo]huihui's buffer;
                    }
                    break;
                //Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    uart_flush_input(thispt->UART_NUM);
                    xQueueReset(thispt->uart_queue);
                    break;
                //Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    uart_flush_input(thispt->UART_NUM);
                    xQueueReset(thispt->uart_queue);
                    break;
                //UART_PATTERN_DET
                default:
                    break;
                }
            }
        }
    }

    /**
     * @brief 
     * 
     * @return int -1 if handler not registed
     */
    int app_uart::init(){
        if (handler == nullptr){
            ESP_LOGE("app_uart","Didnt regist handler before initing!");
            return -1;
        }
        ESP_ERROR_CHECK(uart_driver_install(UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 10, &uart_queue, 0));
        ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));
        ESP_ERROR_CHECK(uart_set_pin(UART_NUM, txPin, rxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
        // ESP_ERROR_CHECK(uart_set_pin(UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
        return xTaskCreate(uart_event_task, "uart_event_task", 2048, this, 12, &xHandle); 
        //[todo] try if this work with mutil app_uart instance. and also destructor
    }

    int app_uart::send(FW_ FW, SFW_ SFW, uint8_t* data){
        static uint8_t Message[22];
        uint8_t tmp_sum = 0;
        memset(Message, 0, sizeof(Message));
        Message[0] = FRAME_HEAD;
        Message[1] = FW;
        Message[2] = SFW;
        memcpy(&Message[3], data, 16);
        for (int i = 1; i < 19; i++) {
            tmp_sum += Message[i];
        }
        Message[19] = tmp_sum;
        Message[20] = FRAME_END;
        return uart_write_bytes(this->UART_NUM, (const char*)Message, sizeof(Message));
    }





    int app_uart::is_frame_right(uint8_t *RxData, app_uart* pt){
        if ((RxData[0] == pt->FRAME_HEAD) && (RxData[20] == pt->FRAME_END))
        {
            uint8_t tmp_sum = 0;
            //帧头帧尾对了，检测一次和校验
            for (int i = 1; i < 19; i++)
                tmp_sum += RxData[i];

            tmp_sum = tmp_sum & 0xff;
            if (tmp_sum != RxData[19])
                return -1; //校验和出错，直接退出,继续在缓冲区检测数据帧

            //帧头帧尾正确，和校验正确，开始解析
            return 0;
        }
        return -1;
    }
} // namespace comm
