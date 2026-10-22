#ifndef __ENCODER_IF_H
#define __ENCODER_IF_H

#include <stdint.h>

typedef struct {
    float   speed;
    float   angle;
    int64_t count;
} encoder_channel_t;

typedef struct {
    int  (*init)(void);
    void (*get_left)(encoder_channel_t *ch);
    void (*get_right)(encoder_channel_t *ch);
} encoder_driver_t;

#endif
