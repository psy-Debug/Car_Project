#include "protocol.h"
#include "pid.h"
#include <stdlib.h>
#include <string.h>

static float Target_Speed_Raw = 0;
static float Target_Turn_Raw = 0;
static int   Flag = -1;

static uint8_t Rx_Buf[64];
static uint8_t Rx_Idx = 0;
static uint8_t Rx_Flag = 0;

void protocol_feed_byte(uint8_t data)
{
    if (data == '{') {
        Rx_Idx = 0;
        memset(Rx_Buf, 0, 64);
    } else if (data == '}') {
        Rx_Flag = 1;
    } else {
        if (Rx_Idx < 63) Rx_Buf[Rx_Idx++] = data;
    }
}

void protocol_poll(void)
{
    if (Rx_Flag) {
        Rx_Flag = 0;
        char cmd = Rx_Buf[0];
        float val = atof((char*)(Rx_Buf + 2));

        switch(cmd) {
            case 'P': PID_Set_Vertical_Kp(val); break;
            case 'D': PID_Set_Vertical_Kd(val); break;
            case 'S': PID_Set_Speed_Kp(val); break;
            case 'I': PID_Set_Speed_Ki(val); break;
            case 'Z': PID_Set_DeadZone(val); break;
            case 'G': PID_Set_Gravity_K(val); break;
            case 'O': PID_Set_Mech_Offset(val); break;

            case 'M':
                if ((int)val == 1) Target_Speed_Raw = 5.0;
                else if ((int)val == 2) Target_Speed_Raw = -5.0;
                else Target_Speed_Raw = 0;
                break;

            case 'F':
                if ((int)val == 1) Flag = 1;
                else Flag = 0;
                break;

            case 'T':
                if ((int)val == 1) Target_Turn_Raw = 1.2;
                else if ((int)val == 2) Target_Turn_Raw = -1.2;
                else Target_Turn_Raw = 0;
                break;
        }
    }
}

float protocol_get_target_speed(void) { return Target_Speed_Raw; }
float protocol_get_target_turn(void)  { return Target_Turn_Raw; }
int   protocol_get_flag(void)         { return Flag; }
