#ifndef __MPU6050_H
#define __MPU6050_H

#include "stm32f10x.h"
#include "imu_if.h"

#define MPU6050_ADDR  0xD0

extern const imu_driver_t mpu6050_driver;

void MPU6050_I2C_Init(void);
void MPU6050_INT_Init(void);
void MPU6050_Init(void);
void MPU6050_Start_DMA_Read(void);
void MPU6050_Read_orgin(void);
uint8_t I2C_CheckEvent_Timeout(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT);

int16_t MPU6050_GetAccX(void);
int16_t MPU6050_GetAccY(void);
int16_t MPU6050_GetAccZ(void);
int16_t MPU6050_GetGyroX(void);
int16_t MPU6050_GetGyroY(void);
int16_t MPU6050_GetGyroZ(void);
int16_t MPU6050_GetTemp(void);

float MPU6050_Getax(void);
float MPU6050_Getay(void);
float MPU6050_Getaz(void);
float MPU6050_Getgx(void);
float MPU6050_Getgy(void);
float MPU6050_Getgz(void);
float MPU6050_Gett(void);
float MPU6050_GetPitch(void);
float MPU6050_GetYaw(void);
float MPU6050_GetRoll(void);

#endif
