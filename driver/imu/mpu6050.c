#include "mpu6050.h"
#include <stdio.h>
#include "qmath.h"

static float Filtered_Gyro = 0;

uint8_t MPU_RawData[14];
volatile uint8_t MPU_DataReady = 0;
volatile uint8_t I2C_TimeOutFlag = 0;
volatile int16_t AccX, AccY, AccZ, GyroX, GyroY, GyroZ, Temp;
volatile float ax, ay, az, gx, gy, gz, t;
static float Pitch = 0.0f, Roll = 0.0f, Yaw = 0.0f;
#define I2C_TIMEOUT_MAX  0x5000

/* ---- I2C helper ---- */
uint8_t I2C_CheckEvent_Timeout(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT) {
    uint32_t timeout = I2C_TIMEOUT_MAX;
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT)) {
        if ((timeout--) == 0) {
            I2C_TimeOutFlag = 1;
            return 1;
        }
    }
    return 0;
}

static void MPU6050_WriteReg(uint8_t reg, uint8_t data) {
    I2C_GenerateSTART(I2C1, ENABLE);
    if(I2C_CheckEvent_Timeout(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) return;
    I2C_Send7bitAddress(I2C1, MPU6050_ADDR, I2C_Direction_Transmitter);
    if(I2C_CheckEvent_Timeout(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) return;
    I2C_SendData(I2C1, reg);
    if(I2C_CheckEvent_Timeout(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) return;
    I2C_SendData(I2C1, data);
    if(I2C_CheckEvent_Timeout(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) return;
    I2C_GenerateSTOP(I2C1, ENABLE);
}

/* ---- DMA config (was dma.c) ---- */
static void DMA_Config_Init(void) {
    DMA_InitTypeDef DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    DMA_DeInit(DMA1_Channel7);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&I2C1->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)MPU_RawData;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = 14;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel7, &DMA_InitStructure);

    DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void DMA1_Channel7_IRQHandler(void) {
    if (DMA_GetITStatus(DMA1_IT_TC7)) {
        I2C_GenerateSTOP(I2C1, ENABLE);
        I2C_DMACmd(I2C1, DISABLE);
        MPU_DataReady = 1;
        DMA_ClearITPendingBit(DMA1_IT_TC7);
    }
}

/* ---- MPU6050 init ---- */
void MPU6050_I2C_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef I2C_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 1000000;

    I2C_Init(I2C1, &I2C_InitStructure);
    I2C_Cmd(I2C1, ENABLE);
}

void MPU6050_INT_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource12);

    EXTI_InitStructure.EXTI_Line = EXTI_Line12;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void MPU6050_Init(void) {
    for(int i=0; i<0xFFFF; i++);
    MPU6050_WriteReg(0x6B, 0x80);
    for(int i=0; i<0xFFFF; i++);
    MPU6050_WriteReg(0x6B, 0x01);
    MPU6050_WriteReg(0x6C, 0x00);
    MPU6050_WriteReg(0x19, 0x07);
    MPU6050_WriteReg(0x1A, 0x00);
    MPU6050_WriteReg(0x1B, 0x18);
    MPU6050_WriteReg(0x1C, 0x00);
    MPU6050_WriteReg(0x37, 0x30);
    MPU6050_WriteReg(0x38, 0x01);
}

void MPU6050_Start_DMA_Read(void) {
    DMA_Cmd(DMA1_Channel7, DISABLE);
    DMA_SetCurrDataCounter(DMA1_Channel7, 14);
    DMA_Cmd(DMA1_Channel7, ENABLE);

    I2C_DMALastTransferCmd(I2C1, ENABLE);

    I2C_GenerateSTART(I2C1, ENABLE);
    if(I2C_CheckEvent_Timeout(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) return;
    I2C_Send7bitAddress(I2C1, MPU6050_ADDR, I2C_Direction_Transmitter);
    if(I2C_CheckEvent_Timeout(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) return;
    I2C_SendData(I2C1, 0x3B);
    if(I2C_CheckEvent_Timeout(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) return;

    I2C_GenerateSTART(I2C1, ENABLE);
    if(I2C_CheckEvent_Timeout(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) return;
    I2C_Send7bitAddress(I2C1, MPU6050_ADDR, I2C_Direction_Receiver);
    if(I2C_CheckEvent_Timeout(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) return;

    I2C_DMACmd(I2C1, ENABLE);
}

void MPU6050_Read_orgin(void)
{
    AccX  = (int16_t)(MPU_RawData[0] << 8 | MPU_RawData[1]);
    AccY  = (int16_t)(MPU_RawData[2] << 8 | MPU_RawData[3]);
    AccZ  = (int16_t)(MPU_RawData[4] << 8 | MPU_RawData[5]);
    Temp  = (int16_t)(MPU_RawData[6] << 8 | MPU_RawData[7]);
    GyroX = (int16_t)(MPU_RawData[8] << 8 | MPU_RawData[9]);
    GyroY = (int16_t)(MPU_RawData[10] << 8 | MPU_RawData[11]);
    GyroZ = (int16_t)(MPU_RawData[12] << 8 | MPU_RawData[13]);

    ax = (AccX ) * 6.1035e-5f;
    ay = (AccY ) * 6.1035e-5f;
    az = (AccZ ) * 6.1035e-5f;

    gx = (GyroX - 136) * 6.1035e-2f;
    gy = (GyroY + 56) * 6.1035e-2f;
    gz = (GyroZ + 2) * 6.1035e-2f;

    t = Temp * 1.0f;

    float dt = 0.001f;
    float Pitch_g = Pitch + gy * dt;
    float Roll_g  = Roll  + gx * dt;
    float Pitch_a = qatan2(ax, az) * 180.0f / 3.1415926f;
    float Roll_a  = qatan2(ay, az) * 180.0f / 3.1415926f;

    float alpha = 0.95f;
    Pitch = alpha * Pitch_g + (1.0f - alpha) * Pitch_a;
    Roll  = alpha * Roll_g  + (1.0f - alpha) * Roll_a;
    Yaw  += gz * dt;
}

/* ---- Getters ---- */
int16_t MPU6050_GetAccX(void)  { return AccX; }
int16_t MPU6050_GetAccY(void)  { return AccY; }
int16_t MPU6050_GetAccZ(void)  { return AccZ; }
int16_t MPU6050_GetGyroX(void) { return GyroX; }
int16_t MPU6050_GetGyroY(void) { return GyroY; }
int16_t MPU6050_GetGyroZ(void) { return GyroZ; }
int16_t MPU6050_GetTemp(void)  { return Temp; }

float MPU6050_Getax(void)  { return ax; }
float MPU6050_Getay(void)  { return ay; }
float MPU6050_Getaz(void)  { return az; }
float MPU6050_Getgx(void)  { return gx; }
float MPU6050_Getgy(void)  { return gy; }
float MPU6050_Getgz(void)  { return gz; }
float MPU6050_Gett(void)   { return t; }
float MPU6050_GetPitch(void) { return Pitch; }
float MPU6050_GetRoll(void)  { return Roll; }
float MPU6050_GetYaw(void)   { return Yaw; }

/* ---- ISR ---- */
void EXTI15_10_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line12) != RESET) {
        MPU6050_Start_DMA_Read();
        EXTI_ClearITPendingBit(EXTI_Line12);
    }
}

/* ---- interface implementation ---- */
static int mpu6050_init(void) {
    I2C_SoftwareResetCmd(I2C1, ENABLE);
    I2C_SoftwareResetCmd(I2C1, DISABLE);
    MPU6050_I2C_Init();
    DMA_Config_Init();
    MPU6050_Init();
    MPU6050_INT_Init();
    return 0;
}

static int mpu6050_read(imu_data_t *data) {
    if (!MPU_DataReady) return -1;
    MPU_DataReady = 0;

    MPU6050_Read_orgin();

    data->acc_x   = AccX;
    data->acc_y   = AccY;
    data->acc_z   = AccZ;
    data->gyro_x  = GyroX;
    data->gyro_y  = GyroY;
    data->gyro_z  = GyroZ;
    data->accel_x = ax;
    data->accel_y = ay;
    data->accel_z = az;
    data->gyro_rx = gx;
    data->gyro_ry = gy;
    data->gyro_rz = gz;
    data->pitch   = Pitch;
    data->roll    = Roll;
    data->yaw     = Yaw;
    data->temp    = t;
    return 0;
}

const imu_driver_t mpu6050_driver = {
    .init = mpu6050_init,
    .read = mpu6050_read,
};
