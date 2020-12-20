/**
 * @file screenChecker.cpp
 * @author kisonhe
 * @brief 
 * @version 0.1
 * @date 2020-12-16
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "esp_adc_cal.h"
#include "driver/adc.h"

#include "screenChecker.h"

//  Configs
#define TAG "SC"
#define NO_OF_SAMPLES 16 //Multisampling
#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
static const uint32_t threshold = 1000; //treat 1000mv as led is on



static esp_adc_cal_characteristics_t *adc_chars;

static uint32_t adc_REDreading = 0;
static uint32_t adc_GREENreading = 0;
static uint32_t adc_REDvoltage = 0;
static uint32_t adc_GREENvoltage = 0;

//random channel
static adc1_channel_t REDchannel = (adc1_channel_t)ADC_CHANNEL_6;   //GPIO34
static adc1_channel_t GREENchannel = (adc1_channel_t)ADC_CHANNEL_7; //GPIO35

namespace ScreenChecker
{
    SCStatus_ SCStatus;
    /**
     * @brief Must call init() before calling this 
     * 
     * @return int 
     */
    int Routine()
    {
        for (int i = 0; i < NO_OF_SAMPLES; i++)
        {
            adc_REDreading += adc1_get_raw(REDchannel);
            adc_GREENreading += adc1_get_raw(GREENchannel);
        }
        adc_REDreading /= NO_OF_SAMPLES;
        adc_GREENreading /= NO_OF_SAMPLES;
        adc_REDvoltage = esp_adc_cal_raw_to_voltage(adc_REDreading, adc_chars);
        adc_GREENvoltage = esp_adc_cal_raw_to_voltage(adc_GREENreading, adc_chars);
        ESP_LOGV(TAG,"Red V=%d",adc_REDvoltage);
        ESP_LOGV(TAG,"GREEN V=%d",adc_GREENvoltage);
        if (adc_REDvoltage < threshold && adc_GREENvoltage < threshold){
            // SCStatus.isPowered = false;
            SCStatus.isTurnedOn = false;
        }
        else if (adc_REDvoltage >= threshold && adc_GREENvoltage >= threshold)
        {
            ESP_LOGW(TAG,"Both LED are on! Adjust your threshold!");
        }
        // else if (adc_REDvoltage >= threshold)
        // {
        //     // SCStatus.isPowered = true;
        //     SCStatus.isTurnedOn = false;
        // }
        else
        {
            // SCStatus.isPowered = true;
            SCStatus.isTurnedOn = true;
        }
        
        
        
        

        return 0;
    }
    //[TODO] 搞懂 ADC Characterization Before writing any mote code
    /**
     * @brief 
     * 
     * @return int 
     */
    int init()
    {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(REDchannel, ADC_ATTEN_DB_0);
        adc1_config_channel_atten(GREENchannel, ADC_ATTEN_DB_0);

        adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
        esp_adc_cal_characterize((adc_unit_t)ADC_UNIT_1, (adc_atten_t)ADC_ATTEN_DB_0, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
        xTimerStart(xTimerCreate("SC", pdMS_TO_TICKS(10), true, nullptr, [](TimerHandle_t tmr) { ScreenChecker::Routine(); }), pdMS_TO_TICKS(10));

        return 0;
    }

} // namespace ScreenChecker