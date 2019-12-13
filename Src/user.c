#include <stdio.h>
#include "user.h"
#include "gpio.h"
#include "usart.h"

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


static void printUartLogo(void)
{
  printf("\r\n");
  printf("\r\n=============================================");
  printf("\r\n|           2019         Duwon Jung         |");
  printf("\r\n|                                           |");
  printf("\r\n|   HAL Lib Test     %s %s   |",  __DATE__, __TIME__);
  printf("\r\n=============================================");
  printf("\r\n\r\n");
}

void user_start(void)
{
	printUartLogo();
  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
}

void user_while(void)
{

}
