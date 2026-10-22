#ifndef __MOTOR_IF_H
#define __MOTOR_IF_H

#include <stdint.h>

typedef struct {
    int  (*init)(void);
    void (*set_output)(int16_t pwm_l, int16_t pwm_r);
} motor_driver_t;

#endif
