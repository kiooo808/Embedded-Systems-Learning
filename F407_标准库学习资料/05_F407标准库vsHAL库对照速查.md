# STM32F407VET6 标准库 vs HAL库 对照速查

## 一、总体对比

| 对比项 | 标准库 (StdPeriph) | HAL库 |
|--------|-------------------|-------|
| 头文件 | `stm32f4xx.h` | `stm32f4xx_hal.h` |
| 初始化 | 直接调用 `GPIO_Init()` | `HAL_GPIO_Init()` + MSP分离 |
| 风格 | 直接寄存器操作 | 硬件抽象层 |
| 代码量 | 较少 | 较多 |
| 可移植性 | F407专用 | 跨F4系列 |
| 学习曲线 | 陡峭 | 平缓 |
| CubeMX支持 | 不支持 | 完全支持 |
| 中断处理 | 在ISR中直接写逻辑 | 回调函数方式 |
| 延时 | 自定义 `Delay_ms()` | `HAL_Delay()` |
| FreeRTOS集成 | 需自行适配 | CMSIS-RTOS兼容 |
| 调试难度 | 较难 | 较易 |
| 代码可读性 | 一般 | 较好 |
| 官方支持 | 停止维护 | 持续更新 |

---

## 二、GPIO 对比

### 标准库写法
```c
// 时钟使能
RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

// 初始化
GPIO_InitTypeDef GPIO_InitStruct;
GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
GPIO_Init(GPIOF, &GPIO_InitStruct);

// 操作
GPIO_SetBits(GPIOF, GPIO_Pin_9);       // 置位
GPIO_ResetBits(GPIOF, GPIO_Pin_9);    // 复位
GPIO_WriteBit(GPIOF, GPIO_Pin_9, Bit_SET);  // 写
GPIO_ToggleBits(GPIOF, GPIO_Pin_9);   // 翻转 (F4新增)
GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);  // 读

// AF配置
GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM3);
```

### HAL库写法
```c
// 时钟使能 (通常在MSP中自动生成)
__HAL_RCC_GPIOF_CLK_ENABLE();

// 初始化
GPIO_InitTypeDef GPIO_InitStruct = {0};
GPIO_InitStruct.Pin = GPIO_PIN_9;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

// 操作
HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_SET);    // 写
HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);                   // 读
HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_9);                 // 翻转

// AF配置 (HAL库自动处理)
// 在gpio.c的MX_GPIO_Init()中由CubeMX生成
```

### GPIO模式对照
| 功能 | 标准库 | HAL库 |
|------|--------|-------|
| 输入 | `GPIO_Mode_IN` | `GPIO_MODE_INPUT` |
| 输出 | `GPIO_Mode_OUT` + `GPIO_OType_PP` | `GPIO_MODE_OUTPUT_PP` |
| 开漏输出 | `GPIO_Mode_OUT` + `GPIO_OType_OD` | `GPIO_MODE_OUTPUT_OD` |
| 复用推挽 | `GPIO_Mode_AF` + `GPIO_OType_PP` | `GPIO_MODE_AF_PP` |
| 复用开漏 | `GPIO_Mode_AF` + `GPIO_OType_OD` | `GPIO_MODE_AF_OD` |
| 模拟 | `GPIO_Mode_AN` | `GPIO_MODE_ANALOG` |
| 上拉 | `GPIO_PuPd_UP` | `GPIO_PULLUP` |
| 下拉 | `GPIO_PuPd_DOWN` | `GPIO_PULLDOWN` |
| 无上下拉 | `GPIO_PuPd_NOPULL` | `GPIO_NOPULL` |

---

## 三、USART 对比

### 标准库写法
```c
// 初始化
USART_InitTypeDef USART_InitStruct;
USART_InitStruct.USART_BaudRate = 115200;
USART_InitStruct.USART_WordLength = USART_WordLength_8b;
USART_InitStruct.USART_StopBits = USART_StopBits_1;
USART_InitStruct.USART_Parity = USART_Parity_No;
USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
USART_InitStruct.USART_Over = USART_Over_16;  // F4新增
USART_Init(USART3, &USART_InitStruct);
USART_Cmd(USART3, ENABLE);

// 发送
USART_SendData(USART3, data);
while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);

// 接收
while (USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == RESET);
uint8_t data = USART_ReceiveData(USART3);

// 中断
USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
void USART3_IRQHandler(void) { /* 直接处理 */ }
```

### HAL库写法
```c
// 初始化 (CubeMX生成)
UART_HandleTypeDef huart3;
huart3.Instance = USART3;
huart3.Init.BaudRate = 115200;
huart3.Init.WordLength = UART_WORDLENGTH_8B;
huart3.Init.StopBits = UART_STOPBITS_1;
huart3.Init.Parity = UART_PARITY_NONE;
huart3.Init.Mode = UART_MODE_TX_RX;
huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
huart3.Init.OverSampling = UART_OVERSAMPLING_16;
HAL_UART_Init(&huart3);

// 发送 (阻塞)
HAL_UART_Transmit(&huart3, &data, 1, 1000);

// 发送 (中断)
HAL_UART_Transmit_IT(&huart3, &data, 1);

// 发送 (DMA)
HAL_UART_Transmit_DMA(&huart3, buf, len);

// 接收
HAL_UART_Receive(&huart3, &data, 1, 100);
HAL_UART_Receive_IT(&huart3, &data, 1);

// 中断回调
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) { /* 回调 */ }
```

---

## 四、TIM 对比

### 标准库写法
```c
// 基本定时器
TIM_TimeBaseInitTypeDef TIM_InitStruct;
TIM_InitStruct.TIM_Period = 9999;
TIM_InitStruct.TIM_Prescaler = 83;
TIM_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
TIM_TimeBaseInit(TIM2, &TIM_InitStruct);
TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
TIM_Cmd(TIM2, ENABLE);

// PWM
TIM_OCInitTypeDef TIM_OCInitStruct;
TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
TIM_OCInitStruct.TIM_Pulse = 500;
TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
TIM_OC1Init(TIM3, &TIM_OCInitStruct);
TIM_SetCompare1(TIM3, compare);  // 更新占空比

// 中断
void TIM2_IRQHandler(void) {
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        // 直接处理
    }
}
```

### HAL库写法
```c
// 基本定时器
TIM_HandleTypeDef htim2;
htim2.Instance = TIM2;
htim2.Init.Period = 9999;
htim2.Init.Prescaler = 83;
htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
HAL_TIM_Base_Init(&htim2);
HAL_TIM_Base_Start_IT(&htim2);

// PWM
TIM_HandleTypeDef htim3;
htim3.Instance = TIM3;
htim3.Init.Period = 999;
htim3.Init.Prescaler = 83;
HAL_TIM_PWM_Init(&htim3);
HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 500);  // 更新占空比

// 中断回调
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
        // 回调处理
    }
}
```

---

## 五、EXTI 对比

### 标准库写法
```c
// 初始化
EXTI_InitTypeDef EXTI_InitStruct;
SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);  // F4: SYSCFG

EXTI_InitStruct.EXTI_Line = EXTI_Line0;
EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
EXTI_InitStruct.EXTI_LineCmd = ENABLE;
EXTI_Init(&EXTI_InitStruct);

// ISR中处理
void EXTI0_IRQHandler(void) {
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
        // 直接业务逻辑
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}
```

### HAL库写法
```c
// 初始化 (CubeMX生成)
GPIO_InitStruct.Pin = GPIO_PIN_0;
GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
HAL_NVIC_EnableIRQ(EXTI0_IRQn);

// ISR
void EXTI0_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);  // 调用HAL处理
}

// 回调
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_0) {
        // 回调中处理
    }
}
```

---

## 六、ADC 对比

### 标准库写法
```c
// 初始化
ADC_InitTypeDef ADC_InitStruct;
ADC_InitStruct.ADC_Mode = ADC_Mode_Independent;
ADC_InitStruct.ADC_ScanConvMode = DISABLE;
ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
ADC_InitStruct.ADC_ChannelCount = 1;  // F4新增
ADC_Init(ADC1, &ADC_InitStruct);

// 通道配置
ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_84Cycles5);

// 转换
ADC_Cmd(ADC1, ENABLE);
ADC_SoftwareStartConv(ADC1);
while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
uint16_t value = ADC_GetConversionValue(ADC1);
```

### HAL库写法
```c
// 初始化 (CubeMX生成)
ADC_HandleTypeDef hadc1;
hadc1.Instance = ADC1;
hadc1.Init.ScanConvMode = ADC_SCANCONVMODE_DISABLE;
hadc1.Init.ContinuousConvMode = DISABLE;
hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
hadc1.Init.NbrOfConversion = 1;
HAL_ADC_Init(&hadc1);

// 通道配置
ADC_ChannelConfTypeDef sConfig;
sConfig.Channel = ADC_CHANNEL_0;
sConfig.Rank = ADC_REGULAR_RANK_1;
sConfig.SamplingTime = ADC_SAMPLETIME_84CYCLES_5;
HAL_ADC_ConfigChannel(&hadc1, &sConfig);

// 转换
HAL_ADC_Start(&hadc1);
HAL_ADC_PollForConversion(&hadc1, 100);
uint16_t value = HAL_ADC_GetValue(&hadc1);
```

---

## 七、Flash 对比

### 标准库写法
```c
// 解锁
FLASH_Unlock();
FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP);

// 擦除扇区 (F4使用扇区擦除)
FLASH_EraseSector(FLASH_Sector_6, FLASH_VOLTAGE_RANGE_3);
while (FLASH_GetStatus() == FLASH_BUSY);

// 编程
FLASH_ProgramWord(address, data);
while (FLASH_GetStatus() == FLASH_BUSY);

// 上锁
FLASH_Lock();

// 读取
uint32_t data = *(__IO uint32_t*)address;
```

### HAL库写法
```c
// 解锁
HAL_FLASH_Unlock();
__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_BSY | FLASH_FLAG_EOP);

// 擦除 (使用Erase函数)
FLASH_Erase_Sector(FLASH_SECTOR_6, FLASH_TYPEERASE_SECTOR, 0);

// 编程
HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, data);

// 上锁
HAL_FLASH_Lock();

// 读取 (相同: 直接指针访问)
uint32_t data = *(__IO uint32_t*)address;
```

---

## 八、NVIC 对比

### 标准库写法
```c
NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

NVIC_InitTypeDef NVIC_InitStruct;
NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
NVIC_Init(&NVIC_InitStruct);
```

### HAL库写法
```c
HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_2);
HAL_NVIC_SetPriority(USART3_IRQn, 1, 0);
HAL_NVIC_EnableIRQ(USART3_IRQn);
```

---

## 九、代码风格对比

### 标准库 - 直接操作
```c
// 风格1: 结构体初始化 + API调用
GPIO_InitTypeDef GPIO_InitStruct;
GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
// ...
GPIO_Init(GPIOF, &GPIO_InitStruct);

// 风格2: 寄存器直接操作 (更快)
GPIOF->BSRR = GPIO_Pin_9;  // 置位
GPIOF->BRR = GPIO_Pin_9;  // 复位
GPIOF->ODR ^= GPIO_Pin_9;  // 翻转
```

### HAL库 - 硬件抽象
```c
// 风格: 句柄 + 初始化 + 操作
GPIO_InitTypeDef GPIO_InitStruct = {0};
GPIO_InitStruct.Pin = GPIO_PIN_9;
// ...
HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

// 操作
HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_SET);
HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_9);
```

---

## 十、选择建议

### 选标准库的场景
1. **性能要求高**: 直接操作寄存器，代码效率高
2. **资源受限**: 代码量小，节省Flash/RAM
3. **底层开发**: 需要精确控制硬件
4. **不想用CubeMX**: 手动配置所有外设
5. **已有标准库经验**: 从F103迁移过来
6. **裸机开发**: 不使用RTOS，追求极简

### 选HAL库的场景
1. **快速原型**: 使用CubeMX一键配置
2. **项目维护**: 代码规范统一，易读易维护
3. **跨芯片开发**: F4系列其他芯片可复用
4. **FreeRTOS**: CMSIS-RTOS直接兼容
5. **团队协作**: 标准化API，沟通成本低
6. **长期支持**: ST官方持续更新HAL库

### 混合使用注意
- **不推荐混用**: 可能导致冲突
- 如果混用: 确保只有一套API操作同一外设
- 常见混用场景: HAL初始化 + 标准库优化操作

---

## 十一、常用功能对照速查表

| 功能 | 标准库 API | HAL库 API |
|------|-----------|-----------|
| 延时 | `Delay_ms()` 自定义 | `HAL_Delay(ms)` |
| 时间戳 | 自定义 `millis()` | `HAL_GetTick()` |
| LED亮 | `GPIO_ResetBits()` | `HAL_GPIO_WritePin(RESET)` |
| LED灭 | `GPIO_SetBits()` | `HAL_GPIO_WritePin(SET)` |
| LED翻转 | `GPIO_ToggleBits()` | `HAL_GPIO_TogglePin()` |
| 读按键 | `GPIO_ReadInputDataBit()` | `HAL_GPIO_ReadPin()` |
| 串口发 | `USART_SendData()` | `HAL_UART_Transmit()` |
| 串口收 | `USART_ReceiveData()` | `HAL_UART_Receive()` |
| PWM设置 | `TIM_SetCompare1()` | `__HAL_TIM_SET_COMPARE()` |
| ADC读 | `ADC_GetConversionValue()` | `HAL_ADC_GetValue()` |
| Flash写 | `FLASH_ProgramWord()` | `HAL_FLASH_Program()` |
| Flash擦 | `FLASH_EraseSector()` | `FLASH_Erase_Sector()` |
| 使能时钟 | `RCC_AHB1PeriphClockCmd()` | `__HAL_RCC_xxx_CLK_ENABLE()` |
| NVIC配置 | `NVIC_Init()` | `HAL_NVIC_SetPriority()` + `HAL_NVIC_EnableIRQ()` |
| EXTI处理 | ISR中直接写逻辑 | ISR → HAL → Callback |

---

## 十二、迁移路径

### 标准库 → HAL库
```
1. 了解CubeMX基础
   ├── 安装CubeMX
   ├── 选择芯片F407VETx
   └── 配置时钟 (168MHz)

2. 从简单外设开始
   ├── GPIO → LED/按键
   ├── USART → 串口打印
   ├── TIM → 定时/PWM
   └── EXTI → 外部中断

3. 学习HAL特有功能
   ├── MSP初始化分离
   ├── 回调函数机制
   ├── 状态机编程
   └── DMA操作

4. 进阶功能
   ├── ADC + DMA
   ├── SPI/I2C
   ├── Flash读写
   └── F4特有外设

5. 完整迁移
   ├── FreeRTOS集成
   ├── Middleware移植
   └── 代码重构
```

### HAL库 → 标准库
```
1. 了解寄存器结构
   ├── 查看芯片参考手册
   ├── 理解外设寄存器映射
   └── 掌握位操作

2. 获取标准库
   ├── 下载STM32F4xx_StdPeriph_Lib
   ├── 添加到工程
   └── 配置头文件路径

3. 替换HAL API
   ├── HAL_GPIO_Init → GPIO_Init
   ├── HAL_UART_Init → USART_Init
   ├── HAL_TIM_Init → TIM_TimeBaseInit
   └── ...

4. 注意F4特有改动
   ├── AF配置: GPIO_PinAFConfig
   ├── EXTI时钟: SYSCFG (非AFIO)
   ├── GPIO结构: 新增OType, PuPd
   └── Flash: 扇区擦除

5. 优化代码
   ├── 直接寄存器访问
   ├── 减少函数调用
   └── 优化中断处理
```

---

## 十三、调试技巧

### 标准库调试
```c
// 1. 检查时钟
if (SystemCoreClock != 168000000) {
    // 时钟配置错误
}

// 2. 检查GPIO状态
uint32_t port_value = GPIOA->IDR;  // 读输入寄存器
uint32_t port_output = GPIOF->ODR;  // 读输出寄存器

// 3. 检查USART状态
if (USART_GetFlagStatus(USART3, USART_FLAG_ORE) != RESET) {
    // 溢出错误，清除并恢复
    USART_ClearFlag(USART3, USART_FLAG_ORE);
}

// 4. 使用GPIO指示调试
GPIO_ResetBits(GPIOF, GPIO_Pin_9);  // 进入某个状态时点亮
// ... 执行代码 ...
GPIO_SetBits(GPIOF, GPIO_Pin_9);   // 退出时熄灭
```

### HAL库调试
```c
// 1. 检查返回值
HAL_StatusTypeDef status = HAL_UART_Transmit(...);
if (status != HAL_OK) {
    // 处理错误
    if (status == HAL_BUSY) { /* 忙 */ }
    if (status == HAL_TIMEOUT) { /* 超时 */ }
    if (status == HAL_ERROR) { /* 错误 */ }
}

// 2. 检查HAL错误码
uint32_t hal_error = HAL_UART_GetError(&huart3);
if (hal_error != HAL_UART_ERROR_NONE) {
    // 具体错误: HAL_UART_ERROR_ORE, FE, NE, PE
}

// 3. 使用调试器
// 在 HAL_ErrorCallback() 中设置断点
void HAL_ErrorCallback(void) {
    __disable_irq();
    while (1) {
        // 死循环，等待调试器连接
    }
}
```

---

## 十四、常用链接

### 官方资源
- [STM32F4 HAL库用户手册](https://www.st.com/resource/en/user_manual/description/stm32f407312317_hal_driver_user_um1725.html)
- [STM32F4 StdPeriph库用户手册](https://www.st.com/resource/en/user_manual/description/stm32f407312317_std_peripheral_library_um1420.html)
- [STM32F407参考手册](https://www.st.com/resource/en/reference_manual/description/stm32f40717317_02_reference_rm0090.html)
- [STM32F407数据手册](https://www.st.com/resource/en/datasheet/stm32f407vet6)

### 开发工具
- [STM32CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html)
- [Keil MDK](https://www.keil.com/)
- [IAR EWARM](https://www.iar.com/iar-embedded/tools/)
- [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html)
