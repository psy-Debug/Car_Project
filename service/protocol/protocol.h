#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include <stdint.h>

void protocol_feed_byte(uint8_t byte);
void protocol_poll(void);
float protocol_get_target_speed(void);
float protocol_get_target_turn(void);
int   protocol_get_flag(void);

#endif
