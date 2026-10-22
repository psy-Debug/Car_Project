#ifndef __USART1_H
#define __USART1_H

#include "stm32f10x.h"
#include <stdio.h>
#include "serial_if.h"

extern const serial_driver_t usart1_driver;

void USART1_Config(void);

#endif
