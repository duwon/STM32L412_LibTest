#include "user.h"
#include "gpio.h"

void user_start(void)
{
  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
}

void user_while(void)
{

}
