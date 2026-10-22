#ifndef __DISPLAY_IF_H
#define __DISPLAY_IF_H

#include <stdint.h>

typedef struct {
    int  (*init)(void);
    void (*clear)(void);
    void (*update)(void);
    void (*show_float)(int16_t x, int16_t y, double num,
                       uint8_t int_len, uint8_t fra_len, uint8_t font);
} display_driver_t;

#endif
