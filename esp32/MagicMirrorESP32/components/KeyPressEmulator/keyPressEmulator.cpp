/**
 * @file keyPressEmulator.cpp
 * @author kisonhe
 * @brief 
 * @version 0.1
 * @date 2020-12-16
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
#include "esp_log.h"
#include "driver/gpio.h"

#include "keyPressEmulator.h"

#define TAG "KPE"

// int createKPETimer(){
//     xTimerStart(xTimerCreate("KPE", 50, true, nullptr, [](TimerHandle_t tmr) { Button::Routine(); }), 50);
// }


/**
 * @brief Config the pin to float gpio input if not using
 * 
 * @return int 
 */
int keyPressEmulator::_deinitPin(){
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = (gpio_int_type_t)GPIO_PIN_INTR_DISABLE;
    //bit mask of the pins that you want to set
    io_conf.pin_bit_mask = (1ULL<<gpioNum);
    //set as input mode    
    io_conf.mode = (gpio_mode_t)GPIO_MODE_INPUT;
    //disable pull-down mode
    io_conf.pull_down_en = (gpio_pulldown_t)0;
    //disable pull-up mode
    io_conf.pull_up_en = (gpio_pullup_t)0;
    return gpio_config(&io_conf);
}

/**
 * @brief Config the pin to gpio out
 * 
 * @return int 
 */
int keyPressEmulator::_initPin(){
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = (gpio_int_type_t)GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = (gpio_mode_t)GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set
    io_conf.pin_bit_mask = (1ULL<<gpioNum);
    //disable pull-down mode
    io_conf.pull_down_en = (gpio_pulldown_t)0;
    //disable pull-up mode
    io_conf.pull_up_en = (gpio_pullup_t)0;
    //configure GPIO with the given settings
    return gpio_config(&io_conf);

}

/**
 * @brief 
 * 
 * @note it will block the caller task for a while
 * add some callback?
 * 
 * @return int 
 */
int keyPressEmulator::doPress(){
    if (lock)
        return -1;
    lock = true;
    _initPin();
    ESP_LOGD(TAG,"Press!");
    gpio_set_level((gpio_num_t)gpioNum,isHigh2Press);
    vTaskDelay(pdMS_TO_TICKS(100));    //simulate the press
    gpio_set_level((gpio_num_t)gpioNum,(!isHigh2Press));
    ESP_LOGD(TAG,"Release!");
    vTaskDelay(pdMS_TO_TICKS(100));    //optional, some board require some delay
    _deinitPin();

    lock = false;
    return 0;
}

keyPressEmulator::keyPressEmulator(int gpioNum_, int isHigh2Press_)
{
    lock = false;
    gpioNum = gpioNum_;
    isHigh2Press = isHigh2Press_;
    _deinitPin();
}


