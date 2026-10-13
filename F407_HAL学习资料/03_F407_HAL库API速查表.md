# STM32F407 HAL库 API 速查表

## 一、通用API

### 初始化与延时
| API | 说明 |
|-----|------|
| `HAL_Init()` | 初始化HAL库，配置SysTick (必须最先调用) |
| `HAL_Delay(ms)` | 毫秒级阻塞延时 (基于SysTick) |
| `HAL_GetTick()` | 获取系统运行时间 (ms) |

### 时钟
| API | 说明 |
|-----|------|
| `SystemClock_Config()` | 系统时钟配置 (CubeMX自动生成) |
| `HAL_RCC_OscConfig()` | 配置振荡器 (HSE/HSI/PLL) |
| `HAL_RCC_ClockConfig()` | 配置总线时钟 |
| `SystemCoreClock` | 全局变量，当前系统主频 (Hz) |

### 使能时钟 (宏)
```c
__HAL_RCC_GPIOA_CLK_ENABLE();    // 开启GPIOA时钟
__HAL_RCC_GPIOB_CLK_ENABLE();    // 开启GPIOB时钟
__HAL_RCC_GPIOC_CLK_ENABLE();    // 开启GPIOC时钟
__HAL_RCC_GPIOD_CLK_ENABLE();    // 开启GPIOD时钟 (F4新增)
__HAL_RCC_GPIOE_CLK_ENABLE();    // 开启GPIOE时钟 (F4新增)
__HAL_RCC_GPIOF_CLK_ENABLE();    // 开启GPIOF时钟 (F4新增)
__HAL_RCC_GPIOG_CLK_ENABLE();    // 开启GPIOG时钟 (F4新增)
__HAL_RCC_GPIOH_CLK_ENABLE();    // 开启GPIOH时钟 (F4新增)
__HAL_RCC_USART1_CLK_ENABLE();   // 开启USART1时钟
__HAL_RCC_USART2_CLK_ENABLE();   // 开启USART2时钟
__HAL_RCC_USART3_CLK_ENABLE();   // 开启USART3时钟
__HAL_RCC_UART4_CLK_ENABLE();    // 开启UART4时钟 (F4新增)
__HAL_RCC_UART5_CLK_ENABLE();    // 开启UART5时钟 (F4新增)
__HAL_RCC_UART6_CLK_ENABLE();    // 开启UART6时钟 (F4新增)
__HAL_RCC_TIM2_CLK_ENABLE();     // 开启TIM2时钟
__HAL_RCC_TIM3_CLK_ENABLE();     // 开启TIM3时钟
__HAL_RCC_ADC1_CLK_ENABLE();     // 开启ADC1时钟
__HAL_RCC_ADC2_CLK_ENABLE();     // 开启ADC2时钟 (F4新增)
__HAL_RCC_ADC3_CLK_ENABLE();     // 开启ADC3时钟 (F4新增)
__HAL_RCC_SPI1_CLK_ENABLE();     // 开启SPI1时钟
__HAL_RCC_SPI2_CLK_ENABLE();     // 开启SPI2时钟
__HAL_RCC_I2C1_CLK_ENABLE();     // 开启I2C1时钟
__HAL_RCC_I2C2_CLK_ENABLE();     // 开启I2C2时钟
__HAL_RCC_I2C3_CLK_ENABLE();     // 开启I2C3时钟 (F4新增)
__HAL_RCC_I2C4_CLK_ENABLE();     // 开启I2C4时钟 (F4新增)
__HAL_RCC_ETH_CLK_ENABLE();      // 开启以太网时钟 (F4新增)
__HAL_RCC_SDIO_CLK_ENABLE();     // 开启SDIO时钟 (F4新增)
__HAL_RCC_USB_OTG_FS_CLK_ENABLE(); // 开启USB OTG FS时钟 (F4新增)
__HAL_RCC_FMC_CLK_ENABLE();      // 开启FMC时钟 (F4新增)
__HAL_RCC_LTDC_CLK_ENABLE();     // 开启LCD-TFT时钟 (F4新增)
__HAL_RCC_RNG_CLK_ENABLE();      // 开启RNG时钟 (F4新增)
__HAL_RCC_CRYP_CLK_ENABLE();     // 开启CRYP时钟 (F4新增)
__HAL_RCC_HASH_CLK_ENABLE();     // 开启HASH时钟 (F4新增)
__HAL_RCC_DCMI_CLK_ENABLE();     // 开启DCMI时钟 (F4新增)
// 规律: __HAL_RCC_外设名_CLK_ENABLE()
```

---

## 二、GPIO API

### 初始化
```c
void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init);
```

### 操作
| API | 说明 |
|-----|------|
| `HAL_GPIO_WritePin(GPIOx, GPIO_PIN_x, GPIO_PIN_SET/RESET)` | 写引脚 |
| `HAL_GPIO_ReadPin(GPIOx, GPIO_PIN_x)` | 读引脚 (返回GPIO_PIN_SET/RESET) |
| `HAL_GPIO_TogglePin(GPIOx, GPIO_PIN_x)` | 翻转引脚 |
| `HAL_GPIO_LockPin(GPIOx, GPIO_PIN_x)` | 锁定引脚 (不可修改) |

### 模式参数
```c
GPIO_InitStruct.Mode:
  GPIO_MODE_INPUT            // 输入
  GPIO_MODE_OUTPUT_PP        // 推挽输出
  GPIO_MODE_OUTPUT_OD        // 开漏输出
  GPIO_MODE_AF_PP            // 复用推挽
  GPIO_MODE_AF_OD            // 复用开漏
  GPIO_MODE_ANALOG           // 模拟
  GPIO_MODE_IT_RISING        // 上升沿中断
  GPIO_MODE_IT_FALLING       // 下降沿中断
  GPIO_MODE_IT_RISING_FALLING // 双边沿中断

GPIO_InitStruct.Pull:
  GPIO_NOPULL    // 无上拉/下拉
  GPIO_PULLUP    // 上拉
  GPIO_PULLDOWN  // 下拉

GPIO_InitStruct.Speed:
  GPIO_SPEED_FREQ_LOW       // < 2MHz
  GPIO_SPEED_FREQ_MEDIUM    // < 25MHz
  GPIO_SPEED_FREQ_HIGH      // < 50MHz
  GPIO_SPEED_FREQ_VERY_HIGH // < 100MHz (F4新增)
```

---

## 三、USART API

### 初始化
```c
HAL_StatusTypeDef HAL_UART_Init(UART_HandleTypeDef *huart);
HAL_StatusTypeDef HAL_UART_DeInit(UART_HandleTypeDef *huart);
```

### 发送
| API | 说明 |
|-----|------|
| `HAL_UART_Transmit(huart, data, len, timeout)` | 阻塞发送 |
| `HAL_UART_Transmit_IT(huart, data, len)` | 中断发送 |
| `HAL_UART_Transmit_DMA(huart, data, len)` | DMA发送 |

### 接收
| API | 说明 |
|-----|------|
| `HAL_UART_Receive(huart, data, len, timeout)` | 阻塞接收 |
| `HAL_UART_Receive_IT(huart, data, len)` | 中断接收 |
| `HAL_UART_Receive_DMA(huart, data, len)` | DMA接收 |

### 控制
| API | 说明 |
|-----|------|
| `HAL_UART_Abort(huart)` | 中止所有操作 |
| `HAL_UART_Transmit_DMA_IT(huart, data, len)` | 中断DMA发送 |
| `HAL_UART_Receive_DMA_IT(huart, data, len)` | 中断DMA接收 |

### 初始化结构体
```c
huart.Init:
  .BaudRate           // 波特率 (如 115200)
  .WordLength         // UART_WORDLENGTH_8B / 9B
  .StopBits           // UART_STOPBITS_1 / 2
  .Parity             // UART_PARITY_NONE / EVEN / ODD
  .Mode               // UART_MODE_TX / RX / TX_RX
  .HwFlowCtl          // UART_HWCONTROL_NONE / RTS / CTS / RTS_CTS
  .OverSampling       // UART_OVERSAMPLING_16 / 8 (F4新增)
```

### 回调函数
```c
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);    // 发送完成
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);    // 接收完成
void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart); // 半发送完成
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart); // 半接收完成
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);     // 错误回调
```

---

## 四、SPI API

### 初始化
```c
HAL_StatusTypeDef HAL_SPI_Init(SPI_HandleTypeDef *hspi);
```

### 发送/接收
| API | 说明 |
|-----|------|
| `HAL_SPI_Transmit(hspi, data, len, timeout)` | 阻塞发送 |
| `HAL_SPI_Receive(hspi, data, len, timeout)` | 阻塞接收 |
| `HAL_SPI_TransmitReceive(hspi, tx, rx, len, timeout)` | 同时收发 |
| `HAL_SPI_Transmit_IT(hspi, data, len)` | 中断发送 |
| `HAL_SPI_Receive_IT(hspi, data, len)` | 中断接收 |
| `HAL_SPI_TransmitReceive_IT(hspi, tx, rx, len)` | 中断同时收发 |
| `HAL_SPI_Transmit_DMA(hspi, data, len)` | DMA发送 |
| `HAL_SPI_Receive_DMA(hspi, data, len)` | DMA接收 |

### 初始化结构体
```c
hspi.Init:
  .Mode               // SPI_MODE_MASTER / SLAVE
  .Direction          // SPI_DIRECTION_2LINES / 1LINE_RX / 1LINE_TX
  .DataSize           // SPI_DATASIZE_8BIT / 16BIT
  .Polarity           // SPI_POLARITY_LOW / HIGH
  .Phase              // SPI_PHASE_1EDGE / 2EDGE
  .NSS                // SPI_NSS_SOFT / HARD
  .BaudRatePrescaler  // SPI_BAUDRATEPRESCALER_2 ~ 256
  .FirstBit           // SPI_FIRSTBIT_MSB / LSB
  .CRCPolynomial      // CRC多项式
```

---

## 五、I2C API

### 初始化
```c
HAL_StatusTypeDef HAL_I2C_Init(I2C_HandleTypeDef *hi2c);
```

### 主模式传输
| API | 说明 |
|-----|------|
| `HAL_I2C_Master_Transmit(hi2c, addr, data, len, timeout)` | 阻塞主发送 |
| `HAL_I2C_Master_Receive(hi2c, addr, data, len, timeout)` | 阻塞主接收 |
| `HAL_I2C_Master_Transmit_IT(hi2c, addr, data, len)` | 中断主发送 |
| `HAL_I2C_Master_Receive_IT(hi2c, addr, data, len)` | 中断主接收 |
| `HAL_I2C_Master_Transmit_DMA(hi2c, addr, data, len)` | DMA主发送 |
| `HAL_I2C_Master_Receive_DMA(hi2c, addr, data, len)` | DMA主接收 |

### 内存地址传输
| API | 说明 |
|-----|------|
| `HAL_I2C_Mem_Write(hi2c, addr, reg, size, data, len, timeout)` | 写寄存器 |
| `HAL_I2C_Mem_Read(hi2c, addr, reg, size, data, len, timeout)` | 读寄存器 |
| `HAL_I2C_Mem_Write_IT(hi2c, addr, reg, size, data, len)` | 中断写 |
| `HAL_I2C_Mem_Read_IT(hi2c, addr, reg, size, data, len)` | 中断读 |

### 初始化结构体
```c
hi2c.Init:
  .Timing              // 时序参数 (CubeMX计算)
  .OwnAddress1         // 自身地址
  .AddressingMode      // I2C_ADDRESSINGMODE_7BIT / 10BIT
  .DualAddressMode     // I2C_DUALADDRESS_DISABLE / ENABLE
  .OwnAddress2         // 第二地址
  .GeneralCallMode     // I2C_GENERALCALL_DISABLE / ENABLE
  .NoStretchMode       // I2C_NOSTRETCH_DISABLE / ENABLE
```

---

## 六、EXTI API

### 配置
```c
// 通过GPIO_InitStruct配置 (见GPIO部分)
// 或使用:
void HAL_NVIC_SetPriority(IRQn, PreemptionPriority, SubPriority);
void HAL_NVIC_EnableIRQ(IRQn);
void HAL_NVIC_DisableIRQ(IRQn);
```

### 处理
| API | 说明 |
|-----|------|
| `HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_x)` | EXTI中断处理 (在stm32f4xx_it.c中调用) |
| `HAL_GPIO_EXTI_Callback(GPIO_PIN_x)` | 业务回调 (用户实现) |

---

## 七、TIM API

### 基本定时器
| API | 说明 |
|-----|------|
| `HAL_TIM_Base_Init(htim)` | 初始化 |
| `HAL_TIM_Base_Start(htim)` | 启动 (阻塞) |
| `HAL_TIM_Base_Start_IT(htim)` | 启动 (中断) |
| `HAL_TIM_Base_Start_DMA(htim, data, len)` | 启动 (DMA) |
| `HAL_TIM_Base_Stop(htim)` | 停止 |
| `HAL_TIM_Base_Stop_IT(htim)` | 停止 (中断) |

### PWM
| API | 说明 |
|-----|------|
| `HAL_TIM_PWM_Start(htim, channel)` | 启动PWM |
| `HAL_TIM_PWM_Stop(htim, channel)` | 停止PWM |
| `__HAL_TIM_SET_COMPARE(htim, channel, value)` | 设置比较值(占空比) |
| `__HAL_TIM_GET_COMPARE(htim, channel)` | 获取比较值 |
| `__HAL_TIM_SET_AUTORELOAD(htim, value)` | 设置自动重载值 |

### 输入捕获
| API | 说明 |
|-----|------|
| `HAL_TIM_IC_Start(htim, channel)` | 启动输入捕获 |
| `HAL_TIM_IC_Start_IT(htim, channel)` | 启动输入捕获(中断) |
| `HAL_TIM_IC_GetCapture(htim, channel)` | 获取捕获值 |

### 输出比较
| API | 说明 |
|-----|------|
| `HAL_TIM_OC_Start(htim, channel)` | 启动输出比较 |
| `HAL_TIM_OC_Start_IT(htim, channel)` | 启动输出比较(中断) |

### 编码器接口 (F4新增)
| API | 说明 |
|-----|------|
| `HAL_TIM_Encoder_Init(htim, config)` | 编码器初始化 |
| `HAL_TIM_Encoder_Start(htim)` | 启动编码器 |
| `HAL_TIM_Encoder_Start_IT(htim)` | 启动编码器(中断) |
| `HAL_TIM_Encoder_GetValue(htim)` | 读取编码器值 |

### 回调函数
```c
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);  // 定时溢出
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim);  // 输出比较
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);       // 输入捕获
void HAL_TIM_ErrorCallback(TIM_HandleTypeDef *htim);            // 错误
void HAL_TIM_TriggerCallback(TIM_HandleTypeDef *htim);          // 触发
void HAL_TIM_CommCallback(TIM_HandleTypeDef *htim);             // 换向(高级定时器)
void HAL_TIM_BreakCallback(TIM_HandleTypeDef *htim);            // 刹车(高级定时器)
```

---

## 八、ADC API

### 单通道
| API | 说明 |
|-----|------|
| `HAL_ADC_Init(hadc)` | 初始化 |
| `HAL_ADC_Start(hadc)` | 启动转换 |
| `HAL_ADC_PollForConversion(hadc, timeout)` | 等待转换完成 |
| `HAL_ADC_GetValue(hadc)` | 读取转换结果 |
| `HAL_ADC_Stop(hadc)` | 停止转换 |

### 中断/DMA
| API | 说明 |
|-----|------|
| `HAL_ADC_Start_IT(hadc)` | 启动 (中断) |
| `HAL_ADC_Start_DMA(hadc, data, len)` | 启动 (DMA) |
| `HAL_ADC_Stop_IT(hadc)` | 停止 (中断) |
| `HAL_ADC_Stop_DMA(hadc)` | 停止 (DMA) |

### 多ADC同步 (F4新增)
| API | 说明 |
|-----|------|
| `HAL_ADCEx_InjectedConfigChannel(hadc, channel, rank, time)` | 注入通道配置 |
| `HAL_ADCEx_MultiModeStart_DMA(hadc, data, len)` | 多ADC DMA启动 |

### 回调
```c
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc);     // 转换完成
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc); // 半转换完成
void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef *hadc); // 超限
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc);         // 错误
void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *hadc); // 注入转换完成
```

---

## 九、Flash API

### 擦除
| API | 说明 |
|-----|------|
| `HAL_FLASH_Unlock()` | 解锁Flash |
| `HAL_FLASH_Lock()` | 上锁Flash |
| `FLASH_Erase_Sector(Sector, TypeErase, VoltageRange)` | 擦除扇区 |
| `__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG)` | 清除标志 |

### 编程
| API | 说明 |
|-----|------|
| `HAL_FLASH_Program(TypeProgram, Address, Data)` | 编程 |
| `HAL_FLASH_Program_Parallel(Address, Data)` | 并行编程 (F4新增) |

### 编程类型
```c
FLASH_TYPEPROGRAM_BYTE        // 8位
FLASH_TYPEPROGRAM_HALFWORD     // 16位
FLASH_TYPEPROGRAM_WORD        // 32位
FLASH_TYPEPROGRAM_DOUBLEWORD  // 64位 (F4新增)
```

### 扇区常量 (F407VET6)
```c
FLASH_SECTOR_0   // 16KB (0x08000000)
FLASH_SECTOR_1   // 16KB (0x08004000)
FLASH_SECTOR_2   // 16KB (0x08008000)
FLASH_SECTOR_3   // 16KB (0x0800C000)
FLASH_SECTOR_4   // 64KB (0x08010000)
FLASH_SECTOR_5   // 128KB (0x08020000)
FLASH_SECTOR_6   // 128KB (0x08040000)
FLASH_SECTOR_7   // 128KB (0x08060000)
```

---

## 十、F407独有外设API

### 以太网 (ETH)
```c
HAL_ETH_Init(heth);
HAL_ETH_Start(heth);
HAL_ETH_Transmit(heth, frame, len, timeout);
HAL_ETH_Receive(heth, frame, len, timeout);
// 注意: 需要移植LwIP协议栈
```

### USB OTG FS
```c
HAL_PCD_Init(hpcd);           // 设备模式
HAL_PCD_Start(hpcd);
HAL_PCD_Transmit(hpcd, data, len);
HAL_HCD_Init(hhcd);           // 主机模式
HAL_HCD_Start(hhcd);
```

### SDIO
```c
HAL_SD_InitCard(&hsd);
HAL_SD_ReadBlocks(&hsd, data, sector, count, timeout);
HAL_SD_WriteBlocks(&hsd, data, sector, count, timeout);
// 注意: 需要移植FatFs文件系统
```

### LCD-TFT
```c
HAL_LTDC_Init(&hltdc);
HAL_LTDC_Enable(&hltdc);
HAL_LTDC_CopyBuffer(&hltdc, src, dst, size);
```

### RNG (随机数)
```c
HAL_RNG_Init(&hrng);
HAL_RNG_GenerateRandomNumber(&hrng, &random32);
```

### CRYP (加密)
```c
HAL_CRYP_Init(&hcryp);
HAL_CRYP_Encrypt(&hcryp, data, len, output, timeout);
HAL_CRYP_Decrypt(&hcryp, data, len, output, timeout);
```

### HASH (哈希)
```c
HASH_HandleTypeDef hhash;
HAL_HASH_Init(&hhash);
HAL_HASH_Start(&hhash, data, len, hash, timeout);
```

### FMC (存储控制器)
```c
HAL_SRAM_Init(&hsram1);  // SRAM接口
HAL_NOR_Init(&hnor1);    // NOR Flash接口
HAL_NAND_Init(&hnand1);  // NAND Flash接口
```

---

## 十一、NVIC 优先级

### 优先级分组
```c
// 在 main.c 或系统初始化时设置:
HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_2);
// 优先级分组决定了抢占优先级和子优先级的位数分配
// F407 (Cortex-M4) 支持: 4位优先级 (抢占+子)
//   NVIC_PRIORITYGROUP_0: 0抢占 + 4子
//   NVIC_PRIORITYGROUP_1: 1抢占 + 3子
//   NVIC_PRIORITYGROUP_2: 2抢占 + 2子 (常用)
//   NVIC_PRIORITYGROUP_3: 3抢占 + 1子
//   NVIC_PRIORITYGROUP_4: 4抢占 + 0子
```

### 设置中断优先级
```c
HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);  // 抢占=2, 子=0
HAL_NVIC_EnableIRQ(EXTI0_IRQn);
```

---

## 十二、状态码

```c
typedef enum {
    HAL_OK       = 0x00,
    HAL_ERROR    = 0x01,
    HAL_BUSY     = 0x02,
    HAL_TIMEOUT  = 0x03
} HAL_StatusTypeDef;

// 检查返回值:
if (HAL_UART_Transmit(&huart3, data, len, 1000) != HAL_OK) {
    // 发送失败
}
```

---

## 十三、CubeMX 常用配置 (F407)

### 时钟配置 (F407VET6)
```
HSE: 8MHz (外部晶振)
PLLM: 8   → VCO输入 = 1MHz
PLLN: 336 → VCO输出 = 336MHz
PLLP: 2   → SYSCLK = 168MHz
PLLQ: 7   → USB时钟 = 48MHz

AHB:    168MHz  (HPRE = /1)
APB1:    42MHz  (PPRE1 = /4, 最高42MHz)
APB2:    84MHz  (PPRE2 = /2, 最高84MHz)

定时器时钟:
  APB1定时器: 84MHz (当APB1分频>1时自动倍频)
  APB2定时器: 168MHz (当APB2分频>1时自动倍频)
```

### 常见外设基地址
| 外设 | 总线 | 时钟 |
|------|------|------|
| USART1 | APB2 | 84MHz |
| USART2 | APB1 | 42MHz |
| USART3 | APB1 | 42MHz |
| UART4 | APB1 | 42MHz |
| TIM1 | APB2 | 168MHz |
| TIM2-7 | APB1 | 84MHz |
| TIM8 | APB2 | 168MHz |
| ADC1-3 | APB2 | 84MHz |
| GPIOA-H | AHB | 168MHz |
| SPI1 | APB2 | 84MHz |
| SPI2-3 | APB1 | 42MHz |
| I2C1-4 | APB1 | 42MHz |
| CAN1-2 | APB1 | 42MHz |
| ETH | AHB | 168MHz |
| USB_OTG_FS | APB2 | 84MHz |
| SDIO | APB2 | 84MHz |
| LTDC | APB2 | 84MHz |
| FMC | FMC | - |

### F407VET6 可用资源
| 资源 | 数量 | 说明 |
|------|------|------|
| Flash | 512KB | 扇区大小不同 (16K/64K/128K) |
| SRAM | 192KB | 112KB主 + 64KB CCM + 16KB Backup |
| GPIO | 140+ | PA0-15, PB0-15, PC0-15, PD0-15, PE0-15, PF0-15, PG0-15, PH0-1 |
| USART/UART | 6 | USART1-3, UART4-6 |
| TIM | 16 | TIM1-14, TIM15-18 |
| ADC | 3 | 12位, 24通道 |
| SPI | 6 | SPI1-6 |
| I2C | 4 | I2C1-4 |
| CAN | 2 | CAN 2.0B |
| USB | 1 | USB OTG FS |
| ETH | 1 | 10/100 MAC |
| SDIO | 1 | SD卡接口 |
| LCD-TFT | 1 | LCD控制器 |
| FMC | 1 | SRAM/PSRAM/NOR/NAND |
| RNG | 1 | 随机数生成 |
| CRYP | 1 | AES/DES/SM4 |
| HASH | 1 | SHA-1/SHA-256 |
| DCMI | 1 | 摄像头接口 |

---

## 十四、常见问题速查

| 问题 | 解决方案 |
|------|----------|
| `stm32f4xx_hal_conf.h` 找不到 | 从CubeMX工程模板拷贝，或在CubeMX中启用对应外设后自动生成 |
| 编译提示 `undefined symbol` | 检查 `stm32f4xx_hal_conf.h` 中是否启用了对应模块 |
| `HAL_Delay` 卡死 | SysTick中断被禁用，检查NVIC配置 |
| 中断不触发 | 检查优先级、EXTI线配置、NVIC是否使能 |
| UART收不到数据 | 检查GPIO复用配置、波特率、时钟频率 |
| PWM输出为0 | 检查GPIO是否配置为AF_PP、定时器是否启动 |
| FPU相关错误 | 在启动文件或main中初始化FPU: SCB->CPACR |=(3<<10*2)\|(3<<11*2) |
| Flash写入失败 | 检查扇区大小是否正确、是否已解锁Flash |
| CubeMX生成的代码被修改 | 所有业务逻辑放在 `USER CODE BEGIN/END` 区域内 |
| 以太网不工作 | 检查PHY芯片、RMII连接、LwIP移植 |
| USB不工作 | 检查USB时钟(48MHz)、配置正确的端点 |
