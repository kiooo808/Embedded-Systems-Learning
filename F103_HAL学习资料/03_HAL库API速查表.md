# STM32F103 HAL库 API 速查表

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
__HAL_RCC_AFIO_CLK_ENABLE();     // 开启AFIO时钟
__HAL_RCC_USART1_CLK_ENABLE();   // 开启USART1时钟
__HAL_RCC_TIM2_CLK_ENABLE();     // 开启TIM2时钟
__HAL_RCC_ADC1_CLK_ENABLE();     // 开启ADC1时钟
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
  GPIO_SPEED_FREQ_VERY_HIGH // < 100MHz
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

### 回调函数
```c
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);    // 发送完成
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);    // 接收完成
void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart); // 半发送完成
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart); // 半接收完成
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);     // 错误回调
```

---

## 四、EXTI API

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
| `HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_x)` | EXTI中断处理 (在stm32f1xx_it.c中调用) |
| `HAL_GPIO_EXTI_Callback(GPIO_PIN_x)` | 业务回调 (用户实现) |

---

## 五、TIM API

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

### 输入捕获
| API | 说明 |
|-----|------|
| `HAL_TIM_IC_Start(htim, channel)` | 启动输入捕获 |
| `HAL_TIM_IC_Start_IT(htim, channel)` | 启动输入捕获(中断) |
| `HAL_TIM_IC_GetCapture(htim, channel)` | 获取捕获值 |

### 回调函数
```c
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);  // 定时溢出
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim);  // 输出比较
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);       // 输入捕获
void HAL_TIM_ErrorCallback(TIM_HandleTypeDef *htim);            // 错误
```

---

## 六、ADC API

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

### 回调
```c
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc);     // 转换完成
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc); // 半转换完成
void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef *hadc); // 超限
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc);         // 错误
```

---

## 七、NVIC 优先级

### 优先级分组
```c
// 在 main.c 或系统初始化时设置:
HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_2);
// 优先级分组决定了抢占优先级和子优先级的位数分配
// F103 支持: 4位优先级 (抢占+子)
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

## 八、状态码

```c
typedef enum {
    HAL_OK       = 0x00,
    HAL_ERROR    = 0x01,
    HAL_BUSY     = 0x02,
    HAL_TIMEOUT  = 0x03
} HAL_StatusTypeDef;

// 检查返回值:
if (HAL_UART_Transmit(&huart1, data, len, 1000) != HAL_OK) {
    // 发送失败
}
```

---

## 九、CubeMX 常用配置

### 时钟配置 (F103)
```
SYSCLK: 72MHz  (HSE 8MHz × 9 PLL)
AHB:    72MHz  (HPRE = /1)
APB1:   36MHz  (PPRE1 = /2, TIM2-7时钟=72MHz)
APB2:   72MHz  (PPRE2 = /1, TIM1时钟=72MHz)
```

### 常见外设基地址
| 外设 | 地址 | 说明 |
|------|------|------|
| USART1 | APB2 | 高速串口 |
| USART2 | APB1 | 普通串口 |
| TIM1 | APB2 | 高级定时器 |
| TIM2-7 | APB1 | 通用定时器 |
| ADC1-2 | APB2 | 模数转换 |
| GPIOA-C | APB2 | 通用IO |

### F103C8T6 可用资源
| 资源 | 数量 | 说明 |
|------|------|------|
| Flash | 64KB | 注意代码大小 |
| SRAM | 20KB | 注意栈/堆使用 |
| GPIO | 37 | PA0-15, PB0-15, PC13-15 |
| USART | 3 | USART1/2/3 |
| TIM | 4+2 | TIM1-4基本, TIM1/8高级 |
| ADC | 2 | 12位, 10通道 |
| SPI | 2 | SPI1/SPI2 |
| I2C | 2 | I2C1/I2C2 |
| CAN | 1 | CAN 2.0B |
| USB | 1 | USB 2.0 FS |

---

## 十、常见问题速查

| 问题 | 解决方案 |
|------|----------|
| `stm32f1xx_hal_conf.h` 找不到 | 从CubeMX工程模板拷贝，或在CubeMX中启用对应外设后自动生成 |
| 编译提示 `undefined symbol` | 检查 `stm32f1xx_hal_conf.h` 中是否启用了对应模块 (`#define HAL_GPIO_MODULE_ENABLED` 等) |
| `HAL_Delay` 卡死 | SysTick中断被禁用，检查NVIC配置 |
| 中断不触发 | 检查优先级、EXTI线配置、NVIC是否使能 |
| UART收不到数据 | 检查GPIO复用配置、波特率、时钟频率 |
| PWM输出为0 | 检查GPIO是否配置为AF_PP、定时器是否启动 |
| `MX_xxx_Init()` 报错 | 在CubeMX中重新生成代码 |
| CubeMX生成的代码被修改 | 所有业务逻辑放在 `USER CODE BEGIN/END` 区域外 |