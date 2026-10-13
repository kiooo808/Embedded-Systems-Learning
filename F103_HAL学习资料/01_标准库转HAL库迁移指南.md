# STM32F103 标准库 → HAL库 迁移指南

## 一、HAL库 vs 标准库 核心区别

| 对比项 | 标准库 (StdPeriph) | HAL库 |
|--------|-------------------|-------|
| 命名风格 | `RCC_APB2PeriphClockCmd()` | `HAL_RCC_APB2_CLK_ENABLE()` |
| 初始化方式 | 结构体+寄存器配置 | 结构体+CubeMX生成 |
| 延时函数 | 自定义 `delay_ms()` | `HAL_Delay()` (基于SysTick) |
| 中断处理 | 直接在中断函数中写逻辑 | 回调函数 `HAL_xxx_Callback()` |
| 设计理念 | 直接操作寄存器 | 硬件抽象层，跨平台统一 |
| MSP初始化 | 不需要 | 分离在 `stm32f1xx_hal_msp.c` |

## 二、GPIO 对比

### 标准库写法
```c
// 开启时钟
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

// 配置引脚
GPIO_InitTypeDef GPIO_InitStruct;
GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
GPIO_Init(GPIOC, &GPIO_InitStruct);

// 操作引脚
GPIO_SetBits(GPIOC, GPIO_Pin_13);
GPIO_ResetBits(GPIOC, GPIO_Pin_13);
GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
```

### HAL库写法
```c
// 开启时钟 (MSP中自动生成)
__HAL_RCC_GPIOC_CLK_ENABLE();

// 配置引脚
GPIO_InitTypeDef GPIO_InitStruct = {0};
GPIO_InitStruct.Pin = GPIO_PIN_13;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

// 操作引脚
HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);  // 新增：翻转
HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);    // 读取
```

### GPIO模式对照表

| 标准库 | HAL库 | 说明 |
|--------|-------|------|
| `GPIO_Mode_AIN` | `GPIO_MODE_ANALOG` | 模拟输入 |
| `GPIO_Mode_IN_FLOATING` | `GPIO_MODE_INPUT` + `GPIO_NOPULL` | 浮空输入 |
| `GPIO_Mode_IPD` | `GPIO_MODE_INPUT` + `GPIO_PULLDOWN` | 下拉输入 |
| `GPIO_Mode_IPU` | `GPIO_MODE_INPUT` + `GPIO_PULLUP` | 上拉输入 |
| `GPIO_Mode_Out_OD` | `GPIO_MODE_OUTPUT_OD` | 开漏输出 |
| `GPIO_Mode_Out_PP` | `GPIO_MODE_OUTPUT_PP` | 推挽输出 |
| `GPIO_Mode_AF_OD` | `GPIO_MODE_AF_OD` | 复用开漏 |
| `GPIO_Mode_AF_PP` | `GPIO_MODE_AF_PP` | 复用推挽 |

## 三、USART 对比

### 标准库写法
```c
USART_InitTypeDef USART_InitStruct;
USART_InitStruct.USART_BaudRate = 115200;
USART_InitStruct.USART_WordLength = USART_WordLength_8b;
USART_InitStruct.USART_StopBits = USART_StopBits_1;
USART_InitStruct.USART_Parity = USART_Parity_No;
USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
USART_Init(USART1, &USART_InitStruct);
USART_Cmd(USART1, ENABLE);

USART_SendData(USART1, data);
while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
```

### HAL库写法
```c
UART_HandleTypeDef huart1;
huart1.Instance = USART1;
huart1.Init.BaudRate = 115200;
huart1.Init.WordLength = UART_WORDLENGTH_8B;
huart1.Init.StopBits = UART_STOPBITS_1;
huart1.Init.Parity = UART_PARITY_NONE;
huart1.Init.Mode = UART_MODE_TX_RX;
huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
huart1.Init.OverSampling = UART_OVERSAMPLING_16;
HAL_UART_Init(&huart1);

HAL_UART_Transmit(&huart1, &data, 1, 100);     // 阻塞发送
HAL_UART_Receive(&huart1, &data, 1, 100);      // 阻塞接收
HAL_UART_Transmit_IT(&huart1, &data, 1);        // 中断发送
HAL_UART_Receive_IT(&huart1, &data, 1);         // 中断接收
HAL_UART_Transmit_DMA(&huart1, buf, len);        // DMA发送
```

## 四、定时器 对比

### 标准库写法
```c
TIM_TimeBaseInitTypeDef TIM_InitStruct;
TIM_InitStruct.TIM_Period = 999;
TIM_InitStruct.TIM_Prescaler = 71;
TIM_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
TIM_TimeBaseInit(TIM2, &TIM_InitStruct);
TIM_Cmd(TIM2, ENABLE);
```

### HAL库写法
```c
TIM_HandleTypeDef htim2;
htim2.Instance = TIM2;
htim2.Init.Period = 999;
htim2.Init.Prescaler = 71;
htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
HAL_TIM_Base_Init(&htim2);
HAL_TIM_Base_Start(&htim2);
HAL_TIM_Base_Start_IT(&htim2);   // 中断方式
```

## 五、中断处理 对比

### 标准库写法
```c
void EXTI0_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
        // 直接写业务逻辑
        GPIOA->ODR ^= GPIO_Pin_13;
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}
```

### HAL库写法
```c
// stm32f1xx_it.c 中
void EXTI0_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);  // 调用HAL处理
}

// 自己的代码中实现回调
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_0) {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);  // 业务逻辑
    }
}
```

## 六、时钟配置 对比

### 标准库
```c
SystemInit();  // 启动文件中自动调用
// 手动设置时钟:
RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE);
```

### HAL库
```c
HAL_Init();                    // 初始化HAL库 + SysTick
SystemClock_Config();          // CubeMX自动生成
__HAL_RCC_GPIOA_CLK_ENABLE();  // 开启GPIOA时钟
```

## 七、CubeMX 代码分区规则

CubeMX生成的代码有明确的分区，**业务代码必须写在 `USER CODE BEGIN` / `USER CODE END` 之间**，否则重新生成代码时会被覆盖：

```c
/* USER CODE BEGIN Includes */
#include "my_module.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PV */
volatile uint8_t flag = 0;
/* USER CODE END PV */

/* USER CODE BEGIN WHILE */
while (1) {
    // 你的循环逻辑
}
/* USER CODE END WHILE */
```

## 八、HAL库学习路线图

```
1. 基础入门
   ├── CubeMX生成工程 → 理解工程结构
   ├── GPIO输入输出 → LED/按键
   ├── 延时函数 → HAL_Delay / 非阻塞延时
   └── 中断EXTI → 按键中断

2. 通信协议
   ├── USART串口 → 重定向printf
   ├── I2C → OLED/传感器
   ├── SPI → TFT屏幕
   └── CAN/USB (进阶)

3. 定时器
   ├── 基本定时 → 定时中断
   ├── PWM → 电机/LED调光
   ├── 输入捕获 → 测量频率
   └── 输出比较 → 精确时序

4. 进阶功能
   ├── ADC → 电压采集
   ├── DMA → 高速数据搬运
   ├── RTC → 实时时钟
   └── Flash → 内部存储

5. 优化
   ├── 低功耗模式
   ├── FreeRTOS移植
   └── 功能安全
```