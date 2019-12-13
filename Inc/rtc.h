/**
  ******************************************************************************
  * File Name          : RTC.h
  * Description        : This file provides code for the configuration
  *                      of the RTC instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __rtc_H
#define __rtc_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern RTC_HandleTypeDef hrtc;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_RTC_Init(void);

/* USER CODE BEGIN Prototypes */
uint32_t HW_RTC_SetTimerContext( void );
uint32_t HW_RTC_GetTimerElapsedTime( void );
uint32_t HW_RTC_GetTimerContext( void );
void HW_RTC_StopAlarm( void );
uint32_t HW_RTC_ms2Tick( uint32_t timeMicroSec );
uint32_t HW_RTC_GetTimerValue( void );
uint32_t HW_RTC_Tick2ms( uint32_t tick );
void HW_RTC_StartWakeUpAlarm( uint32_t timeout );
void HW_RTC_AlarmConfig(void);
void HW_RTC_DelayMs( uint32_t delay );

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ rtc_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
