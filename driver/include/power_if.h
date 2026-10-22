#ifndef __POWER_IF_H
#define __POWER_IF_H

typedef struct {
    int   (*init)(void);
    float (*get_voltage)(void);
} power_driver_t;

#endif
