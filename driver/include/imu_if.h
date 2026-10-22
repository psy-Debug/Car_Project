#ifndef __IMU_IF_H
#define __IMU_IF_H

#include <stdint.h>

typedef struct {
    int16_t acc_x,  acc_y,  acc_z;
    int16_t gyro_x, gyro_y, gyro_z;
    float   accel_x, accel_y, accel_z;
    float   gyro_rx, gyro_ry, gyro_rz;
    float   pitch, roll, yaw;
    float   temp;
} imu_data_t;

typedef struct {
    int  (*init)(void);
    int  (*read)(imu_data_t *data);
} imu_driver_t;

#endif
