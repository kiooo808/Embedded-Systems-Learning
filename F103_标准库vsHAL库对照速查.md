# STM32F103 标准库 vs HAL库 对照速查

## 一、初始化函数对照

| 功能 | 标准库 (StdPeriph) | HAL库 |
|------|-------------------|-------|
| 系统初始化 | `SystemInit()` (自动) | `HAL_Init()` + `SystemClock_Config()` |
| GPIO时钟使能 | `RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE)` | `__HAL_RCC_GPIOA_CLK_ENABLE()` |
| USART时钟使能 | `RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE)` | `__HAL_RCC_USART1_CLK_ENABLE()` |
| TIM时钟使能 | `RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE)` | `__HAL_RCC_TIM2_CLK_ENABLE()` |
| ADC时钟使能 | `RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE)` | `__HAL_RCC_ADC1_CLK_ENABLE()` |
| AFIO时钟使能 | `RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE)` | `__HAL_RCC_AFIO_CLK_ENABLE()` |

## 二、GPIO 对照

### 初始化
```c
// 标准库
GPIO_InitTypeDef GPIO_InitStruct;
GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
GPIO_Init(GPIOC, &GPIO_InitStruct);

// HAL库
GPIO_InitTypeDef GPIO_InitStruct = {0};
GPIO_InitStruct.Pin = GPIO_PIN_13;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
```

### 操作
| 操作 | 标准库 | HAL库 |
|------|--------|-------|
| 写高 | `GPIO_SetBits(GPIOC, GPIO_Pin_13)` | `HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET)` |
| 写低 | `GPIO_ResetBits(GPIOC, GPIO_Pin_13)` | `HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET)` |
| 写指定 | `GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET)` | `HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET)` |
| 翻转 | `GPIOC->ODR ^= GPIO_Pin_13` | `HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13)` |
| 读引脚 | `GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)` | `HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)` |
| 读整个端口 | `GPIO_ReadInputData(GPIOA)` | `(uint32_t)GPIOA->IDR` |
| 写整个端口 | `GPIO_Write(GPIOA, 0xFFFF)` | `GPIOA->ODR = 0xFFFF` |

### 模式对照
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

### 速度对照
| 标准库 | HAL库 |
|--------|-------|
| `GPIO_Speed_2MHz` | `GPIO_SPEED_FREQ_LOW` |
| `GPIO_Speed_25MHz` | `GPIO_SPEED_FREQ_MEDIUM` |
| `GPIO_Speed_50MHz` | `GPIO_SPEED_FREQ_HIGH` |
| `GPIO_Speed_10MHz` | `GPIO_SPEED_FREQ_VERY_HIGH` |

## 三、USART 对照

### 初始化
```c
// 标准库
USART_InitTypeDef USART_InitStruct;
USART_InitStruct.USART_BaudRate = 115200;
USART_InitStruct.USART_WordLength = USART_WordLength_8b;
USART_InitStruct.USART_StopBits = USART_StopBits_1;
USART_InitStruct.USART_Parity = USART_Parity_No;
USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
USART_Init(USART1, &USART_InitStruct);
USART_Cmd(USART1, ENABLE);

// HAL库
UART_HandleTypeDef huart1;
huart1.Instance = USART1;
huart1.Init.BaudRate = 115200;
huart1.Init.WordLength = UART_WORDLENGTH_8B;
huart1.Init.StopBits = UART_STOPBITS_1;
huart1.Init.Parity = UART_PARITY_NONE;
huart1.Init.Mode = UART_MODE_TX_RX;
HAL_UART_Init(&huart1);
```

### 收发
| 操作 | 标准库 | HAL库 |
|------|--------|-------|
| 发送1字节 | `USART_SendData(USART1, data)` | `HAL_UART_Transmit(&huart1, &data, 1, 100)` |
| 发送字符串 | 循环 `USART_SendData` | `HAL_UART_Transmit(&huart1, str, len, 1000)` |
| 接收1字节 | `USART_ReceiveData(USART1)` | `HAL_UART_Receive(&huart1, &data, 1, 100)` |
| 等待TXE | `while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)==RESET)` | (HAL库内部处理) |
| 使能接收中断 | `USART_ITConfig(USART1, USART_IT_RXNE, ENABLE)` | `HAL_UART_Receive_IT(&huart1, &data, 1)` |
| 中断发送 | (手动) | `HAL_UART_Transmit_IT(&huart1, data, len)` |
| DMA发送 | (需配置DMA) | `HAL_UART_Transmit_DMA(&huart1, data, len)` |

### 中断处理
```c
// 标准库: 直接在中断函数中处理
void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        data = USART_ReceiveData(USART1);
        // 直接写业务逻辑
    }
}

// HAL库: 先调HAL处理，再实现回调
void USART1_IRQHandler(void) {
    HAL_UART_IRQHandler(&huart1);  // HAL处理
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    // 业务逻辑
}
```

## 四、TIM 对照

### 基本定时
```c
// 标准库
TIM_TimeBaseInitTypeDef TIM_InitStruct;
TIM_InitStruct.TIM_Period = 9999;
TIM_InitStruct.TIM_Prescaler = 71;
TIM_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
TIM_TimeBaseInit(TIM2, &TIM_InitStruct);
TIM_Cmd(TIM2, ENABLE);

// HAL库
TIM_HandleTypeDef htim2;
htim2.Instance = TIM2;
htim2.Init.Period = 9999;
htim2.Init.Prescaler = 71;
htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
HAL_TIM_Base_Init(&htim2);
HAL_TIM_Base_Start(&htim2);
```

### PWM
```c
// 标准库
TIM_OCInitTypeDef TIM_OCInitStruct;
TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
TIM_OCInitStruct.TIM_Pulse = 500;
TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
TIM_OC1Init(TIM3, &TIM_OCInitStruct);
TIM_Cmd(TIM3, ENABLE);

// HAL库
TIM_OC_InitTypeDef TIM_OCInitStruct;
TIM_OCInitStruct.OCMode = TIM_OCMODE_PWM1;
TIM_OCInitStruct.Pulse = 500;
TIM_OCInitStruct.OCPolarity = TIM_OCPOLARITY_HIGH;
HAL_TIM_PWM_ConfigChannel(&htim3, &TIM_OCInitStruct, TIM_CHANNEL_1);
HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
```

### 定时器操作
| 操作 | 标准库 | HAL库 |
|------|--------|-------|
| 启动 | `TIM_Cmd(TIM2, ENABLE)` | `HAL_TIM_Base_Start(&htim2)` |
| 中断启动 | `TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE)` + `TIM_Cmd` | `HAL_TIM_Base_Start_IT(&htim2)` |
| 设置比较值 | `TIM_SetCompare1(TIM3, 500)` | `__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 500)` |
| 设置重载 | `TIM_SetAutoreload(TIM2, 9999)` | `__HAL_TIM_SET_AUTORELOAD(&htim2, 9999)` |
| 读计数器 | `TIM_GetCounter(TIM2)` | `__HAL_TIM_GET_COUNTER(&htim2)` |
| 清除标志 | `TIM_ClearFlag(TIM2, TIM_IT_Update)` | `__HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE)` |

## 五、EXTI 对照

### 初始化
```c
// 标准库
EXTI_InitTypeDef EXTI_InitStruct;
GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
EXTI_InitStruct.EXTI_Line = EXTI_Line0;
EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
EXTI_InitStruct.EXTI_LineCmd = ENABLE;
EXTI_Init(&EXTI_InitStruct);

// HAL库 (在GPIO初始化时配置)
GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
HAL_NVIC_EnableIRQ(EXTI0_IRQn);
```

### 中断处理
```c
// 标准库
void EXTI0_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
        EXTI_ClearITPendingBit(EXTI_Line0);
        // 业务逻辑
    }
}

// HAL库
void EXTI0_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    // 业务逻辑
}
```

## 六、ADC 对照

### 初始化
```c
// 标准库
ADC_InitTypeDef ADC_InitStruct;
ADC_InitStruct.ADC_Mode = ADC_Mode_Independent;
ADC_InitStruct.ADC_ScanConvMode = DISABLE;
ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
ADC_InitStruct.ADC_NbrOfChannel = 1;
ADC_Init(ADC1, &ADC_InitStruct);
ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);
ADC_Cmd(ADC1, ENABLE);
ADC_StartCalibration(ADC1);

// HAL库
ADC_ChannelConfTypeDef sConfig;
hadc1.Instance = ADC1;
hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
hadc1.Init.ContinuousConvMode = DISABLE;
hadc1.Init.DiscontinuousConvMode = DISABLE;
hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
hadc1.Init.NbrOfConversion = 1;
HAL_ADC_Init(&hadc1);
sConfig.Channel = ADC_CHANNEL_0;
sConfig.Rank = ADC_REGULAR_RANK_1;
sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
HAL_ADC_ConfigChannel(&hadc1, &sConfig);
```

### 转换
```c
// 标准库
ADC_SoftwareStartConvCmd(ADC1, ENABLE);
while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
uint16_t val = ADC_GetConversionValue(ADC1);

// HAL库
HAL_ADC_Start(&hadc1);
HAL_ADC_PollForConversion(&hadc1, 100);
uint16_t val = HAL_ADC_GetValue(&hadc1);
```

## 七、NVIC 对照

| 操作 | 标准库 | HAL库 |
|------|--------|-------|
| 分组设置 | `NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2)` | `HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_2)` |
| 使能中断 | `NVIC_Init()` 结构体方式 | `HAL_NVIC_SetPriority()` + `HAL_NVIC_EnableIRQ()` |
| 使能外部中断 | `EXTI_InitStruct.EXTI_LineCmd = ENABLE` + `EXTI_Init()` | `HAL_NVIC_EnableIRQ(EXTI0_IRQn)` |

## 八、延时对照

| 操作 | 标准库 | HAL库 |
|------|--------|-------|
| 毫秒延时 | 自定义 `Delay_ms()` | `HAL_Delay(ms)` |
| 微秒延时 | 自定义 `Delay_us()` | (需自己实现) |
| 系统运行时间 | 读 `SysTick->VAL` | `HAL_GetTick()` |
| 注意 | 中断中不能使用 | 中断中不能使用 `HAL_Delay()` |

## 九、代码分区对照

| 项目 | 标准库 | HAL库 |
|------|--------|-------|
| 代码生成 | 手写 | CubeMX自动生成框架 |
| 代码覆盖 | 无保护 | `USER CODE BEGIN/END` 保护区 |
| MSP初始化 | 不需要 | 分离到 `stm32f1xx_hal_msp.c` |
| 回调机制 | 无，直接写中断函数 | 回调函数 `HAL_xxx_Callback()` |
| 重定向printf | `fputc()` | `fputc()` 或 `__io_putchar()` |

## 十、迁移建议

### 从标准库到HAL
1. 保留标准库知识 — 有助于理解底层原理
2. 新项目用HAL — CubeMX生成，开发效率高
3. 旧项目维护 — 继续用标准库，不强制迁移
4. 混合使用 — 尽量避免，会增加复杂度

### 从HAL到标准库
1. 理解HAL封装 — 知道底层在做什么
2. 移植代码 — 按模块逐个迁移
3. 调试问题 — 标准库直接操作寄存器更方便
4. 性能优化 — 标准库开销更小

### 推荐做法
```
学习阶段: 标准库 → 理解原理
开发阶段: HAL库 → 提高效率
调试阶段: 寄存器 → 解决问题
```