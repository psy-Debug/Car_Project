# 架构文档 — Car Project

> STM32F103 四层嵌入式控制框架 | 版本 2.0

---

## 一、整体架构

```
┌─────────────────────────────────────────────────────────┐
│  App (应用层)                                            │
│  main.c — 系统初始化 + 主循环（采集→控制→输出→显示）      │
│  只依赖: 接口头文件 + service 层                          │
├─────────────────────────────────────────────────────────┤
│  Service (服务层)                                        │
│  pid        纯 PID 算法，不碰寄存器                        │
│  qmath      正弦/正切查表，不碰寄存器                      │
│  scheduler  5ms 定时调度，碰 TIM4 寄存器（职责单一）        │
│  protocol   蓝牙 `{命令:值}` 帧解析，不碰 USART 寄存器     │
├─────────────────────────────────────────────────────────┤
│  Driver (驱动层)                                         │
│  每个驱动实现一个 interface (位于 driver/include/)         │
│  ┌──────────┬──────────────┬────────────────┐          │
│  │ imu      │ mpu6050      │ imu_driver_t   │          │
│  │ encoder  │ encoder      │ encoder_driver_t│         │
│  │ motor    │ motor+tb6612 │ motor_driver_t │          │
│  │ display  │ oled+data    │ display_driver_t│         │
│  │ power    │ battery      │ power_driver_t │          │
│  │ serial   │ usart1/2/3   │ serial_driver_t│          │
│  │ gpio     │ key, led     │ (直接接口)      │          │
│  └──────────┴──────────────┴────────────────┘          │
│  驱动之间互不引用                                        │
├─────────────────────────────────────────────────────────┤
│  sys (系统基础设施)                                       │
│  delay/delay1  SysTick 延时时基                           │
│  stm32f10x_it  系统异常 + SysTick_Handler                │
│  stm32f10x_conf 外设库 include 开关                       │
├─────────────────────────────────────────────────────────┤
│  lib (ST 固件包)                                         │
│  CMSIS         ARM 内核 + 寄存器定义 + 启动文件            │
│  StdPeriph     GPIO/USART/TIM/I2C/ADC/DMA 外设驱动       │
└─────────────────────────────────────────────────────────┘
```

### 依赖方向（单向 ↓）

```
App → Service → Driver → lib/sys
         ↓                    ↓
    不碰寄存器              只碰自己负责的寄存器
```

- App 不知道下面跑的是 MPU6050 还是 BMI160
- Service 不包含任何 `stm32f10x.h`
- Driver 之间不互相 `#include`
- lib 是完全独立的固件包，可替换

---

## 二、各层文件清单

### App 层

| 文件 | 职责 |
|------|------|
| `app/main.c` | 初始化所有驱动 → 进入主循环 |

### Service 层

| 模块 | 文件 | 依赖 |
|------|------|------|
| pid | `service/pid/pid.c .h` | `qmath.h`, `<math.h>` |
| qmath | `service/math/qmath.c .h` | `<math.h>` 查表数据 |
| scheduler | `service/scheduler/timer_scheduler.c .h` | `stm32f10x.h` (仅 TIM4) |
| protocol | `service/protocol/protocol.c .h` | `pid.h`, `<stdlib.h>`, `<string.h>` |

### Driver 层

| 模块 | 文件 | 实现接口 | 硬件资源 |
|------|------|----------|----------|
| imu | `driver/imu/mpu6050.c .h` | `imu_driver_t` | I2C1( PB6/PB7 ) + DMA1_CH7 + EXTI12( PA12 ) |
| encoder | `driver/encoder/encoder.c .h` | `encoder_driver_t` | TIM2( PA0/PA1 ) + TIM3( PA6/PA7 ) |
| motor | `driver/motor/motor.c .h` | `motor_driver_t` | — |
| └ tb6612 | `driver/motor/tb6612.c .h` | (内部) | PA5+PB12/13/14/5 |
| └ pwm | `driver/motor/motor_pwm.c .h` | (内部) | TIM1_CH1( PA8 ) + TIM1_CH4( PA11 ) |
| display | `driver/display/oled.c .h` | `display_driver_t` | 软 I2C( PB8/PB9 ) |
| └ data | `driver/display/oled_data.c .h` | (字库) | — |
| power | `driver/power/battery.c .h` | `power_driver_t` | ADC1( PA4 ) + DMA1_CH1 |
| serial | `driver/serial/usart1.c .h` | `serial_driver_t` | USART1( PA9/PA10 ) |
| └ | `driver/serial/usart2.c .h` | `serial_driver_t` | USART2( PA2/PA3 ) |
| └ | `driver/serial/usart3.c .h` | `serial_driver_t` | USART3( PB10/PB11 ) |
| gpio | `driver/gpio/key.c .h` | 直接调用 | PB1/PB11 |
| └ | `driver/gpio/led.c .h` | 直接调用 | PA1/PA2 |

### Interface 定义

| 文件 | 包含 |
|------|------|
| `driver/include/imu_if.h` | `imu_data_t` 结构体 + `imu_driver_t{init, read}` |
| `driver/include/encoder_if.h` | `encoder_channel_t` 结构体 + `encoder_driver_t{init, get_left, get_right}` |
| `driver/include/motor_if.h` | `motor_driver_t{init, set_output}` |
| `driver/include/display_if.h` | `display_driver_t{init, clear, update, show_float}` |
| `driver/include/power_if.h` | `power_driver_t{init, get_voltage}` |
| `driver/include/serial_if.h` | `serial_driver_t{init, send_string, send_bytes, poll_recv}` |

### sys 层

| 文件 | 职责 |
|------|------|
| `sys/delay.c .h` | 阻塞式 SysTick 延时 (Delay_us/ms/s) |
| `sys/delay1.c .h` | 非阻塞 SysTick 时基 (GetUs/GetTick/Delay) |
| `sys/stm32f10x_it.c .h` | 系统异常 + SysTick_Handler |
| `sys/stm32f10x_conf.h` | ST 外设库 include 开关 |

### lib 层（ST 固件包）

| 目录 | 内容 |
|------|------|
| `lib/CMSIS/` | `core_cm3.c/.h` `stm32f10x.h` `system_stm32f10x.c/.h` + 8 个 startup 文件 |
| `lib/STM32F10x_StdPeriph_Driver/inc/` | 23 个外设头文件 |
| `lib/STM32F10x_StdPeriph_Driver/src/` | 23 个外设源文件 |

---

## 三、中断向量表（ISR 全局一览）

| ISR 名称 | 文件 | 功能 |
|----------|------|------|
| `Reset_Handler` | `lib/CMSIS/startup/startup_xxx.s` | 上电入口，调用 SystemInit→main |
| `NMI_Handler` | `sys/stm32f10x_it.c` | 不可屏蔽中断 |
| `HardFault_Handler` | `sys/stm32f10x_it.c` | 硬件错误（死循环） |
| `MemManage_Handler` | `sys/stm32f10x_it.c` | MPU 违规 |
| `BusFault_Handler` | `sys/stm32f10x_it.c` | 总线错误 |
| `UsageFault_Handler` | `sys/stm32f10x_it.c` | 未定义指令 |
| `SVC_Handler` | `sys/stm32f10x_it.c` | 系统服务调用 |
| `DebugMon_Handler` | `sys/stm32f10x_it.c` | 调试监视 |
| `PendSV_Handler` | `sys/stm32f10x_it.c` | 可挂起系统调用 |
| `SysTick_Handler` | `sys/stm32f10x_it.c` | 每 1ms 递增 ulTicks |
| `EXTI15_10_IRQHandler` | `driver/imu/mpu6050.c` | MPU6050 INT 引脚 → 启动 DMA 读取 |
| `DMA1_Channel7_IRQHandler` | `driver/imu/mpu6050.c` | DMA 传输完成 → 设置 MPU_DataReady |
| `TIM2_IRQHandler` | `driver/encoder/encoder.c` | 左轮编码器脉冲捕获 |
| `TIM3_IRQHandler` | `driver/encoder/encoder.c` | 右轮编码器脉冲捕获 |
| `TIM4_IRQHandler` | `service/scheduler/timer_scheduler.c` | 5ms 周期 → 设置 PID_Task_Flag |
| `USART2_IRQHandler` | `driver/serial/usart2.c` | 蓝牙数据接收 → protocol_feed_byte |

**链接原理**：startup 文件中每个 ISR 声明为 `WEAK`，默认指向空函数。当你写的 .c 文件定义了同名函数，链接器用强定义覆盖弱定义，向量表中填入你的函数地址。

---

## 四、上电启动流程

```
上电 / 复位
    │
    ▼
硬件: PC ← 0x08000004 (向量表 Reset_Handler 入口)
    │
    ▼
startup.s: Reset_Handler
    ├── 调用 SystemInit() — 配置 HCLK=72MHz, APB1=36MHz, APB2=72MHz
    ├── 调用 __main()   — 初始化 .data / .bss 段（全局变量赋值/清零）
    └── 调用 main()
           │
           ▼
       NVIC_PriorityGroupConfig()
       oled->init()     → 软 I2C 初始化 + OLED 命令序列
       debug->init()    → USART1 115200 调试串口
       bt->init()       → USART2 9600 蓝牙串口 + 中断使能
       motor->init()    → TIM1 PWM + TB6612 GPIO
       encoder->init()  → TIM2/TIM3 编码器模式 + 中断使能
       imu->init()      → I2C1 + DMA1_CH7 + EXTI12 + MPU6050 寄存器配置
       battery->init()  → ADC1 + DMA1_CH1 连续采样电池电压
       Timer4_Init()    → TIM4 5ms 周期中断
       PID_Init()
           │
           ▼
       while(1) {
           if (imu->read())     → 姿态数据更新
           if (PID_Task_Flag)   → PID 计算 → motor->set_output()
           protocol_poll()      → 蓝牙命令解析
           oled 刷新
       }
```

---

## 五、6 个 Driver Interface 详细定义

### 5.1 IMU 接口 (`imu_if.h`)

```c
typedef struct {
    int16_t acc_x,  acc_y,  acc_z;   // 原始值
    int16_t gyro_x, gyro_y, gyro_z;
    float   ax, ay, az;              // 物理单位
    float   gx, gy, gz;
    float   pitch, roll, yaw;
    float   temp;
} imu_data_t;

typedef struct {
    int  (*init)(void);              // 0=成功
    int  (*read)(imu_data_t *data);  // 0=新数据可用，-1=无数据
} imu_driver_t;
```

### 5.2 编码器接口 (`encoder_if.h`)

```c
typedef struct {
    float   speed;  // rad/s
    float   angle;  // 累计角度
    int64_t count;  // 累计脉冲
} encoder_channel_t;

typedef struct {
    int  (*init)(void);
    void (*get_left)(encoder_channel_t *ch);
    void (*get_right)(encoder_channel_t *ch);
} encoder_driver_t;
```

### 5.3 电机接口 (`motor_if.h`)

```c
typedef struct {
    int  (*init)(void);
    void (*set_output)(int16_t pwm_l, int16_t pwm_r); // -7200 ~ +7200
} motor_driver_t;
```

### 5.4 显示接口 (`display_if.h`)

```c
typedef struct {
    int  (*init)(void);
    void (*clear)(void);
    void (*update)(void);
    void (*show_float)(int16_t x, int16_t y, double num,
                       uint8_t int_len, uint8_t fra_len, uint8_t font);
} display_driver_t;
```

### 5.5 电源接口 (`power_if.h`)

```c
typedef struct {
    int   (*init)(void);
    float (*get_voltage)(void);
} power_driver_t;
```

### 5.6 串口接口 (`serial_if.h`)

```c
typedef struct {
    int  (*init)(uint32_t baudrate);
    void (*send_string)(const char *str);
    void (*send_bytes)(const uint8_t *buf, uint16_t len);
    int  (*poll_recv)(uint8_t *buf, uint16_t max_len); // 返回收了多少字节
} serial_driver_t;
```

---

## 六、如何添加新驱动（以 IMU 为例）

假设要接一个 BMI160 替代 MPU6050：

### Step 1：写驱动

```c
// driver/imu/bmi160.h
#include "stm32f10x.h"
#include "imu_if.h"
extern const imu_driver_t bmi160_driver;
// ... bmi160 私有函数声明

// driver/imu/bmi160.c
#include "bmi160.h"
// ... SPI 初始化、寄存器读写、数据解析 ...

static int bmi160_init(void)  { /* 初始化 SPI + BMI160 寄存器 */ return 0; }
static int bmi160_read(imu_data_t *d) { /* 读数据填充 d */ return 0; }

const imu_driver_t bmi160_driver = {
    .init = bmi160_init,
    .read = bmi160_read,
};
```

### Step 2：替换引用

```c
// app/main.c — 只改这一行
extern const imu_driver_t bmi160_driver;
static const imu_driver_t *imu = &bmi160_driver;
```

### 其他文件零改动

`PID_Calc_Vertical(roll, gyro_x)` → 不变
`encoder->get_left()` → 不变
`motor->set_output()` → 不变

因为 main.c 只通过接口访问 IMU，不知道下面是 MPU6050 还是 BMI160。

---

## 七、旧代码到新代码的映射

| 旧文件 (hardware/) | 新文件 |
|-------------------|--------|
| `hardware/MPU6050.c` + `hardware/DMA.c` | `driver/imu/mpu6050.c` (合并) |
| `hardware/encoder.c` | `driver/encoder/encoder.c` |
| `hardware/motor_control.c` | `driver/motor/motor.c` |
| `hardware/tb6612.c` | `driver/motor/tb6612.c` |
| `hardware/motor_pwm.c` | `driver/motor/motor_pwm.c` |
| `hardware/OLED.c` | `driver/display/oled.c` |
| `hardware/OLED_Data.c` | `driver/display/oled_data.c` |
| `hardware/battery.c` | `driver/power/battery.c` |
| `hardware/usart1.c` | `driver/serial/usart1.c` |
| `hardware/usart2.c` | `driver/serial/usart2.c` |
| `hardware/usart3.c` | `driver/serial/usart3.c` |
| `hardware/k.c` | `driver/gpio/key.c` |
| `hardware/led.c` | `driver/gpio/led.c` |
| `hardware/pid.c` | `service/pid/pid.c` |
| `hardware/qmath.c` | `service/math/qmath.c` |
| `hardware/timer_scheduler.c` | `service/scheduler/timer_scheduler.c` |
| — (从 usart2.c 抽出) | `service/protocol/protocol.c` |
| `main.c` (根目录) | `app/main.c` |
| `system/Delay.c .h` | `sys/delay.c .h` |
| `system/delay1.c .h` | `sys/delay1.c .h` |
| `user/stm32f10x_it.c .h` | `sys/stm32f10x_it.c .h` |
| `user/stm32f10x_conf.h` | `sys/stm32f10x_conf.h` |

### 新相对旧的关键改变

| 改动 | 旧 | 新 |
|------|----|----|
| DMA 合并 | DMA.c 独立文件 | 并入 mpu6050.c，消除双向依赖 |
| MPU6050→OLED 耦合 | `#include "OLED.h"` | 删除 |
| PID 全局变量 | `extern float Target_Speed_Raw` | 改为参数传入 `target_offset` |
| 调度器耦合 | `#include "encoder.h" "usart1.h"` | 删除 |
| 协议分离 | 混在 usart2.c 中 | 独立 `protocol.c` |
| 接口抽象 | 直接调用 `MPU6050_GetRoll()` | 通过 `imu->read(&data)` |
