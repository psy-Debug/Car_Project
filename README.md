# Car Project — STM32F103 两轮自平衡小车

> 四层可移植嵌入式控制框架 | Cortex-M3 裸机

---

## 快速开始

1. **Keil 打开项目**：添加 `app/` `driver/` `service/` `sys/` `lib/` 到工程
2. **include paths**：`app` `driver/include` `driver/imu` `driver/encoder` `driver/motor` `driver/display` `driver/power` `driver/serial` `driver/gpio` `service/pid` `service/math` `service/scheduler` `service/protocol` `sys` `lib/CMSIS` `lib/STM32F10x_StdPeriph_Driver/inc`
3. **编译** → **烧录** → 上电即平衡

---

## 目录结构

```
Car_Project/
├── app/       main.c                    ← 应用层（系统初始化和控制循环）
├── driver/
│   ├── include/  imu_if.h encoder_if.h   ← 6 个统一接口
│   │             motor_if.h display_if.h
│   │             power_if.h serial_if.h
│   ├── imu/      mpu6050.c/h            ← IMU 驱动（DMA 已合并）
│   ├── encoder/  encoder.c/h            ← 编码器驱动
│   ├── motor/    motor.c/h + tb6612 + pwm ← 电机驱动组
│   ├── display/  oled.c/h + data + font  ← OLED 显示驱动
│   ├── power/    battery.c/h            ← 电池电压检测
│   ├── serial/   usart1/2/3.c/h         ← 串口驱动
│   └── gpio/     key.c/h   led.c/h      ← GPIO 驱动
├── service/
│   ├── pid/       pid.c/h              ← PID 控制算法
│   ├── math/      qmath.c/h            ← 快速数学库（查表法）
│   ├── scheduler/ timer_scheduler.c/h   ← 5ms 定时调度
│   └── protocol/  protocol.c/h         ← 蓝牙遥控协议解析
├── sys/
│   ├── delay.c/h + delay1.c/h           ← SysTick 时基
│   ├── stm32f10x_it.c/h                ← 系统异常处理
│   └── stm32f10x_conf.h               ← 外设库配置
├── lib/
│   ├── CMSIS/     ARM Cortex-M3 内核 + 寄存器定义 + 启动文件
│   └── STM32F10x_StdPeriph_Driver/      ST 标准外设驱动库
└── doc/          架构文档
```

---

## 架构分层

```
App (main.c)
  │  只引用: imu_if.h, encoder_if.h, motor_if.h,
  │          display_if.h, power_if.h, serial_if.h,
  │          pid.h, protocol.h, scheduler.h
  ▼
Service (pid, qmath, scheduler, protocol)
  │  纯算法/纯协议，不包含任何 stm32f10x.h
  ▼
Driver (mpu6050, encoder, motor, oled, battery, usart, key, led)
  │  操作寄存器，每个模块实现对应的 _if.h 接口
  ▼
lib (CMSIS + StdPeriph_Driver)
  │  ST 固件包，不包含任何业务逻辑
  ▼
sys (delay, stm32f10x_it, stm32f10x_conf)
  系统基础设施，被 driver 和 service 共用
```

**核心规则**：依赖方向严格单向 ↓，上层不直接引用下层具体实现。

---

## 如何替换驱动（以 IMU 为例）

1. 写一个 `driver/imu/bmi160.c`，实现 `imu_driver_t` 接口
2. 在 `app/main.c` 中改一行：

```c
// 原来
extern const imu_driver_t mpu6050_driver;
static const imu_driver_t *imu = &mpu6050_driver;

// 改成
extern const imu_driver_t bmi160_driver;
static const imu_driver_t *imu = &bmi160_driver;
```

**应用层代码一行不用改。**
