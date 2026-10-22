#ifndef __SERIAL_IF_H
#define __SERIAL_IF_H

#include <stdint.h>

typedef struct {
    int  (*init)(uint32_t baudrate);
    void (*send_string)(const char *str);
    void (*send_bytes)(const uint8_t *buf, uint16_t len);
    int  (*poll_recv)(uint8_t *buf, uint16_t max_len);
} serial_driver_t;

#endif
