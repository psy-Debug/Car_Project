#ifndef __USART3_H
#define __USART3_H

#include "stm32f10x.h"
#include <stdio.h>
#include "serial_if.h"

extern const serial_driver_t usart3_driver;

void USART3_Init(uint32_t baudrate);

#endif
