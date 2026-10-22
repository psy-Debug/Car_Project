#ifndef __USART2_H
#define __USART2_H

#include "stm32f10x.h"
#include "serial_if.h"

extern const serial_driver_t usart2_driver;

void USART2_Config(uint32_t baudrate);

#endif
