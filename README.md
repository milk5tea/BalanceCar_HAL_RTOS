## 完整 README 内容

```markdown
# 平衡车（STM32F103 + HAL 库 + FreeRTOS）

基于江协科技标准库版本的平衡车，重构为 **HAL 库 + CubeMX + CMake + FreeRTOS** 架构。

> 重构过程中解决了 11 个关键问题，包括蓝牙零丢包接收、波形 DMA 非阻塞发送、DWT 做 HAL 时基等。详见 [`docs/技术报告.md`](docs/技术报告.md)。

---

## 特性

- ✅ **三环串级 PID**（角度环 + 速度环 + 转向环）
- ✅ **蓝牙实时调参** + DMA 波形调试（并行工作，互不干扰）
- ✅ **DMA + IDLE 零丢包蓝牙接收**（老 HAL 库上复刻新 API 功能）
- ✅ **位置环、前馈补偿**（预留 `#if` 开关，一键启用）
- ✅ **FreeRTOS 集成**（DWT 做 HAL 时基，不占外设定时器）
- ✅ **栈溢出 / 堆不足双频率 LED 报警**

---

## 硬件清单

| 项目 | 型号 |
|---|---|
| MCU | STM32F103C8T6 |
| 姿态传感器 | MPU6050（软件 I2C） |
| 电机驱动 | TB6612 |
| 电机 | 直流减速电机（9.27666 减速比） |
| 编码器 | 霍尔编码器（TIM3 / TIM4 Encoder 模式） |
| 显示 | OLED 0.96"（软件 I2C） |
| 蓝牙 | HC-04 BLE（USART2, 9600bps） |
| 调试串口 | USB-TTL（USART1, 115200bps） |

---

## 引脚分配

| 功能 | 引脚 | 说明 |
|---|---|---|
| MPU6050 SCL | PB10 | 软件 I2C |
| MPU6050 SDA | PB11 | 软件 I2C |
| OLED SCL | PB8 | 软件 I2C |
| OLED SDA | PB9 | 软件 I2C |
| 左电机 PWM | PA0 | TIM2_CH1 |
| 左电机 AIN1 / AIN2 | PB12 / PB13 | 方向控制 |
| 右电机 PWM | PA1 | TIM2_CH2 |
| 右电机 BIN1 / BIN2 | PB14 / PB15 | 方向控制 |
| 左编码器 A / B | PA6 / PA7 | TIM3 Encoder |
| 右编码器 A / B | PB6 / PB7 | TIM4 Encoder |
| 蓝牙 TX / RX | PA2 / PA3 | USART2 |
| 调试 TX / RX | PA9 / PA10 | USART1 |
| LED | PC13 | 状态指示 / 报警 |

---

## 目录结构

```
.
├── Algorithm/                  控制算法
│   ├── Filter/                 互补滤波
│   └── PID/                    微分先行 + 条件积分
│
├── Core/                       CubeMX 生成的核心代码
│   ├── Inc/                    头文件
│   └── Src/                    main.c / freertos.c / it.c 等
│
├── Drivers/                    ST 官方 HAL 库 + CMSIS
│
├── Hardware/                   自定义硬件驱动
│   ├── MyI2C/                  软件 I2C 底层
│   ├── MPU6050/                姿态传感器
│   ├── OLED/                   0.96" 显示
│   ├── Motor/                  电机 PWM + 方向
│   ├── PWM/                    PWM 输出
│   ├── Encoder/                编码器读取
│   ├── Key/                    按键
│   ├── Serial/                 USART1 调试串口
│   └── BlueSerial/             蓝牙 DMA + IDLE 接收
│
├── Middlewares/                FreeRTOS
│
├── Tasks/task_main/            三环 PID + 任务调度
│
├── cmake/                      CMake 构建配置
├── CMakeLists.txt
├── CMakePresets.json
└── GPIO_test.ioc               CubeMX 工程
```

---

## 系统架构

```
┌─────────────────────────────────────────────┐
│ 1ms 硬件定时器中断（TIM1）                    │
│  ├── 每 10 次分频 → 10ms 任务：               │
│  │      姿态读取 + 互补滤波 + 角度环 PID      │
│  │      + 电机 PWM 输出                       │
│  └── 每 50 次分频 → 50ms 任务：               │
│         编码器读取 + 速度环 PID + 转向环 PID  │
├─────────────────────────────────────────────┤
│ 主循环（FreeRTOS：ControlTask）               │
│  ├── 按键处理                                 │
│  ├── 蓝牙调参（DMA 接收，零阻塞）             │
│  ├── OLED 显示（100ms 限频）                  │
│  └── 波形打印（DMA 发送，50ms 一次）          │
└─────────────────────────────────────────────┘
```

**核心设计**：控制实时性（10ms / 50ms）和用户交互（调参 / 显示 / 波形）**彻底解耦**。

---

## 三环串级结构

```
摇杆 LV ─→ SpeedPID.Target
              ↓
         SpeedPID.Out ──→ AnglePID.Target
                            ↓
                       AnglePID.Out ──→ AvePWM
                                          ↓
摇杆 RH ─→ TurnPID.Target → TurnPID.Out → DifPWM
                                          ↓
                        AvePWM ± DifPWM/2 → 左右电机
```

| 环 | 周期 | 作用 |
|---|---|---|
| 角度环 | 10ms | 车姿态 → PWM |
| 速度环 | 50ms | 车速 → 角度目标 |
| 转向环 | 50ms | 左右轮差速 → 差速 PWM |

---

## 关键技术实现

### 1. 蓝牙零丢包接收（DMA + IDLE）

使用 **DMA Circular 模式 + IDLE 中断**，在老 HAL 库上复刻 `HAL_UARTEx_ReceiveToIdle_DMA` 的功能。

详见 [`Hardware/BlueSerial/BlueSerial.c`](Hardware/BlueSerial/BlueSerial.c)。

### 2. 波形 DMA 非阻塞发送

`BlueSerial_Printf_DMA` 用 DMA 发送波形，主循环开销从 **30ms 降到 < 1ms**。

**效果**：波形调试和蓝牙调参**同时可用**。

### 3. DWT 做 HAL 时基（RTOS 版本）

FreeRTOS 占用 SysTick 后，用 Cortex-M3 内核的 DWT 周期计数器做 HAL 时基：

```c
uint32_t HAL_GetTick(void)
{
    return DWT->CYCCNT / (SystemCoreClock / 1000);
}
```

**好处**：不占任何外设定时器，精度比 TIM 高。

### 4. 中断优先级分层

| 中断 | 优先级 | 说明 |
|---|---|---|
| USART2 | 0 | 蓝牙接收最高 |
| TIM1 | 1 | 1ms 控制中断 |
| USART1 | 1 | 调试串口 |
| DMA1_CH6/CH7 | 1 | 蓝牙收发 DMA |

### 5. 双频率 LED 报警（RTOS 版本）

- **慢闪**（1 秒）= 栈溢出；
- **快闪**（200ms）= 堆不足。

不需要调试器，看灯就知道问题类型。

---

## 编译方法

### 依赖

- **arm-none-eabi-gcc** 13.3+
- **CMake** 3.22+
- **Ninja**
- **STM32CubeMX**（可选，用于修改外设配置）

### 编译

```bash
# 配置
cmake --preset Debug

# 编译
cmake --build build/Debug
```

编译产物：`build/Debug/GPIO_test.elf`

### 烧录

用 STM32CubeProgrammer 或 J-Flash 烧录 `.elf` 文件。

### 调试

用 SEGGER Ozone 或 VSCode + Cortex-Debug 插件。

---

## 使用说明

### 按键

| 按键 | 功能 |
|---|---|
| K1 | 启动 / 停止 PID |
| K2 | 重置位置环原点（位置环启用时有效） |

### 蓝牙调参

手机端通过蓝牙串口助手发送数据包：

| 数据包 | 说明 |
|---|---|
| `[slider,AngleKp,3.0]` | 修改角度环 Kp |
| `[slider,SpeedKi,0.05]` | 修改速度环 Ki |
| `[joystick,0,127,0,0]` | 摇杆前后推到底 |
| `[joystick,0,0,127,0]` | 摇杆向右推到底 |

支持的滑杆参数：`AngleKp`、`AngleKi`、`AngleKd`、`Offset`、`SpeedKp`、`SpeedKi`、`SpeedKd`、`TurnKp`、`TurnKi`、`TurnKd`（位置环启用后加 `PosKp`、`PosKi`、`PosKd`）

### 波形调试

蓝牙串口助手切到"绘图"页面，波形数据格式：

```
[plot,SpeedPID.Target,AveSpeed]
```

红线 = 目标速度，绿线 = 实际速度。

---

## 性能指标

| 维度 | 标准库版 | HAL 重构版 |
|---|---|---|
| 基础时基 | 1ms 硬件定时器 | 1ms 硬件定时器 |
| 角度环周期 | 10ms | 10ms |
| 速度/转向环周期 | 50ms | 50ms |
| 蓝牙丢包 | 偶发 | **零丢包** |
| 波形阻塞 | 30ms | **< 1ms** |
| 编码器高速读数 | 偏小 | **准确** |
| 调试并行 | 二选一 | **波形 + 调参同时** |
| 可移植性 | 换芯片大改 | **CubeMX 重新生成** |

---

## 已知问题与解决方案

见 [`docs/技术报告.md`](docs/技术报告.md)，包括：

1. 控制环放中断还是主循环
2. 中断优先级冲突导致蓝牙丢包
3. 编码器高速时读数偏小
4. 蓝牙接收偶发卡死（5 种方案对比）
5. 波形打印阻塞主循环
6. 启动时角度跳变
7. OLED 刷新拖慢主循环
8. 摇杆数据"漂移"（误判）
9. FreeRTOS 集成（HAL 时基冲突）
10. FreeRTOS 栈溢出
11. `configUSE_TIMERS` 与 `xTimerPendFunctionCall` 冲突

---

## 后续规划

- [ ] 位置环（四环串级）
- [ ] 前馈补偿
- [ ] 卡尔曼滤波替换互补滤波
- [ ] 加入 RM 电控组通信任务
- [ ] Python 上位机（实时波形 + 调参面板）

---

## 许可证

MIT License

---

## 参考

- 江协科技标准库版本：本项目重构前的原始版本
- STM32F1 HAL 库文档
- FreeRTOS 官方文档
```
5. **Commit changes**。

改完刷新页面，README 会渲染成漂亮的格式。截图发我看看效果。
