#include <stdio.h>
#include "user.h"
#include "gpio.h"
#include "usart.h"
#include "rtc.h"
#include "timeserver.h"

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

PUTCHAR_PROTOTYPE
{
  while (HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF) != HAL_OK)
    ;
  return ch;
}


uint8_t rxData[1];

static TimerEvent_t LedTimer;
static void OnLedTimerEvent(void);

static void OnLedTimerEvent(void)
{
  TimerStart(&LedTimer);
  HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
}

static void timer_start(void)
{
  TimerInit(&LedTimer, OnLedTimerEvent);
  TimerSetValue(&LedTimer, 1000);
  TimerStart(&LedTimer);
}

static void printUartLogo(void)
{
  printf("\r\n");
  printf("\r\n=============================================");
  printf("\r\n|           2019         Duwon Jung         |");
  printf("\r\n|                                           |");
  printf("\r\n|   HAL Lib Test     %s %s   |", __DATE__, __TIME__);
  printf("\r\n=============================================");
  printf("\r\n\r\n");
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == USART2)
  {
    printf("%c",rxData[0]);
    HAL_UART_Receive_DMA(huart, rxData, 1);
  }
}

void user_start(void)
{
  printUartLogo();
  HAL_UART_Receive_DMA(&huart2, rxData, 1);
  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
	
	timer_start();
}

void user_while(void)
{
  HW_RTC_DelayMs(5000);
	printf("while...\r\n");
	
}
