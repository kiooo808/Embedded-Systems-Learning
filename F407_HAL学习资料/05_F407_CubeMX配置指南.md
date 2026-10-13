# STM32F407VET6 CubeMX 配置指南

## 一、创建新工程

### 步骤1: 启动CubeMX
1. 打开 STM32CubeMX
2. 点击 "Start My Project from MCU/MPU Selection"
3. 在芯片列表中选择 "STM32F407VETx"
4. 点击 "Start Project"

### 步骤2: 配置系统时钟
1. 左侧点击 "Clock Configuration"
2. 选择 "PLL Source Mux" 为 HSE
3. 设置 PLL 参数:
   - PLLM: 8 (HSE分频)
   - PLLN: 336 (VCO倍频)
   - PLLP: 2 (SYSCLK分频)
   - PLLQ: 7 (USB时钟分频)
4. 配置总线时钟:
   - AHB Prescaler: /1 (168MHz)
   - APB1 Prescaler: /4 (42MHz)
   - APB2 Prescaler: /2 (84MHz)

### 步骤3: 配置调试接口
1. 左侧点击 "Pinout & Configuration"
2. System Core → SYS
3. Debug: Serial Wire

### 步骤4: 保存生成代码
1. Project → Generate Code
2. 选择工程路径
3. Toolchain 选择:
   - Makefile (GCC)
   - MDK-ARM (Keil)
   - IAR (EWARM)
4. 点击 "GENERATE CODE"

---

## 二、GPIO 配置

### 配置LED引脚 (PF9, PF10)
1. Pinout 视图中点击 PF9
2. 选择 "GPIO_Output"
3. 点击 PF10 选择 "GPIO_Output"
4. System Core → GPIO
5. 配置:
   - GPIO Mode: Output Push Pull
   - GPIO Pull-up/Pull-down: No pull-up and no pull-down
   - Maximum output speed: Low

### 配置按键引脚 (PA0)
1. Pinout 视图中点击 PA0
2. 选择 "GPIO_Input"
3. System Core → GPIO
4. 添加引脚 PA0
5. 配置:
   - GPIO Mode: Input mode
   - GPIO Pull-up/Pull-down: Pull-up

### 配置外部中断 (按键中断)
1. Pinout 视图中点击 PA0
2. 选择 "EXTI0" → "GPIO_EXTI0"
3. System Core → GPIO
4. 配置 GPIO:
   - GPIO mode: External Interrupt Mode with Rising edge trigger detection
   - GPIO Pull-up/Pull-down: Pull-up

---

## 三、USART 配置

### 配置USART3 (串口通信)
1. Connectivity → USART3
2. Mode: Asynchronous
3. Hardware Flow Control: None
4. Parameter Settings:
   - Baud Rate: 115200
   - Word Length: 8 Bits
   - Parity: No Parity
   - Stop Bits: 1
5. GPIO Settings:
   - PD8 (USART3_TX): AF_PP, Very High Speed
   - PD9 (USART3_RX): AF_PP, Very High Speed
6. NVIC Settings:
   - 勾选 USART3 global interrupt
   - Preemption Priority: 1
   - Sub Priority: 0

### 配置USART1 (高速串口)
1. Connectivity → USART1
2. Mode: Asynchronous
3. Parameter Settings: 115200, 8N1
4. GPIO: PA9 (TX), PA10 (RX)
5. NVIC: 勾选中断

---

## 四、TIM 定时器配置

### 配置TIM2 (基本定时中断)
1. Timers → TIM2
2. Clock Source: Internal Clock
3. Parameter Settings:
   - Prescaler: 83 (84MHz/84 = 1MHz)
   - Counter Period: 9999 (10ms)
   - Counter Mode: Up
   - Clock Division: DIV1
4. NVIC Settings:
   - 勾选 TIM2 global interrupt
   - Preemption Priority: 2
   - Sub Priority: 0

### 配置TIM3 (PWM输出)
1. Timers → TIM3
2. Clock Source: Internal Clock
3. Channel 1 → PWM Generation CH1
4. Parameter Settings:
   - Prescaler: 83 (1MHz)
   - Counter Period: 999 (1kHz)
   - Pulse: 500 (50%占空比)
5. GPIO: PA6 (TIM3_CH1), AF_PP, Very High
6. NVIC: 可选

### 配置TIM4 (输入捕获)
1. Timers → TIM4
2. Channel 1 → Input Capture CH1
3. Parameter Settings:
   - Prescaler: 83 (1MHz)
   - Counter Period: 65535
4. GPIO: PB6 (TIM4_CH1), AF_PP, Very High

---

## 五、ADC 配置

### 配置ADC1 (单通道)
1. Analog → ADC1
2. IN0 (PA0)
3. Parameter Settings:
   - Resolution: 12 bits
   - Data Alignment: Right alignment
   - Scan Conversion Mode: Disabled
   - Continuous Conversion Mode: Disabled
   - External Trigger Conversion Edge: Software trigger
4. Channel Configuration:
   - Channel: 0
   - Sampling Time: 55 cycles
5. DMA Settings (可选):
   - 勾选 Circular

### 配置ADC1 (多通道扫描)
1. Analog → ADC1
2. Mode: Independent
3. IN0, IN1, IN4
4. Scan Conversion Mode: Enabled
5. Continuous Conversion Mode: Enabled
6. Channel Configuration:
   - Rank 1: Channel 0, 55 cycles
   - Rank 2: Channel 1, 55 cycles
   - Rank 3: Channel 4, 55 cycles
7. DMA: Enabled, Circular, Word

---

## 六、SPI 配置

### 配置SPI1 (TFT/Flash)
1. Connectivity → SPI1
2. Mode: Transmit Only Master
3. Parameter Settings:
   - Frame Format: Motorola
   - Data Size: 8 Bits
   - First Bit: MSB First
   - Prescaler: 16
   - CPOL: Low
   - CPHA: 1 Edge
   - NSS: Software
4. GPIO:
   - PA5 (SCK): AF_PP, Very High
   - PA7 (MOSI): AF_PP, Very High
   - PA6 (MISO): Input (如需接收)

### 配置SPI3 (Flash/SD)
1. Connectivity → SPI3
2. Mode: Full-Duplex Master
3. Parameter Settings: 8Bits, 16分频, CPOL=0, CPHA=1
4. GPIO:
   - PC10 (SCK): AF_PP
   - PC11 (MISO): AF_PP
   - PC12 (MOSI): AF_PP

---

## 七、I2C 配置

### 配置I2C1 (OLED/Sensor)
1. Connectivity → I2C1
2. Mode: I2C
3. Parameter Settings:
   - Speed Mode: Fast Mode
   - Speed Frequency (Hz): 400000
4. GPIO:
   - PB6 (SCL): AF_OD, Very High
   - PB7 (SDA): AF_OD, Very High
5. NVIC: 勾选 I2C1 event interrupt

### 配置I2C3 (额外传感器)
1. Connectivity → I2C3
2. Mode: I2C
3. Speed Mode: Fast Mode
4. GPIO:
   - PA8 (SCL): AF_OD
   - PB4 (SDA): AF_OD

---

## 八、DMA 配置

### 添加DMA通道
1. 外设配置页面底部的 "DMA Settings"
2. 点击 "Add" 添加新DMA请求
3. 选择 DMA Request:
   - USART_RX
   - USART_TX
   - ADC
   - SPI_RX
   - SPI_TX
   - TIM_CHx
   - I2C_RX_TX
4. 配置:
   - Direction: Peripheral To Memory / Memory To Peripheral
   - Priority: Low / Medium / High / Very High
   - Mode: Normal / Circular
   - Data Width: Byte / Half Word / Word
   - Increment Address: Memory (Enable) / Peripheral (Disable)

### DMA示例 (USART3 RX)
1. Connectivity → USART3 → DMA Settings
2. Add → USART_RX
3. Direction: Peripheral To Memory
4. Priority: High
5. Mode: Circular
6. Data Width: Byte (8 Bits)
7. Increment Address: Memory (Enable)

---

## 九、F407 独有外设配置

### 以太网 (ETH)
1. Connectivity → ETH
2. Mode: RMII
3. GPIO:
   - PA1 (RMII_REF_CLK)
   - PA2 (RMII_MDIO)
   - PA7 (RMII_CRS_DV)
   - PB0 (RMII_RXD0)
   - PB1 (RMII_RXD1)
   - PA11 (RMII_TX_EN)
   - PA12 (RMII_TXD0)
   - PB11 (RMII_TXD1)
4. NVIC: 勾选 ETH global interrupt
5. 注意: 需要外部PHY芯片 (如LAN8720) 和LwIP协议栈

### USB OTG FS
1. Connectivity → USB_OTG_FS
2. Mode: Device (或 Host)
3. GPIO:
   - PA11 (USB_DM)
   - PA12 (USB_DP)
4. 注意: USB时钟必须是48MHz

### SDIO
1. Connectivity → SDIO
2. Mode: SDIO 1-bit / 4-bit
3. GPIO:
   - PC8 (SDIO_D0)
   - PC9 (SDIO_D1)
   - PC10 (SDIO_D2)
   - PC11 (SDIO_D3)
   - PC12 (SDIO_CLK)
   - PD2 (SDIO_CMD)
4. 注意: 需要FatFs文件系统

### LCD-TFT (FSMC接口)
1. Connectivity → FMC
2. Mode: SRAM 16bit / NOR / NAND
3. GPIO:
   - PD0-15 (数据/地址)
   - PE2-15 (地址/控制)
4. 用于LCD显示或外部存储

### RNG (随机数)
1. System Core → RNG
2. GPIO: 无 (内部外设)
3. 用于生成随机数

### Crypto (加密)
1. Security → CRYP
2. Mode: AES / DES / SM4
3. GPIO: 无 (内部外设)

### HASH (哈希)
1. Security → HASH
2. Mode: SHA-1 / SHA-256
3. GPIO: 无 (内部外设)

---

## 十、F407 时钟配置详解

### 时钟树结构
```
                ┌─────────────────┐
                │  HSE (8MHz)     │  外部晶振
                │  HSI (16MHz)    │  内部RC
                │  LSE (32.768kHz)│  外部低速晶振
                │  LSI (32kHz)    │  内部低速RC
                └────────┬────────┘
                         │
                    ┌────▼────┐
                    │  PLL     │  锁相环
                    │  PLLM=8  │
                    │  PLLN=336│
                    │  PLLP=2  │
                    │  PLLQ=7  │
                    └────┬────┘
                         │
              ┌──────────┼──────────┐
              │          │          │
         ┌────▼────┐ ┌──▼──┐ ┌───▼──┐
         │  SYSCLK │ │ USB │ │ RNG  │
         │ 168MHz  │ │48MHz│ │48MHz │
         └────┬────┘ └─────┘ └──────┘
              │
    ┌─────────┼─────────┐
    │         │         │
┌───▼───┐ ┌──▼──┐ ┌───▼───┐
│ AHB   │ │APB1 │ │ APB2  │
│168MHz │ │42MHz│ │84MHz  │
│ /1    │ │ /4  │ │ /2    │
└───┬───┘ └──┬──┘ └───┬───┘
    │        │        │
    │   ┌────┴────┐   │
    │   │         │   │
    │  TIM2-7  USART2-3│
    │  (84MHz) (42MHz) │
    │   │         │   │
    │   └─────────┘   │
    │                 │
┌───▼───┐       ┌─────▼─────┐
│ GPIO  │       │ TIM1, TIM8 │
│ 168MHz│       │ (168MHz)  │
└───────┘       └───────────┘
```

### 定时器时钟规则
- 当APB1分频 > 1时，APB1定时器时钟自动倍频到84MHz
- 当APB2分频 > 1时，APB2定时器时钟自动倍频到168MHz
- 这使得定时器即使APB总线降频也能保持高时钟频率

### 时钟计算示例
```c
// TIM2 配置为 10ms 定时
// 定时器时钟 = 84MHz (APB1 = 42MHz, 定时器倍频)
// Prescaler = 83 → 84MHz / 84 = 1MHz
// Period = 9999 → 10000 / 1MHz = 10ms

htim2.Init.Prescaler = 83;
htim2.Init.Period = 9999;

// TIM3 配置为 1kHz PWM
// 定时器时钟 = 84MHz
// Prescaler = 83 → 84MHz / 84 = 1MHz
// Period = 999 → 1000 / 1MHz = 1kHz
// Compare = 500 → 500 / 1000 = 50% 占空比

htim3.Init.Prescaler = 83;
htim3.Init.Period = 999;
__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 500);
```

---

## 十一、代码生成选项

### 生成设置
1. Project Manager → Project Name: 输入工程名
2. Application Structure: Basic
3. Toolchain / IDE: 选择合适的工具链
4. Generate peripheral initialization: Enabled
5. Set all free pins as analog: Disabled
6. Keep User Code when re-generating: Enabled

### 代码生成
1. Project → Generate Code (Ctrl+Shift+G)
2. 确认保存
3. 等待生成完成
4. 打开生成的工程进行编译

---

## 十二、常见CubeMX配置问题

### Q1: 如何启用FPU?
A: CubeMX自动处理。在生成的代码中，FPU初始化在system_stm32f4xx.c中。

### Q2: 如何配置外部时钟源?
A: Clock Configuration → 选择HSE作为时钟源，配置PLL。

### Q3: 为什么定时器不工作?
A: 检查：
   1. 是否启用了定时器时钟
   2. 是否调用了HAL_TIM_Base_Start()或HAL_TIM_Base_Start_IT()
   3. NVIC是否使能

### Q4: USART收不到数据?
A: 检查：
   1. GPIO是否正确配置为AF_PP
   2. 波特率是否匹配
   3. 时钟是否正确
   4. 是否启用了USART时钟

### Q5: ADC值不准确?
A: 检查：
   1. 采样时间是否足够长
   2. 参考电压是否稳定
   3. 阻抗是否过高 (建议<10kΩ)

### Q6: DMA传输不工作?
A: 检查：
   1. DMA配置是否正确
   2. 数据缓冲区是否在有效RAM区
   3. 是否启动了DMA传输

### Q7: 如何使用printf?
A: 重定向printf到USART：
```c
// 使用GCC/Clang:
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart3, (uint8_t*)&ch, 1, 100);
    return ch;
}

// 使用Keil:
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart3, (uint8_t*)&ch, 1, 100);
    return ch;
}
```
