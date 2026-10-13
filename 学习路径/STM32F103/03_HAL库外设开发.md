# STM32F103C8T6 学习路径 - 第三章：HAL 库外设开发

## 学习目标
- 掌握 STM32 HAL 库的编程范式
- 学会使用 CubeMX 生成配置代码
- 理解 HAL 库与标准库的映射关系
- 实现与标准库对照的完整外设示例

---

## 第一节：HAL 库概述与迁移基础

### 1.1 HAL 库特点
| 特性 | 说明 |
|------|------|
| 全系列支持 | F0/F1/F2/F3/F4/L0/L4/H7 等 |
| 统一 API | 相同功能不同芯片 API 一致 |
| CubeMX 集成 | 图形化配置，自动生成代码 |
| 三种开发模式 | Polling（轮询）/Interrupt（中断）/DMA |
| 回调机制 | 中断处理通过回调函数 |
| MSP 分离 | 初始化代码分层（外设初始化 + 低层初始化） |

### 1.2 关键概念
```
HAL 库分层结构:
┌─────────────────────────────────┐
│     用户应用程序 (app)           │
├─────────────────────────────────┤
│   HAL API (高层)                  │
│   ├── HAL_GPIO_Init()            │
│   ├── HAL_UART_Init()            │
│   └── HAL_TIM_PWM_Init()         │
├─────────────────────────────────┤
│   MSP API (低层初始化)            │
│   ├── HAL_GPIO_MspInit()         │
│   ├── HAL_UART_MspInit()         │
│   └── HAL_TIM_PWM_MspInit()      │
├─────────────────────────────────┤
│   LL API (可选，更高效)           │
│   ├── LL_GPIO_Init()             │
│   └── LL_TIM_Init()              │
├─────────────────────────────────┤
│   硬件抽象层 (寄存器操作)          │
└─────────────────────────────────┘
```

### 1.3 标准库 → HAL 库映射

| 功能 | 标准库 | HAL 库 |
|------|--------|--------|
| 初始化 | `GPIO_Init()` | `HAL_GPIO_Init()` |
| 开时钟 | `RCC_APB2PeriphClockCmd()` | `__HAL_RCC_GPIOC_CLK_ENABLE()` |
| 配置中断 | `NVIC_Init()` | `HAL_NVIC_SetPriority()` + `HAL_NVIC_EnableIRQ()` |
| 设置电平 | `GPIO_SetBits()` / `GPIO_ResetBits()` | `HAL_GPIO_WritePin()` |
| 读取电平 | `GPIO_ReadInputDataBit()` | `HAL_GPIO_ReadPin()` |
| 延时 | 自定义 `Delay_ms()` | `HAL_Delay()` |
| 发送数据 | `USART_SendData()` + 等待 | `HAL_UART_Transmit()` |
| 接收数据 | `USART_ReceiveData()` | `HAL_UART_Receive()` |
| 定时中断 | `TIM_ITConfig()` + `TIM_Cmd()` | `HAL_TIM_Base_Start_IT()` |
| PWM | `TIM_OC1Init()` + `TIM_SetCompare1()` | `HAL_TIM_PWM_Start()` + `__HAL_TIM_SET_COMPARE()` |

---

## 第二节：CubeMX 配置基础

### 2.1 启动 CubeMX
1. 双击 STM32CubeMX 图标
2. 创建新工程或打开已有工程

### 2.2 配置步骤
#### 步骤 1：选择芯片
- Part Number: `STM32F103C8Tx`（LQFP48 封装）

#### 步骤 2：配置调试
- Connectivity → `SYS` → Debug → `Serial Wire`

#### 步骤 3：配置时钟
- Clock Configuration 选项卡
- HSE: `Crystal/Ceramic Resonator`（外部晶振 8MHz）
- PLL 配置：`HSE × 9 = 72MHz`
- APB1: `36MHz`
- APB2: `72MHz`

#### 步骤 4：配置 GPIO
- GPIO → PC13 → 设置为 `GPIO_Output` → LED
- GPIO → PA0 → 设置为 `GPIO_Input` → KEY（EXTI 或普通输入）
- 或 PA0 → 设置为 `EXTI ... Rising edge trigger detection`

#### 步骤 5：配置 USART
- Connectivity → USART1 → Mode: `Asynchronous`
- 参数：`115200, 8 Bits, No Parity, 1 Stop Bit`
- NVIC → 勾选 `USART1 global interrupt`

#### 步骤 6：配置 TIM（PWM）
- Timers → TIM3 → Channel1 → `PWM Generation CH1`
- Prescaler: `71`（72MHz/72 = 1MHz）
- Counter Period: `999`（1MHz/1000 = 1kHz）

#### 步骤 7：配置 TIM（定时中断）
- Timers → TIM2 → Clock Source: `Internal Clock`
- Prescaler: `71`（1MHz）
- Counter Period: `999`（1kHz，1ms 中断）
- NVIC → 勾选 `TIM2 global interrupt`

#### 步骤 8：生成代码
- Project → Generate Code（或 Ctrl+S）
- Toolchain: 选择 `STM32CubeCLT` 或 `Makefile`

### 2.3 CubeMX 生成的文件
```
Core/
├── Inc/
│   ├── main.h          # 主头文件
│   ├── gpio.h          # GPIO 配置
│   ├── tim.h           # 定时器配置
│   └── usart.h         # 串口配置
└── Src/
    ├── main.c          # 主程序
    ├── gpio.c          # GPIO 初始化
    ├── tim.c           # 定时器初始化
    ├── usart.c         # 串口初始化
    ├── stm32f1xx_it.c  # 中断处理
    └── system_stm32f1xx.c
```

---

## 第三节：GPIO HAL 开发

### 3.1 输出配置（CubeMX 生成）
```c
// gpio.c
void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 1. 开启时钟
    __HAL_RCC_GPIOC_CLK_ENABLE();

    // 2. 配置 LED 引脚
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    // 3. 设置初始值
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}
```

### 3.2 输入配置（EXTI）
```c
// CubeMX 配置：PA0 → External Interrupt Mode with Rising edge trigger
void MX_GPIO_Init(void)
{
    // ...

    // 配置 PA0 为上升沿中断
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;   // 上升沿中断
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;          // 下拉
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 配置中断优先级
    HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}
```

### 3.3 手动配置示例
```c
// 完全手动（不依赖 CubeMX）
void GPIO_LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}
```

### 3.4 常用函数
```c
// 写
HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);     // 置位
HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);  // 复位
HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);                  // 翻转

// 读
GPIO_PinState state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
if (state == GPIO_PIN_SET) { /* 高电平 */ }
if (state == GPIO_PIN_RESET) { /* 低电平 */ }

// 中断
HAL_GPIO_IT_ENABLE(GPIOA, GPIO_PIN_0);
HAL_GPIO_IT_DISABLE(GPIOA, GPIO_PIN_0);
HAL_GPIO_IRQHandler(GPIO_PIN_0);  // 在 ISR 中调用
```

---

## 第四节：USART HAL 开发

### 4.1 初始化（CubeMX 生成）
```c
// usart.c
UART_HandleTypeDef huart1;

void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
}

// MSP 回调函数（CubeMX 生成，在 stm32f1xx_hal_msp.c）
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    if (huart->Instance == USART1)
    {
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        // TX (PA9)
        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        // RX (PA10)
        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        // NVIC
        HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
    }
}
```

### 4.2 阻塞方式收发
```c
// 发送
void USART_SendString(char *str)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 1000);
}

void USART_SendData(uint8_t *data, uint16_t len)
{
    HAL_UART_Transmit(&huart1, data, len, 1000);
}

// 接收
uint8_t USART_ReceiveByte(void)
{
    uint8_t data;
    HAL_UART_Receive(&huart1, &data, 1, 1000);
    return data;
}
```

### 4.3 中断方式收发
```c
// 启动接收
uint8_t rx_buffer[64];
HAL_UART_Receive_IT(&huart1, rx_buffer, 1);

// 发送
HAL_UART_Transmit_IT(&huart1, (uint8_t*)"Hello", 5);

// 回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        // 处理接收完成
        // 继续接收
        HAL_UART_Receive_IT(&huart1, rx_buffer, 1);
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        // 发送完成处理
    }
}
```

### 4.4 DMA 方式收发
```c
// CubeMX 配置：USART1 → DMA Settings → Add
// Direction: Memory To Peripheral (发送)
// Direction: Peripheral To Memory (接收)
// Priority: High

// DMA 发送
uint8_t tx_data[] = "Hello DMA!\r\n";
HAL_UART_Transmit_DMA(&huart1, tx_data, strlen(tx_data));

// DMA 接收
uint8_t rx_buf[64];
HAL_UART_Receive_DMA(&huart1, rx_buf, sizeof(rx_buf));
__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);  // 空闲中断

// 空闲中断（不定长接收）
void USART1_IRQHandler(void)
{
    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE))
    {
        __HAL_UART_CLEAR_IDLEFLAG(&huart1);
        // 停止 DMA，获取已接收长度
        // 处理数据...
        // 重新启动 DMA 接收
    }
    HAL_UART_IRQHandler(&huart1);
}
```

### 4.5 printf 重定向
```c
// GCC/Clang (VSCode)
#ifdef __GNUC__
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
#endif

// Keil MDK
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
```

---

## 第五节：TIM HAL 开发

### 5.1 基本定时中断
```c
// tim.c
TIM_HandleTypeDef htim2;

void MX_TIM2_Init(void)
{
    TIM_IC_InitTypeDef sConfigIC = {0};
    
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 71;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 999;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
}

// 启动定时中断
HAL_TIM_Base_Start_IT(&htim2);

// 回调函数
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        // 1ms 定时任务
        // 注意：中断中不要使用阻塞函数
    }
}
```

### 5.2 PWM 输出
```c
// tim.c
TIM_HandleTypeDef htim3;

void MX_TIM3_Init(void)
{
    TIM_OC_InitTypeDef sConfigOC = {0};
    
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 71;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 999;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    
    if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
    {
        Error_Handler();
    }
    
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 500;                    // 50% 占空比
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    
    if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }
}

// 启动 PWM
HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

// 动态设置占空比
__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 300);  // 30%

// 呼吸灯示例
void PWM_Breathing(void)
{
    uint16_t duty;
    for (duty = 0; duty <= 999; duty += 10)
    {
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, duty);
        HAL_Delay(5);
    }
    for (duty = 999; duty >= 0; duty -= 10)
    {
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, duty);
        HAL_Delay(5);
    }
}
```

---

## 第六节：I2C HAL 开发

### 6.1 初始化
```c
// i2c.c
I2C_HandleTypeDef hi2c1;

void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 400000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    
    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        Error_Handler();
    }
}
```

### 6.2 阻塞方式收发
```c
// 写寄存器
HAL_StatusTypeDef I2C_Write(uint16_t dev_addr, uint8_t reg, 
                             uint8_t *data, uint16_t len)
{
    return HAL_I2C_Mem_Write(&hi2c1, 
                              dev_addr << 1,          // 7位地址左移
                              reg,                     // 寄存器地址
                              I2C_MEMADD_SIZE_8BIT,   // 8位寄存器
                              data, len, 
                              1000);                   // 超时1秒
}

// 读寄存器
HAL_StatusTypeDef I2C_Read(uint16_t dev_addr, uint8_t reg, 
                            uint8_t *data, uint16_t len)
{
    return HAL_I2C_Mem_Read(&hi2c1,
                              dev_addr << 1,
                              reg,
                              I2C_MEMADD_SIZE_8BIT,
                              data, len,
                              1000);
}
```

### 6.3 内存 → 内存 DMA
```c
// 将数据从一个内存区域拷贝到另一个
HAL_StatusTypeDef status = HAL_I2C_Mem_Write_DMA(&hi2c1, 
                                    dev_addr << 1, reg,
                                    I2C_MEMADD_SIZE_8BIT,
                                    tx_buf, len);

HAL_StatusTypeDef status = HAL_I2C_Mem_Read_DMA(&hi2c1,
                                    dev_addr << 1, reg,
                                    I2C_MEMADD_SIZE_8BIT,
                                    rx_buf, len);
```

---

## 第七节：SPI HAL 开发

### 7.1 初始化
```c
// spi.c
SPI_HandleTypeDef hspi1;

void MX_SPI1_Init(void)
{
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 7;
    
    if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
        Error_Handler();
    }
}
```

### 7.2 数据传输
```c
// 阻塞方式
uint8_t SPI_Transfer(uint8_t data)
{
    uint8_t rx_data;
    HAL_SPI_TransmitReceive(&hspi1, &data, &rx_data, 1, 1000);
    return rx_data;
}

void SPI_Transfer_Buf(uint8_t *tx, uint8_t *rx, uint16_t len)
{
    HAL_SPI_TransmitReceive(&hspi1, tx, rx, len, 10000);
}

// DMA 方式
HAL_SPI_Transmit_DMA(&hspi1, tx_buf, len);
HAL_SPI_Receive_DMA(&hspi1, rx_buf, len);
HAL_SPI_TransmitReceive_DMA(&hspi1, tx_buf, rx_buf, len);
```

---

## 第八节：ADC HAL 开发

### 8.1 初始化与读取
```c
// adc.c
ADC_HandleTypeDef hadc1;

void MX_ADC1_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    
    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    
    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }
    
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

// 单次读取
uint16_t ADC_Read(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 100);
    return HAL_ADC_GetValue(&hadc1);
}

// 多通道扫描 + DMA
uint16_t adc_values[4];
HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_values, 4);
```

---

## 第九节：NVIC 与中断 HAL 开发

### 9.1 优先级配置
```c
// 在 main.c 中
void SystemClock_Config(void)
{
    // ... 时钟配置
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_2);
}
```

### 9.2 中断处理函数
```c
// stm32f1xx_it.c
void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);  // HAL 处理
}

void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);  // HAL 处理
}

void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2);  // HAL 处理
}
```

### 9.3 回调函数
```c
// main.c
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_0)
    {
        // 按键处理
        HAL_Delay(20);
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET)
        {
            // 执行操作
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    // 串口接收完成
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    // 定时器周期到达
}

// 错误回调（必须实现）
void HAL_ErrorCallback(void)
{
    // 错误处理
}
```

---

## 第十节：Flash HAL 开发

### 10.1 Flash 写入
```c
#define FLASH_USER_ADDR  0x0803F800  // 页地址（最后一页）

void Flash_WriteData(uint32_t addr, uint16_t *data, uint16_t len)
{
    // 1. 解锁
    HAL_FLASH_Unlock();
    
    // 2. 清除标志
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_BSY | FLASH_FLAG_EOP);
    
    // 3. 擦除页
    // 查找地址所在页号
    uint32_t page = (addr - FLASH_BASE) / 1024;  // F103: 1KB 页
    FLASH_Erase_Page(page, FLASH_TYPEERASE_PAGEERASE);
    while (HAL_FLASH_GetError() != 0);
    
    // 4. 编程写入
    for (uint16_t i = 0; i < len; i++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, 
                          addr + i * 2, data[i]);
        while (HAL_FLASH_GetError() != 0);
    }
    
    // 5. 上锁
    HAL_FLASH_Lock();
}

void Flash_ReadData(uint32_t addr, uint16_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        data[i] = *(__IO uint16_t*)(addr + i * 2);
    }
}
```

---

## 学习练习

### 练习 1：CubeMX 完整配置
- [ ] 新建 CubeMX 工程
- [ ] 配置 GPIO（LED + KEY）
- [ ] 配置 USART1
- [ ] 配置 TIM2（定时中断）
- [ ] 配置 TIM3（PWM）
- [ ] 生成代码并编译

### 练习 2：HAL 库实现所有基础功能
- [ ] GPIO 输出 + 输入 + 中断
- [ ] USART 阻塞 + 中断 + DMA
- [ ] TIM 定时 + PWM
- [ ] I2C + SPI + ADC

### 练习 3：HAL 库与标准库对照练习
- [ ] 分别用两种库实现 LED 闪烁
- [ ] 分别用两种库实现串口回环
- [ ] 分别用两种库实现 PWM 呼吸灯
- [ ] 对比代码风格和效率

### 练习 4：CubeMX + 自定义代码
- [ ] 用 CubeMX 生成框架
- [ ] 在 `USER CODE BEGIN/END` 区域添加自定义代码
- [ ] 重新生成代码后，自定义代码不会被覆盖
- [ ] 理解 CubeMX 代码保护机制

### 练习 5：综合项目
- [ ] OLED 显示 + ADC 采样
- [ ] 按键菜单 + 多任务调度
- [ ] Flash 参数存储 + 掉电保存

---

## 本章小结
1. ✅ 理解 HAL 库的分层架构
2. ✅ 掌握 CubeMX 图形化配置流程
3. ✅ 掌握 HAL 库 GPIO/USART/TIM/I2C/SPI/ADC 开发
4. ✅ 掌握阻塞/中断/DMA 三种开发模式
5. ✅ 掌握 HAL 库的回调机制
6. ✅ 掌握 CubeMX 的代码保护机制
7. ✅ 完成 5 个练习

**下一章**：综合项目实战与进阶技巧
