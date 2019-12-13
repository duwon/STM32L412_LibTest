/**
  ******************************************************************************
  * File Name          : RTC.c
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

/* Includes ------------------------------------------------------------------*/
#include "rtc.h"

/* USER CODE BEGIN 0 */

#include "timeserver.h"
/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
  RTC_AlarmTypeDef sAlarm = {0};

  /** Initialize RTC Only 
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 124;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */
    
  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date 
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /** Enable the Alarm A 
  */
  sAlarm.AlarmTime.Hours = 0x0;
  sAlarm.AlarmTime.Minutes = 0x0;
  sAlarm.AlarmTime.Seconds = 0x0;
  sAlarm.AlarmTime.SubSeconds = 0x0;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay = 0x1;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

    HAL_RTCEx_EnableBypassShadow(rtcHandle);
  /* USER CODE END RTC_MspInit 0 */
    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();

    /* RTC interrupt Init */
    HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();

    /* RTC interrupt Deinit */
    HAL_NVIC_DisableIRQ(RTC_Alarm_IRQn);
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */
#define MIN_ALARM_DELAY 3                /* MCU Wake Up Time.  in ticks */
#define N_PREDIV_S 8                     /* subsecond number of bits */
#define PREDIV_S ((1 << N_PREDIV_S) - 1) /* Synchonuous prediv  */
#define HW_RTC_ALARMSUBSECONDMASK N_PREDIV_S << RTC_ALRMASSR_MASKSS_Pos;

/* RTC Time base in us */
#define USEC_NUMBER 1000000
#define MSEC_NUMBER (USEC_NUMBER / 1000)
#define RTC_ALARM_TIME_BASE (USEC_NUMBER >> N_PREDIV_S)

#define SECONDS_IN_1DAY ((uint32_t)86400U)
#define SECONDS_IN_1HOUR ((uint32_t)3600U)
#define SECONDS_IN_1MINUTE ((uint32_t)60U)
#define MINUTES_IN_1HOUR ((uint32_t)60U)
#define HOURS_IN_1DAY ((uint32_t)24U)

typedef struct
{
  uint32_t Rtc_Time;               /* Reference time */
  RTC_TimeTypeDef RTC_Calndr_Time; /* Reference time in calendar format */
  RTC_DateTypeDef RTC_Calndr_Date; /* Reference date in calendar format */
} RtcTimerContext_t;

static RtcTimerContext_t RtcTimerContext;
static RTC_AlarmTypeDef RTC_AlarmStructure;

static const uint8_t DaysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};         /* Number of days in each month on a normal year */
static const uint8_t DaysInMonthLeapYear[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; /* Number of days in each month on a leap year */

static uint64_t HW_RTC_GetCalendarValue(RTC_DateTypeDef *RTC_DateStruct, RTC_TimeTypeDef *RTC_TimeStruct);

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
  TimerIrqHandler();
}

/*!
 * @brief returns the wake up time in ticks
 * @param none
 * @retval wake up time in ticks
 */
uint32_t HW_RTC_GetMinimumTimeout(void)
{
  return (MIN_ALARM_DELAY);
}

/*!
 * @brief converts time in ms to time in ticks
 * @param [IN] time in milliseconds
 * @retval returns time in timer ticks
 */
uint32_t HW_RTC_ms2Tick(uint32_t timeMicroSec)
{
  return (uint32_t)(timeMicroSec * 1000 / RTC_ALARM_TIME_BASE);
}

/*!
 * @brief converts time in ticks to time in ms
 * @param [IN] time in timer ticks
 * @retval returns time in milliseconds
 */
uint32_t HW_RTC_Tick2ms(uint32_t tick)
{
  /*return( ( timeMicroSec * RTC_ALARM_TIME_BASE ) ); */
  uint32_t seconds = tick >> N_PREDIV_S;
  tick = tick & PREDIV_S;
  return ((seconds * 1000) + ((tick * 1000) >> N_PREDIV_S));
}

/*!
 * @brief Get the RTC timer elapsed time since the last Alarm was set
 * @param none
 * @retval RTC Elapsed time in ticks
 */
uint32_t HW_RTC_GetTimerElapsedTime(void)
{
  RTC_TimeTypeDef RTC_TimeStruct;
  RTC_DateTypeDef RTC_DateStruct;

  uint32_t CalendarValue = (uint32_t)HW_RTC_GetCalendarValue(&RTC_DateStruct, &RTC_TimeStruct);

  return ((uint32_t)(CalendarValue - RtcTimerContext.Rtc_Time));
}

/*!
 * @brief Get the RTC timer value
 * @param none
 * @retval RTC Timer value in ticks
 */
uint32_t HW_RTC_GetTimerValue(void)
{
  RTC_TimeTypeDef RTC_TimeStruct;
  RTC_DateTypeDef RTC_DateStruct;

  uint32_t CalendarValue = (uint32_t)HW_RTC_GetCalendarValue(&RTC_DateStruct, &RTC_TimeStruct);

  return (CalendarValue);
}

/*!
 * @brief Stop the Alarm
 * @param none
 * @retval none
 */
void HW_RTC_StopAlarm(void)
{
  HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);        /* Disable the Alarm A interrupt */ 
  __HAL_RTC_ALARM_CLEAR_FLAG(&hrtc, RTC_FLAG_ALRAF);  /* Clear RTC Alarm Flag */
  __HAL_RTC_ALARM_EXTI_CLEAR_FLAG();                  /* Clear the EXTI's line Flag for RTC Alarm */
}

/*!
 * @brief set Time Reference set also the RTC_DateStruct and RTC_TimeStruct
 * @param none
 * @retval Timer Value
 */
uint32_t HW_RTC_SetTimerContext(void)
{
  RtcTimerContext.Rtc_Time = (uint32_t)HW_RTC_GetCalendarValue(&RtcTimerContext.RTC_Calndr_Date, &RtcTimerContext.RTC_Calndr_Time);
  return (uint32_t)RtcTimerContext.Rtc_Time;
}

/*!
 * @brief Get the RTC timer Reference
 * @param none
 * @retval Timer Value in  Ticks
 */
uint32_t HW_RTC_GetTimerContext(void)
{
  return RtcTimerContext.Rtc_Time;
}

/*!
 * @brief start wake up alarm
 * @note  alarm in RtcTimerContext.Rtc_Time + timeoutValue
 * @param timeoutValue in ticks
 * @retval none
 */
void HW_RTC_StartWakeUpAlarm(uint32_t timeoutValue)
{
  uint16_t rtcAlarmSubSeconds = 0;
  uint16_t rtcAlarmSeconds = 0;
  uint16_t rtcAlarmMinutes = 0;
  uint16_t rtcAlarmHours = 0;
  uint16_t rtcAlarmDays = 0;
  RTC_TimeTypeDef RTC_TimeStruct = RtcTimerContext.RTC_Calndr_Time;
  RTC_DateTypeDef RTC_DateStruct = RtcTimerContext.RTC_Calndr_Date;

  HW_RTC_StopAlarm();

  /*reverse counter */
  rtcAlarmSubSeconds = PREDIV_S - RTC_TimeStruct.SubSeconds;
  rtcAlarmSubSeconds += (timeoutValue & PREDIV_S);

  timeoutValue >>= N_PREDIV_S; /* convert timeout  in seconds */

  /*convert microsecs to RTC format and add to 'Now' */
  rtcAlarmDays = RTC_DateStruct.Date;
  while (timeoutValue >= SECONDS_IN_1DAY)
  {
    timeoutValue -= SECONDS_IN_1DAY;
    rtcAlarmDays++;
  }

  /* calc hours */
  rtcAlarmHours = RTC_TimeStruct.Hours;
  while (timeoutValue >= SECONDS_IN_1HOUR)
  {
    timeoutValue -= SECONDS_IN_1HOUR;
    rtcAlarmHours++;
  }

  /* calc minutes */
  rtcAlarmMinutes = RTC_TimeStruct.Minutes;
  while (timeoutValue >= SECONDS_IN_1MINUTE)
  {
    timeoutValue -= SECONDS_IN_1MINUTE;
    rtcAlarmMinutes++;
  }

  /* calc seconds */
  rtcAlarmSeconds = RTC_TimeStruct.Seconds + timeoutValue;

  /***** correct for modulo********/
  while (rtcAlarmSubSeconds >= (PREDIV_S + 1))
  {
    rtcAlarmSubSeconds -= (PREDIV_S + 1);
    rtcAlarmSeconds++;
  }

  while (rtcAlarmSeconds >= SECONDS_IN_1MINUTE)
  {
    rtcAlarmSeconds -= SECONDS_IN_1MINUTE;
    rtcAlarmMinutes++;
  }

  while (rtcAlarmMinutes >= MINUTES_IN_1HOUR)
  {
    rtcAlarmMinutes -= MINUTES_IN_1HOUR;
    rtcAlarmHours++;
  }

  while (rtcAlarmHours >= HOURS_IN_1DAY)
  {
    rtcAlarmHours -= HOURS_IN_1DAY;
    rtcAlarmDays++;
  }

  if (RTC_DateStruct.Year % 4 == 0)
  {
    if (rtcAlarmDays > DaysInMonthLeapYear[RTC_DateStruct.Month - 1])
    {
      rtcAlarmDays = rtcAlarmDays % DaysInMonthLeapYear[RTC_DateStruct.Month - 1];
    }
  }
  else
  {
    if (rtcAlarmDays > DaysInMonth[RTC_DateStruct.Month - 1])
    {
      rtcAlarmDays = rtcAlarmDays % DaysInMonth[RTC_DateStruct.Month - 1];
    }
  }

  /* Set RTC_AlarmStructure with calculated values*/
  RTC_AlarmStructure.AlarmTime.SubSeconds = PREDIV_S - rtcAlarmSubSeconds;
  RTC_AlarmStructure.AlarmSubSecondMask = HW_RTC_ALARMSUBSECONDMASK;
  RTC_AlarmStructure.AlarmTime.Seconds = rtcAlarmSeconds;
  RTC_AlarmStructure.AlarmTime.Minutes = rtcAlarmMinutes;
  RTC_AlarmStructure.AlarmTime.Hours = rtcAlarmHours;
  RTC_AlarmStructure.AlarmDateWeekDay = (uint8_t)rtcAlarmDays;
  RTC_AlarmStructure.AlarmTime.TimeFormat = RTC_TimeStruct.TimeFormat;
  RTC_AlarmStructure.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  RTC_AlarmStructure.AlarmMask = RTC_ALARMMASK_NONE;
  RTC_AlarmStructure.Alarm = RTC_ALARM_A;
  RTC_AlarmStructure.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  RTC_AlarmStructure.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;

  /* Set RTC_Alarm */
  HAL_RTC_SetAlarm_IT(&hrtc, &RTC_AlarmStructure, RTC_FORMAT_BIN);
}

/*!
 * @brief a delay of delay ms by polling RTC
 * @param delay in ms
 * @retval none
 */
void HW_RTC_DelayMs( uint32_t delay )
{
  uint32_t delayValue = 0;
  uint32_t timeout = 0;

  delayValue = HW_RTC_ms2Tick( delay );

  /* Wait delay ms */
  timeout = HW_RTC_GetTimerValue( );
  while( ( ( HW_RTC_GetTimerValue( ) - timeout ) ) < delayValue )
  {
    __NOP( );
  }
}

/* Private functions ---------------------------------------------------------*/

/*!
 * @brief get current time from calendar in ticks
 * @param pointer to RTC_DateStruct
 * @param pointer to RTC_TimeStruct
 * @retval time in ticks
 */
static uint64_t HW_RTC_GetCalendarValue(RTC_DateTypeDef *RTC_DateStruct, RTC_TimeTypeDef *RTC_TimeStruct)
{
  uint64_t calendarValue = 0;
  uint32_t correction;
  uint32_t seconds;

  /* Get Time and Date*/
  HAL_RTC_GetDate(&hrtc, RTC_DateStruct, RTC_FORMAT_BIN);
  HAL_RTC_GetTime(&hrtc, RTC_TimeStruct, RTC_FORMAT_BIN);

  /* calculte amount of elapsed days since 01/01/2000 */
  seconds = (1461 * RTC_DateStruct->Year / 4);
  correction = ((RTC_DateStruct->Year % 4) == 0) ? 0x99AAA0 : 0x445550;
  seconds += ((((RTC_DateStruct->Month - 1) * (30 + 31) + 1) / 2) - (((correction >> ((RTC_DateStruct->Month - 1) * 2)) & 0x3)));
  seconds += (RTC_DateStruct->Date - 1);

  /* convert from days to seconds */
  seconds *= 86400;
  seconds += ((uint32_t)RTC_TimeStruct->Seconds +
              ((uint32_t)RTC_TimeStruct->Minutes * SECONDS_IN_1MINUTE) +
              ((uint32_t)RTC_TimeStruct->Hours * SECONDS_IN_1HOUR));

  calendarValue = (((uint64_t)seconds) << N_PREDIV_S) + (PREDIV_S - RTC_TimeStruct->SubSeconds);

  return (calendarValue);
}

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
