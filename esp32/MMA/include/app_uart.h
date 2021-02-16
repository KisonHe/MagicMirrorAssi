/**
 * @file app_uart.h
 * @author kisonhe
 * @brief 
 * @version 0.1
 * @date 2020-12-20
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#ifndef APP_UART_H
#define APP_UART_H

#include "freertos/queue.h"
#include "driver/uart.h"

namespace comm{
    class app_uart
    {
    private:
        TaskHandle_t xHandle = nullptr;
        static void uart_event_task(void *pvParameters);
        uint8_t FRAME_HEAD = 0xAF;
        uint8_t FRAME_END = 0xFF;
        int UART_NUM = 0;
        int txPin = 0;
        int rxPin = 0;
        static int is_frame_right(uint8_t* RxData, app_uart* pt);
        QueueHandle_t uart_queue;
        /* data */
    public:
        static const int BUF_SIZE = 512;
        uart_config_t uart_config = {
            .baud_rate = 115200,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .source_clk = UART_SCLK_APB,
        };
        /**
         * @brief FW_HB: Heart Beat
         * 
         */
        enum FW_ {FW_HB = 0,FW_OPEN = 1,FW_CLOSE = 2};
        enum SFW_ {SFW_RESQUEST = 0,SFW_RESPOND = 1};
        int (*handler)(FW_ FW, SFW_ SFW, uint8_t* data, app_uart* pt) = nullptr;
        int init();//Must register handler before init
        int send(FW_ FW, SFW_ SFW, uint8_t* data);
        app_uart(int UART_NUM_,int txPin_, int rxPin_,uint8_t FRAME_HEAD_ = 0xAF,uint8_t FRAME_END_ = 0xFF);
        ~app_uart();
    };
    
}

#endif
