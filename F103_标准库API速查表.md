# STM32F103 标准外设库 (StdPeriph) API 速查表

## 一、通用初始化

### 系统初始化
```c
SystemInit();                    // 启动文件中自动调用
SystemCoreClockUpdate();         // 更新系统时钟变量
SystemCoreClock;                 // 全局变量，当前主频 (Hz)
```

### 时钟使能
```c
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
// 规律:
//   APB2总线: RCC_APB2PeriphClockCmd(外设名, ENABLE)
//   APB1总线: RCC_APB1PeriphClockCmd(外设名, ENABLE)
```

### 外设时钟常量
```c
// APB2 总线外设
RCC_APB2Periph_AFIO     // 复用IO
RCC_APB2Periph_GPIOA    // GPIOA
RCC_APB2Periph_GPIOB    // GPIOB
RCC_APB2Periph_GPIOC    // GPIOC
RCC_APB2Periph_GPIOD    // GPIOD
RCC_APB2Periph_GPIOE    // GPIOE
RCC_APB2Periph_USART1   // USART1
RCC_APB2Periph_SPI1     // SPI1
RCC_APB2Periph_TIM1     // TIM1
RCC_APB2Periph_ADC1     // ADC1
RCC_APB2Periph_ADC2     // ADC2

// APB1 总线外设
RCC_APB1Periph_USART2   // USART2
RCC_APB1Periph_USART3   // USART3
RCC_APB1Periph_SPI2     // SPI2
RCC_APB1Periph_TIM2     // TIM2
RCC_APB1Periph_TIM3     // TIM3
RCC_APB1Periph_TIM4     // TIM4
RCC_APB1Periph_I2C1     // I2C1
RCC_APB1Periph_CAN      // CAN
RCC_APB1Periph_USB      // USB
RCC_APB1Periph_BKP      // 备份寄存器
RCC_APB1Periph_PWR      // 电源控制
```

---

## 二、GPIO API

### 初始化
```c
void GPIO_Init(GPIO_TypeDef* GPIOx, GPIO_InitTypeDef* GPIO_InitStruct);
```

### 操作
| API | 说明 |
|-----|------|
| `GPIO_SetBits(GPIOx, GPIO_Pin_x)` | 置位 (输出高) |
| `GPIO_ResetBits(GPIOx, GPIO_Pin_x)` | 复位 (输出低) |
| `GPIO_WriteBit(GPIOx, GPIO_Pin_x, Bit_SET/Bit_RESET)` | 写指定引脚 |
| `GPIO_ReadInputDataBit(GPIOx, GPIO_Pin_x)` | 读输入引脚 |
| `GPIO_ReadOutputDataBit(GPIOx, GPIO_Pin_x)` | 读输出引脚 |
| `GPIO_ReadInputData(GPIOx)` | 读整个端口 |
| `GPIO_Write(GPIOx, Value)` | 写整个端口 |
| `GPIO_PinRemapConfig(remap, ENABLE)` | 引脚重映射 |

### GPIO_InitTypeDef 结构体
```c
typedef struct {
    uint16_t GPIO_Pin;       // 引脚号
    GPIOSpeed_TypeDef GPIO_Speed;  // 速度
    GPIOMode_TypeDef GPIO_Mode;    // 模式
} GPIO_InitTypeDef;
```

### 引脚常量
```c
GPIO_Pin_0  ~ GPIO_Pin_15
GPIO_Pin_All  // 全部引脚
```

### 模式枚举
```c
GPIO_Mode_AIN       // 模拟输入
GPIO_Mode_IN_FLOATING  // 浮空输入
GPIO_Mode_IPD       // 下拉输入
GPIO_Mode_IPU       // 上拉输入
GPIO_Mode_Out_OD    // 开漏输出
GPIO_Mode_Out_PP    // 推挽输出
GPIO_Mode_AF_OD     // 复用开漏
GPIO_Mode_AF_PP     // 复用推挽
```

### 速度枚举
```c
GPIO_Speed_2MHz     // 低速
GPIO_Speed_25MHz    // 中速
GPIO_Speed_50MHz    // 高速
GPIO_Speed_10MHz    // 超高速 (低功耗)
```

### 位带操作 (直接操作寄存器)
```c
// 原子操作，无需关中断
#define LED_ON()   GPIOC->BSRR = GPIO_Pin_13    // 置位
#define LED_OFF()  GPIOC->BRR = GPIO_Pin_13    // 复位

// 或使用位带别名
#define PA0_IN     BITBAND(GPIOA_BASE, 0)       // PA0 输入
#define PC13_OUT   BITBAND(GPIOC_BASE, 13)      // PC13 输出

// 设置位带区域
#define BITBAND(addr, bitnum) ((addr & 0xF0000000) + 0x2000000 + ((addr & 0xFFFFF) << 5) + (bitnum << 2))
#define MEM_ADDR(addr)  *((volatile unsigned long *)(addr))
#define BITBAND(addr, bitnum) MEM_ADDR(BITBAND(addr, bitnum))
```

---

## 三、USART API

### 初始化
```c
void USART_Init(USART_TypeDef* USARTx, USART_InitTypeDef* USART_InitStruct);
void USART_Cmd(USART_TypeDef* USARTx, FunctionalState NewState);
```

### USART_InitTypeDef 结构体
```c
typedef struct {
    uint32_t USART_BaudRate;       // 波特率
    uint16_t USART_WordLength;     // 字长
    uint16_t USART_StopBits;       // 停止位
    uint16_t USART_Parity;         // 校验位
    uint16_t USART_Mode;           // 收发模式
    uint16_t USART_HardwareFlowControl;  // 硬件流控
} USART_InitTypeDef;
```

### 参数常量
```c
// 字长
USART_WordLength_8b   // 8位
USART_WordLength_9b   // 9位

// 停止位
USART_StopBits_1      // 1位
USART_StopBits_0_5    // 0.5位
USART_StopBits_2      // 2位
USART_StopBits_1_5    // 1.5位

// 校验
USART_Parity_No       // 无校验
USART_Parity_Even     // 偶校验
USART_Parity_Odd      // 奇校验

// 模式
USART_Mode_Rx         // 只接收
USART_Mode_Tx         // 只发送
USART_Mode_Rx | USART_Mode_Tx  // 收发

// 流控
USART_HardwareFlowControl_None      // 无
USART_HardwareFlowControl_RTS      // RTS
USART_HardwareFlowControl_CTS      // CTS
USART_HardwareFlowControl_RTS_CTS  // RTS+CTS
```

### 数据收发
| API | 说明 |
|-----|------|
| `void USART_SendData(USART_TypeDef* USARTx, uint16_t Data)` | 发送数据 |
| `uint16_t USART_ReceiveData(USART_TypeDef* USARTx)` | 接收数据 |

### 状态标志
```c
USART_GetFlagStatus(USART1, USART_FLAG_TXE)   // 发送寄存器空
USART_GetFlagStatus(USART1, USART_FLAG_RXNE)   // 接收寄存器非空
USART_GetFlagStatus(USART1, USART_FLAG_TC)    // 传输完成
USART_GetFlagStatus(USART1, USART_FLAG_ORE)   // 溢出错误
USART_GetFlagStatus(USART1, USART_FLAG_NE)    // 噪声错误
USART_GetFlagStatus(USART1, USART_FLAG_FE)    // 帧错误
USART_GetFlagStatus(USART1, USART_FLAG_PE)    // 校验错误

// 清除标志
USART_ClearFlag(USART1, USART_FLAG_ORE);
USART_ClearITPendingBit(USART1, USART_IT_ORE);
```

### 中断控制
```c
USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);   // 接收中断
USART_ITConfig(USART1, USART_IT_TXE, ENABLE);    // 发送中断
USART_ITConfig(USART1, USART_IT_TC, ENABLE);     // 传输完成中断

// 中断类型
USART_IT_TXE   // 发送空
USART_IT_RXNE  // 接收非空
USART_IT_TC    // 传输完成
USART_IT_ORE   // 溢出错误
USART_IT_NE    // 噪声错误
USART_IT_FE    // 帧错误
USART_IT_PE    // 校验错误
```

---

## 四、EXTI (外部中断) API

### 初始化
```c
void EXTI_Init(EXTI_InitTypeDef* EXTI_InitStruct);
```

### EXTI_InitTypeDef 结构体
```c
typedef struct {
    uint32_t EXTI_Line;       // 中断线
    EXTIMode_TypeDef EXTI_Mode;     // 模式
    EXTITrigger_TypeDef EXTI_Trigger;  // 触发方式
    FunctionalState EXTI_LineCmd;   // 使能
} EXTI_InitTypeDef;
```

### 参数常量
```c
// 中断线
EXTI_Line0   ~ EXTI_Line15
EXTI_LineNone
EXTI_LineAll

// 模式
EXTI_Mode_Interrupt    // 中断
EXTI_Mode_Event        // 事件

// 触发
EXTI_Trigger_Rising         // 上升沿
EXTI_Trigger_Falling        // 下降沿
EXTI_Trigger_Rising_Falling // 双边沿
```

### 中断处理
```c
// 检查中断状态
ITStatus EXTI_GetITStatus(uint32_t EXTI_Line);

// 清除中断标志
void EXTI_ClearITPendingBit(uint32_t EXTI_Line);
```

### 中断服务函数
```c
void EXTI0_IRQHandler(void)   // EXTI0
void EXTI1_IRQHandler(void)   // EXTI1
...
void EXTI15_IRQHandler(void)  // EXTI15
```

---

## 五、TIM (定时器) API

### 基本定时器初始化
```c
void TIM_TimeBaseInit(TIM_TypeDef* TIMx, TIM_TimeBaseInitTypeDef* TIM_TimeBaseInitStruct);
void TIM_Cmd(TIM_TypeDef* TIMx, FunctionalState NewState);
```

### TIM_TimeBaseInitTypeDef 结构体
```c
typedef struct {
    uint16_t TIM_Period;         // 自动重载值 (ARR)
    uint16_t TIM_Prescaler;      // 预分频 (PSC)
    uint8_t TIM_ClockDivision;   // 时钟分频
    TIM_CounterMode_TypeDef TIM_CounterMode;  // 计数模式
    uint16_t TIM_RepetitionCounter;  // 重复计数器 (仅TIM1/TIM8)
} TIM_TimeBaseInitTypeDef;
```

### 参数常量
```c
// 时钟分频
TIM_CKD_DIV1   // 不分频
TIM_CKD_DIV2   // 2分频
TIM_CKD_DIV4   // 4分频
TIM_CKD_DIV8   // 8分频
TIM_CKD_DIV16  // 16分频

// 计数模式
TIM_CounterMode_Up     // 向上计数
TIM_CounterMode_Down   // 向下计数
TIM_CounterMode_CenterAligned1  // 中心对齐1
TIM_CounterMode_CenterAligned2  // 中心对齐2
TIM_CounterMode_CenterAligned3  // 中心对齐3
```

### PWM 输出
```c
void TIM_OCInit(TIM_TypeDef* TIMx, uint8_t TIM_Channel, TIM_OCInitTypeDef* TIM_OCInitStruct);
void TIM_OC1Init(TIM_TypeDef* TIMx, TIM_OCInitTypeDef* TIM_OCInitStruct);
// OC1~OC4 对应 CH1~CH4
```

### TIM_OCInitTypeDef 结构体
```c
typedef struct {
    TIM_OCMode_TypeDef TIM_OCMode;     // 输出比较模式
    uint16_t TIM_Pulse;                // 比较值 (CCR)
    TIM_OCPolarity_TypeDef TIM_OCPolarity;  // 极性
    TIM_OCNState_TypeDef TIM_OCNState;  // 互补输出
    TIM_OCNPolarity_TypeDef TIM_OCNPolarity;  // 互补极性
    uint8_t TIM_OCIdleState;           // 空闲状态
    uint8_t TIM_OCNIdleState;          // 互补空闲状态
} TIM_OCInitTypeDef;
```

### 参数常量
```c
// 通道
TIM_Channel_1  ~ TIM_Channel_4
TIM_Channel_All

// 输出比较模式
TIM_OCMode_Timing     // 冻结
TIM_OCMode_Active     // 主动
TIM_OCMode_Inactive   // 不主动
TIM_OCMode_Toggle     // 翻转
TIM_OCMode_PWM1       // PWM模式1
TIM_OCMode_PWM2       // PWM模式2
TIM_OCMode_OnePulse   // 单次脉冲

// 极性
TIM_OCPolarity_High   // 高电平有效
TIM_OCPolarity_Low    // 低电平有效
```

### 定时器操作
| API | 说明 |
|-----|------|
| `void TIM_SetCompare1(TIM_TypeDef* TIMx, uint16_t Compare1)` | 设置CH1比较值 |
| `void TIM_SetCompare2(TIM_TypeDef* TIMx, uint16_t Compare2)` | 设置CH2比较值 |
| `uint16_t TIM_GetCounter(TIM_TypeDef* TIMx)` | 获取当前计数值 |
| `void TIM_SetCounter(TIM_TypeDef* TIMx, uint16_t Counter)` | 设置计数值 |
| `void TIM_SetAutoreload(TIM_TypeDef* TIMx, uint16_t Autoreload)` | 设置自动重载值 |
| `FlagStatus TIM_GetFlagStatus(TIM_TypeDef* TIMx, uint16_t TIM_FLAG)` | 检查标志 |
| `void TIM_ClearFlag(TIM_TypeDef* TIMx, uint16_t TIM_FLAG)` | 清除标志 |

### 定时器中断
```c
TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);   // 更新中断
TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);      // CH1比较中断
TIM_ITConfig(TIM2, TIM_IT_CC2, ENABLE);      // CH2比较中断

// 中断类型
TIM_IT_Update   // 更新中断
TIM_IT_CC1      // CH1
TIM_IT_CC2      // CH2
TIM_IT_CC3      // CH3
TIM_IT_CC4      // CH4
TIM_IT_Trigger   // 触发
TIM_IT_Break    // 刹车
TIM_IT_Commutation  // 换向

// 中断标志
TIM_FLAG_Update
TIM_FLAG_CC1
TIM_FLAG_CC2
...
```

### 定时器计算
```
时钟频率 = 定时器时钟 / (Prescaler + 1)
定时时间 = (Period + 1) / 时钟频率
PWM频率 = 时钟频率 / (Period + 1)
占空比 = CompareValue / (Period + 1) × 100%

示例 (72MHz 定时器时钟):
  Prescaler = 71  → 时钟频率 = 72MHz / 72 = 1MHz
  Period = 9999   → 定时 = 10000 / 1MHz = 10ms
  Compare = 5000  → PWM占空比 = 5000/10000 = 50%
```

---

## 六、NVIC (中断控制器) API

### 配置
```c
void NVIC_PriorityGroupConfig(uint32_t NVIC_PriorityGroup);
void NVIC_Init(NVIC_InitTypeDef* NVIC_InitStruct);
void NVIC_SetPriority(IRQn_Type IRQn, uint8_t PreemptionPriority, uint8_t SubPriority);
void NVIC_EnableIRQ(IRQn_Type IRQn);
void NVIC_DisableIRQ(IRQn_Type IRQn);
```

### NVIC_InitTypeDef 结构体
```c
typedef struct {
    uint8_t NVIC_IRQChannel;        // 中断通道
    uint8_t NVIC_IRQChannelPreemptionPriority;  // 抢占优先级
    uint8_t NVIC_IRQChannelSubPriority;   // 子优先级
    FunctionalState NVIC_IRQChannelCmd;   // 使能
} NVIC_InitTypeDef;
```

### 优先级分组
```c
NVIC_PriorityGroup_0   // 0抢占 + 4子
NVIC_PriorityGroup_1   // 1抢占 + 3子
NVIC_PriorityGroup_2   // 2抢占 + 2子 (常用)
NVIC_PriorityGroup_3   // 3抢占 + 1子
NVIC_PriorityGroup_4   // 4抢占 + 0子

// 中断号
EXTI0_IRQn   ~ EXTI15_IRQn
USART1_IRQn
TIM2_IRQn
TIM3_IRQn
// ...
```

---

## 七、ADC API

### 初始化
```c
void ADC_Init(ADC_TypeDef* ADCx, ADC_InitTypeDef* ADC_InitStruct);
void ADC_Cmd(ADC_TypeDef* ADCx, FunctionalState NewState);
void ADC_DMACmd(ADC_TypeDef* ADCx, FunctionalState NewState);
void ADC_ResetCalibration(ADC_TypeDef* ADCx);
void ADC_StartCalibration(ADC_TypeDef* ADCx);
```

### ADC_InitTypeDef 结构体
```c
typedef struct {
    uint32_t ADC_Mode;        // 模式
    uint8_t ADC_ScanConvMode;      // 扫描模式
    uint8_t ADC_ContinuousConvMode;  // 连续转换
    uint8_t ADC_ExternalTrigConv;   // 外部触发
    uint32_t ADC_DataAlign;         // 数据对齐
    uint8_t ADC_NbrOfChannel;        // 通道数
} ADC_InitTypeDef;
```

### 参数常量
```c
// 模式
ADC_Mode_Independent             // 独立模式
ADC_Mode_RegSimult_LeftInj      // 规则同步左注入
ADC_Mode_RegSimult_RightInj     // 规则同步右注入
ADC_Mode_InterSimult_LeftInj    // 交叉同步左注入
ADC_Mode_InterSimult_RightInj   // 交叉同步右注入
ADC_Mode_RegSimult              // 规则同步
ADC_Mode_InterSimult            // 交叉同步
ADC_Mode_FastInterSimult        // 快速交叉同步
ADC_Mode_InjecSimult            // 注入同步

// 数据对齐
ADC_DataAlign_Right             // 右对齐
ADC_DataAlign_Left              // 左对齐

// 外部触发
ADC_ExternalTrigConv_None       // 软件触发
ADC_ExternalTrigConv_T1_CC1     // TIM1 CH1
// ...其他定时器触发源

// 通道数
1 ~ 16
```

### 通道配置
```c
void ADC_ChannelConfig(ADC_TypeDef* ADCx, uint8_t ADC_Channel, uint8_t ADC_Rank);

// 通道常量
ADC_Channel_0  ~ ADC_Channel_17
ADC_Channel_TempSensor   // 温度传感器
ADC_Channel_Vrefint      // 内部参考电压

// 采样时间
ADC_SampleTime_1Cycles5    // 1.5周期
ADC_SampleTime_7Cycles5    // 7.5周期
ADC_SampleTime_13Cycles5   // 13.5周期
ADC_SampleTime_28Cycles5   // 28.5周期
ADC_SampleTime_41Cycles5   // 41.5周期
ADC_SampleTime_55Cycles5   // 55.5周期
ADC_SampleTime_71Cycles5   // 71.5周期
ADC_SampleTime_239Cycles5  // 239.5周期
```

### 数据读取
```c
uint16_t ADC_GetConversionValue(ADC_TypeDef* ADCx);   // 读取转换结果
uint32_t ADC_GetDualConversionValue(void);             // 双ADC读取
FlagStatus ADC_GetFlagStatus(ADC_TypeDef* ADCx, uint8_t ADC_FLAG);
void ADC_ClearFlag(ADC_TypeDef* ADCx, uint8_t ADC_FLAG);
```

### 常用流程
```c
// 单次转换
ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);
ADC_SoftwareStartConvCmd(ADC1, ENABLE);
while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
uint16_t value = ADC_GetConversionValue(ADC1);
float voltage = value * 3.3f / 4096;

// 连续扫描
ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);
ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_239Cycles5);
ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 3, ADC_SampleTime_239Cycles5);
// 使用DMA传输...
```

---

## 八、SPI API

### 初始化
```c
void SPI_I2S_Init(SPI_TypeDef* SPIx, SPI_I2S_InitTypeDef* SPI_I2S_InitStruct);
void SPI_Cmd(SPI_TypeDef* SPIx, FunctionalState NewState);
```

### SPI_I2S_InitTypeDef 结构体
```c
typedef struct {
    uint16_t SPI_Direction;      // 方向
    uint16_t SPI_Mode;           // 模式
    uint16_t SPI_DataSize;       // 数据大小
    uint16_t SPI_CPOL;           // 时钟极性
    uint16_t SPI_CPHA;           // 时钟相位
    uint16_t SPI_NSS;            // NSS引脚
    uint16_t SPI_BaudRatePrescaler;  // 波特率预分频
    uint16_t SPI_FirstBit;       // 首位
} SPI_I2S_InitTypeDef;
```

### 参数常量
```c
// 方向
SPI_Direction_2Lines_FullDuplex   // 双线全双工
SPI_Direction_2Lines_RxOnly       // 双线只接收
SPI_Direction_1Line_Rx             // 单线接收
SPI_Direction_1Line_Tx            // 单线发送

// 模式
SPI_Mode_Master   // 主机
SPI_Mode_Slave    // 从机

// 数据大小
SPI_DataSize_8b   // 8位
SPI_DataSize_16b  // 16位

// 时钟极性
SPI_CPOL_Low    // 空闲低
SPI_CPOL_High   // 空闲高

// 时钟相位
SPI_CPHA_1Edge   // 第1个边沿
SPI_CPHA_2Edge   // 第2个边沿

// NSS
SPI_NSS_Soft     // 软件控制
SPI_NSS_Hard     // 硬件控制

// 首位
SPI_FirstBit_MSB   // MSB先
SPI_FirstBit_LSB   // LSB先
```

### 数据收发
```c
void SPI_I2S_SendData(SPI_TypeDef* SPIx, uint16_t Data);
uint16_t SPI_I2S_ReceiveData(SPI_TypeDef* SPIx);
```

---

## 九、I2C API

### 初始化
```c
void I2C_Init(I2C_TypeDef* I2Cx, I2C_InitTypeDef* I2C_InitStruct);
void I2C_Cmd(I2C_TypeDef* I2Cx, FunctionalState NewState);
```

### I2C_InitTypeDef 结构体
```c
typedef struct {
    uint32_t I2C_ClockSpeed;         // 时钟速度
    uint16_t I2C_Mode;               // 模式
    uint16_t I2C_DutyCycle;          // 占空比
    uint16_t I2C_OwnAddress1;        // 自身地址
    uint16_t I2C_Ack;                // 应答
    uint16_t I2C_AcknowledgedAddress; // 应答地址
} I2C_InitTypeDef;
```

### 参数常量
```c
// 模式
I2C_Mode_I2C        // I2C模式
I2C_Mode_SMBusDevice  // SMBus设备
I2C_Mode_SMBusHost    // SMBus主机

// 占空比
I2C_DutyCycle_2    // 1:1
I2C_DutyCycle_16_9 // 16:9 (快速模式)

// 应答
I2C_Ack_Enable     // 使能
I2C_Ack_Disable    // 禁用

// 应答地址
I2C_AcknowledgedAddress_7bit   // 7位
I2C_AcknowledgedAddress_10bit  // 10位
```

### 数据收发
```c
void I2C_SendData(I2C_TypeDef* I2Cx, uint8_t Data);
uint8_t I2C_ReceiveData(I2C_TypeDef* I2Cx);

// 主模式发送
I2C_GenerateSTART(I2C1, ENABLE);                    // 起始
while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));  // 等待START
I2C_Send7bitAddress(I2C1, addr, I2C_Direction_Transmitter);  // 发送地址
while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
I2C_SendData(I2C1, data);                           // 发送数据
while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
I2C_GenerateSTOP(I2C1, ENABLE);                     // 停止
```

---

## 十、Flash API

### 操作
```c
void FLASH_Unlock(void);
void FLASH_Lock(void);
void FLASH_ClearFlag(uint32_t FLASH_FLAG);
void FLASH_PageErase(uint32_t PageAddress);
void FLASH_ClearITPendingBit(uint32_t FLASH_IT);

// 编程
FLASH_Status FLASH_ProgramWord(uint32_t Address, uint32_t Data);
FLASH_Status FLASH_ProgramHalfWord(uint32_t Address, uint16_t Data);
FLASH_Status FLASH_ProgramByte(uint32_t Address, uint8_t Data);
FLASH_Status FLASH_ProgramOptionByte(uint8_t FLASH_Sector, uint8_t FLASH_OB);

// 状态
FLASH_Status FLASH_GetStatus(void);
void FLASH_ClearFlag(uint32_t FLASH_FLAG);
```

### Flash 地址 (F103C8T6)
```
起始地址: 0x0800 0000
大小: 64KB (65536字节)
页大小: 1KB (1024字节)
页0: 0x0800 0000 ~ 0x0800 03FF
页1: 0x0800 0400 ~ 0x0800 07FF
...
页63: 0x0800 FC00 ~ 0x0800 FFFF

常用操作:
  读: 直接用指针读取
    uint32_t data = *(__IO uint32_t*)addr;

  写: FLASH_ProgramWord(addr, data);

  擦除页:
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP);
    FLASH_PageErase(addr);  // 传该页内任意地址
    while (FLASH_GetFlagStatus(FLASH_FLAG_BSY));
    FLASH_Lock();
```

---

## 十一、常用工具函数

### 延时函数
```c
// 简单延时 (不精确)
void Delay_ms(uint32_t ms) {
    uint32_t i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 8000; j++);
}

// 基于SysTick的精确延时
void Delay_us(uint32_t us) {
    uint32_t ticks = us * (SystemCoreClock / 1000000);
    SysTick->LOAD = ticks - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
    while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
    SysTick->CTRL = 0;
}

void Delay_ms(uint32_t ms) {
    for (uint32_t i = 0; i < ms; i++)
        Delay_us(1000);
}
```

### printf 重定向 (Keil)
```c
#include <stdio.h>
#pragma import(__use_no_semihosting)
struct __FILE { int handle; };
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f) {
    USART_SendData(USART1, (uint8_t)ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    return ch;
}

int fgetc(FILE *f) {
    while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
    return USART_ReceiveData(USART1);
}
```

### printf 重定向 (GCC)
```c
#include <stdio.h>

int __io_putchar(int ch) {
    USART_SendData(USART1, (uint8_t)ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    return ch;
}

int _write(int file, char *ptr, int len) {
    int i;
    for (i = 0; i < len; i++)
        __io_putchar(*ptr++);
    return len;
}
```

---

## 十二、标准库头文件

```c
#include "stm32f10x.h"               // 总头文件 (包含所有)
// 或单独包含:
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x_flash.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_misc.h"            // NVIC
#include "stm32f10x_dma.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_bkp.h"
```

---

## 十三、常见问题

| 问题 | 解决方案 |
|------|----------|
| `stm32f10x.h` 找不到 | 检查头文件路径，确保包含 `\Libraries\CMSIS\ST\STM32F10x\Include` |
| 编译报 `undefined reference` | 检查是否在工程中添加了对应的 `.c` 文件 |
| GPIO 不工作 | 检查时钟是否使能、模式是否正确 |
| USART 收不到数据 | 检查波特率、GPIO复用、USART_Cmd是否ENABLE |
| 中断不触发 | 检查NVIC优先级、EXTI配置、中断函数名是否正确 |
| TIM PWM 无输出 | 检查TIM_Cmd、OC配置、GPIO复用模式 |
| ADC 值不准 | 增加采样时间、校准ADC |
| printf 无输出 | 检查重定向函数、USART配置 |
| FreeRTOS 下延时不准 | 使用 vTaskDelay 或 OS 延时 |