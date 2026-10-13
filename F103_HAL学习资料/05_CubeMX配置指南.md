# STM32F103C8T6 CubeMX 配置指南

## 一、新建工程

1. 打开 STM32CubeMX
2. **File → New Project**
3. 芯片选择: **STM32F103C8** (在MCU/MPU Selector中搜索)
4. 创建工程后，进入配置界面

## 二、时钟树配置

### 目标: 系统主频 72MHz

```
Pinout & Configuration → System Core → RCC
```

| 参数 | 设置 | 说明 |
|------|------|------|
| High Speed Clock (HSE) | Crystal/Ceramic Resonator | 外部晶振 8MHz |
| Low Speed Clock (LSE) | Disable | 不使用RTC时关闭 |
| PLL Source | HSE | 使用外部时钟 |
| PLLMUL | ×9 | 8MHz × 9 = 72MHz |
| SYSCLK | PLLCLK | 主时钟源 |
| AHB | /1 | 72MHz |
| APB1 | /2 | 36MHz (不能超过36MHz) |
| APB2 | /1 | 72MHz |

**注意**: APB1 时钟不能超过 36MHz，TIM2-TIM7 的时钟为 APB1×2 = 72MHz。

### 无外部晶振时 (HSI)
如果开发板没有外部晶振，使用 HSI:
- HSI: 8MHz (内部RC)
- PLLMUL: ×9
- SYSCLK: 72MHz

## 三、GPIO 配置

### LED 示例 (PC13)
```
Pinout → 点击 PC13 → GPIO_Output
```

参数设置:
| 参数 | 值 |
|------|-----|
| GPIO output level | High (默认灭) |
| GPIO mode | Output Push Pull |
| Pull-up/Pull-down | No pull-up and no pull-down |
| Maximum output speed | Low |
| User Label | LED |

### 按键示例 (PA0)
```
Pinout → 点击 PA0 → GPIO_Input
```

参数设置:
| 参数 | 值 |
|------|-----|
| GPIO mode | Input mode |
| Pull-up/Pull-down | Pull-up |
| User Label | KEY |

### 外部中断示例 (PA0)
```
Pinout → 点击 PA0 → EXTI → EXTI mode with falling edge trigger detection
```

参数设置:
| 参数 | 值 |
|------|-----|
| GPIO mode | External Interrupt Mode with Falling edge trigger |
| Pull-up/Pull-down | Pull-up |

NVIC设置:
```
System Core → NVIC → 勾选 EXTI line0 interrupt
Preemption Priority: 2
Sub Priority: 0
```

## 四、USART 配置

### USART1 (常用)
```
Connectivity → USART1 → Mode: Asynchronous
```

参数设置:
| 参数 | 值 |
|------|-----|
| Mode | Asynchronous |
| Hardware Flow Control | None |
| Baud Rate | 115200 |
| Word Length | 8 Bits |
| Parity | None |
| Stop Bits | 1 |
| Over Sampling | 16 Samples |

GPIO设置 (自动):
- PA9: USART1_TX (Alternate Function Push Pull)
- PA10: USART1_RX (Input Floating)

NVIC:
- 勾选 USART1 global interrupt

## 五、TIM 配置

### 基本定时器 (TIM2 定时中断)
```
Timers → TIM2 → Clock Source: Internal Clock
```

参数设置:
| 参数 | 值 | 说明 |
|------|-----|------|
| Prescaler | 71 | 72MHz/(71+1) = 1MHz |
| Counter Mode | Up | 向上计数 |
| Counter Period | 9999 | (9999+1) × 1μs = 10ms |
| Clock Division | DIV1 | |

计算: `定时时间 = (Period+1) × (Prescaler+1) / 72000000`

NVIC:
- 勾选 TIM2 global interrupt

### PWM 输出 (TIM3 CH1)
```
Timers → TIM3 → Channels: CH1 → PWM Generation CH1
```

参数设置:
| 参数 | 值 |
|------|-----|
| Prescaler | 71 |
| Counter Period | 999 |
| Counter Mode | Up |
| Pulse | 500 (初始占空比50%) |
| Channel | TIM_CHANNEL_1 |

GPIO自动配置:
- PA6: TIM3_CH1 (AF_PP)

### 常用定时器参数计算

| 目标 | Prescaler | Period | PWM频率 |
|------|-----------|--------|---------|
| 1ms中断 | 71 | 999 | - |
| 10ms中断 | 71 | 9999 | - |
| 1Hz中断 | 7199 | 9999 | - |
| 1kHz PWM | 71 | 999 | 1kHz |
| 50Hz PWM (舵机) | 71 | 19999 | 50Hz |

## 六、ADC 配置

### 单通道 (PA0)
```
Analog → ADC1 → IN0
```

参数设置:
| 参数 | 值 |
|------|-----|
| Resolution | 12 bits |
| Scan Conversion Mode | Disabled |
| Continuous Conversion Mode | Disabled |
| Discontinuous | Disabled |

GPIO自动配置:
- PA0: ADC1_IN0 (Analog mode)

### DMA传输 (进阶)
```
ADC1 → DMA Settings → Add
```

| 参数 | 值 |
|------|-----|
| DMA Request | ADC1 |
| Channel | Channel 0 |
| Direction | Peripheral-to-Memory |
| Priority | Low |
| Mode | Circular |
| Data Width | Half Word |

## 七、I2C 配置 (OLED示例)
```
Connectivity → I2C1 → Mode: I2C
```

参数设置:
| 参数 | 值 |
|------|-----|
| Speed Mode | Fast Mode Plus |
| Clock Speed (KHz) | 400 |

GPIO自动配置:
- PB6: I2C1_SCL
- PB7: I2C1_SDA

## 八、SPI 配置 (TFT/Flash示例)
```
Connectivity → SPI1 → Mode: Full-Duplex Master
```

参数设置:
| 参数 | 值 |
|------|-----|
| Data Size | 8 Bits |
| First Bit | MSB First |
| Prescaler | 16 |
| CPOL | Low |
| CPHA | 1 Edge |

## 九、调试接口

```
Debug → Serial Wire (Synchronous)
```

| 参数 | 值 |
|------|-----|
| Debug | Serial Wire |
| Serial Wire Clock | 4 MHz |

SWD接口使用: PA13 (SWDIO), PA14 (SWCLK)

## 十、生成代码

1. **Project → Generate Code** (Ctrl+S 也可)
2. 选择工程保存位置
3. 打开生成的工程进行编译

**重要**: 每次CubeMX修改后重新生成代码，你写在 `USER CODE BEGIN/END` 区域的代码会保留。

## 十一、常见外设引脚分配 (F103C8T6)

| 功能 | 引脚 | 说明 |
|------|------|------|
| LED | PC13 | 板载LED (低电平亮) |
| KEY | PA0 | 板载按键 |
| USART1_TX | PA9 | USB转串口 |
| USART1_RX | PA10 | USB转串口 |
| USART2_TX | PA2 | |
| USART2_RX | PA3 | |
| TIM2_CH1 | PA0 | 基本定时器 |
| TIM2_CH2 | PA1 | |
| TIM3_CH1 | PA6 | PWM输出 |
| TIM3_CH2 | PA7 | |
| TIM4_CH1 | PB6 | |
| TIM4_CH2 | PB7 | |
| ADC1_IN0 | PA0 | 模拟输入 |
| ADC1_IN1 | PA1 | |
| ADC1_IN4 | PA4 | |
| I2C1_SCL | PB6 | OLED/Sensor |
| I2C1_SDA | PB7 | |
| SPI1_SCK | PA5 | TFT/Flash |
| SPI1_MISO | PA6 | |
| SPI1_MOSI | PA7 | |
| CAN_TX | PB9 | |
| CAN_RX | PB8 | |
| USB_DM | PA11 | USB FS |
| USB_DP | PA12 | USB FS |

**注意**: PA0 同时连接板载按键和 TIM2_CH1/ADC1_IN0，使用时注意冲突。