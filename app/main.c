#include "stm32f10x.h"
#include "imu_if.h"
#include "encoder_if.h"
#include "motor_if.h"
#include "display_if.h"
#include "power_if.h"
#include "serial_if.h"
#include "pid.h"
#include "protocol.h"
#include "timer_scheduler.h"

/* ---- driver instances (defined in driver layer) ---- */
extern const imu_driver_t      mpu6050_driver;
extern const encoder_driver_t  encoder_driver;
extern const motor_driver_t    motor_driver;
extern const display_driver_t  oled_driver;
extern const power_driver_t    battery_driver;
extern const serial_driver_t   usart1_driver;
extern const serial_driver_t   usart2_driver;

static const imu_driver_t     *imu     = &mpu6050_driver;
static const encoder_driver_t *encoder = &encoder_driver;
static const motor_driver_t   *motor   = &motor_driver;
static const display_driver_t *oled    = &oled_driver;
static const power_driver_t   *battery = &battery_driver;
static const serial_driver_t  *debug   = &usart1_driver;
static const serial_driver_t  *bt      = &usart2_driver;

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    oled->init();
    debug->init(115200);
    bt->init(9600);
    motor->init();
    encoder->init();
    imu->init();
    battery->init();
    Timer4_Init();
    PID_Init();

    imu_data_t        imu_data;
    encoder_channel_t enc_l, enc_r;
    float             roll      = 0.0f;
    float             gyro_x    = 0.0f;
    float             voltage   = 12.6f;
    float             target_speed;
    float             target_turn;
    int               oled_flag;

    while(1)
    {
        /* ---- Block A: sensor data (MPU6050 via DMA+ISR) ---- */
        if (imu->read(&imu_data) == 0) {
            roll   = imu_data.roll;
            gyro_x = imu_data.gyro_rx;
        }

        /* ---- Block B: control loop (5ms ISR driven) ---- */
        if (PID_Task_Flag) {
            PID_Task_Flag = 0;

            encoder->get_left(&enc_l);
            encoder->get_right(&enc_r);
            float speed_avg = (enc_l.speed + enc_r.speed) / 2.0f;

            target_speed = protocol_get_target_speed();
            target_turn  = protocol_get_target_turn();

            float V_Speed    = PID_Calc_Speed(0, speed_avg);
            float V_Vertical = -PID_Calc_Vertical(0.0f, roll, -gyro_x, target_speed);
            float V_Gravity  = PID_Calc_Gravity(roll);

            float V_Base = V_Vertical + V_Speed + V_Gravity;
            V_Base = PID_Calc_DeadZone_Compensate(V_Base);

            float V_Final_L = V_Base + target_turn;
            float V_Final_R = V_Base - target_turn;

            voltage = battery->get_voltage();
            int16_t PWM_L = (int16_t)((V_Final_L / voltage) * 7200.0f);
            int16_t PWM_R = -(int16_t)((V_Final_R / voltage) * 7200.0f);

            motor->set_output(PWM_L, PWM_R);
        }

        /* ---- Block C: command & display (low priority) ---- */
        protocol_poll();
        oled_flag = protocol_get_flag();

        if (oled_flag == 1) {
            oled->update();
            oled->show_float(16, 16, roll,    5, 1, 8);
            oled->show_float(16, 32, gyro_x,  5, 1, 8);
            oled->show_float(16, 48, voltage, 5, 1, 8);
        } else if (oled_flag == 0) {
            oled->clear();
        }
    }
}
