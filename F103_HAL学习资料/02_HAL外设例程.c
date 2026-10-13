/**
 * ============================================================================
 * STM32F103C8T6 HAL库 常用外设示例
 * ============================================================================
 * 本文件包含以下示例：
 *   1. GPIO 输入输出 (LED + 按键)
 *   2. EXTI 外部中断 (按键中断)
 *   3. USART 串口通信 (重定向printf)
 *   4. TIM 定时器 (PWM输出 + 定时中断)
 *   5. ADC 模数转换
 *
 * 使用方法：将需要的示例代码复制到你的 CubeMX 工程对应位置
 *   - 初始化代码放入 main.c 的 USER CODE BEGIN 2 区域
 *   - 回调函数放入 main.c 或单独的 .c 文件
 *   - 中断处理放入 stm32f1xx_it.c
 * ============================================================================
 */

#include "main.h"
#include "gpio.h"

/* ============================================================================
 * 第一部分：GPIO 示例
 * ============================================================================
 */

/*
 * 1.1 GPIO 输出 - LED闪烁
 * 标准库: GPIO_SetBits() / GPIO_ResetBits()
 * HAL库: HAL_GPIO_WritePin()
 *
 * 硬件连接: PC13 -> LED (低电平点亮)
 */
void GPIO_LED_Blink(void)
{
    /* USER CODE BEGIN GPIO_LED_Blink */

    // 方式1: 置位/复位
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);  // LED亮
    HAL_Delay(500);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);    // LED灭
    HAL_Delay(500);

    // 方式2: 翻转 (相当于 toggle)
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    HAL_Delay(500);

    /* USER CODE END GPIO_LED_Blink */
}

/*
 * 1.2 GPIO 输入 - 读取按键状态
 * 标准库: GPIO_ReadInputDataBit()
 * HAL库: HAL_GPIO_ReadPin()
 *
 * 硬件连接: PA0 -> 按键 (另一端接地，内部上拉)
 */
void GPIO_Button_Read(void)
{
    /* USER CODE BEGIN GPIO_Button_Read */

    // 读取PA0引脚电平
    // 按键未按下: 上拉为高电平 (GPIO_PIN_SET = 1)
    // 按键按下:   接地为低电平 (GPIO_PIN_RESET = 0)
    GPIO_PinState btn_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);

    if (btn_state == GPIO_PIN_RESET) {
        // 按键按下
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);  // LED亮
    } else {
        // 按键松开
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);    // LED灭
    }

    /* USER CODE END GPIO_Button_Read */
}

/*
 * 1.3 GPIO 初始化完整示例
 * 标准库: GPIO_Init()
 * HAL库: HAL_GPIO_Init()
 *
 * 配置多引脚: PA0(输入) + PC13(输出)
 */
void GPIO_Init_Example(void)
{
    /* USER CODE BEGIN GPIO_Init_Example */

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 1. 开启时钟 (CubeMX的MX_GPIO_Init中已自动完成)
    // __HAL_RCC_GPIOA_CLK_ENABLE();
    // __HAL_RCC_GPIOC_CLK_ENABLE();

    // 2. 配置 PA0 为输入 (按键)
    GPIO_InitStruct.Pin = GPIO_PIN_0;           // 引脚号
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;      // 输入模式
    GPIO_InitStruct.Pull = GPIO_PULLUP;          // 上拉 (按键接地)
    // GPIO_InitStruct.Pull = GPIO_PULLDOWN;     // 下拉 (按键接VCC)
    // GPIO_InitStruct.Pull = GPIO_NOPULL;       // 无上下拉
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 3. 配置 PC13 为推挽输出 (LED)
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;   // 推挽输出
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;  // 速度等级
    // 速度选项: GPIO_SPEED_FREQ_LOW / MEDIUM / HIGH / VERY_HIGH
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* USER CODE END GPIO_Init_Example */
}


/* ============================================================================
 * 第二部分：EXTI 外部中断示例
 * ============================================================================
 */

/*
 * 2.1 EXTI 初始化
 * 硬件连接: PA0 -> 按键 (下降沿触发)
 *
 * CubeMX配置步骤:
 *   GPIO -> 选择PA0 -> 模式设为 External Interrupt Mode with Falling edge trigger
 *          -> 内部上拉
 *   NVIC -> 勾选 EXTI line0 interrupt
 */
void EXTI_Init_Example(void)
{
    /* USER CODE BEGIN EXTI_Init_Example */

    // 此代码通常由CubeMX自动生成在 gpio.c 的 MX_GPIO_Init() 中
    // 手动配置示例:

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;  // 下降沿中断
    // 其他选项:
    //   GPIO_MODE_IT_RISING      - 上升沿中断
    //   GPIO_MODE_IT_RISING_FALLING - 双边沿中断
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 使能中断线
    HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);  // 优先级: 抢占2, 子优先级0
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);

    /* USER CODE END EXTI_Init_Example */
}

/*
 * 2.2 EXTI 中断回调函数
 * 这是最重要的函数！所有EXTI中断最终都会调用这个回调
 *
 * 在 stm32f1xx_it.c 中:
 *   void EXTI0_IRQHandler(void) {
 *       HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);  // 调用HAL库处理
 *   }
 *
 * 然后在这里实现你的业务逻辑:
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    /* USER CODE BEGIN HAL_GPIO_EXTI_Callback */

    if (GPIO_Pin == GPIO_PIN_0) {
        // PA0按键触发的中断
        // 消抖处理 (建议延时10-20ms再判断)
        HAL_Delay(20);
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) {
            // 确认按键按下
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);  // 翻转LED
        }
    }
    // 可以继续添加其他引脚的中断处理:
    // else if (GPIO_Pin == GPIO_PIN_1) { ... }

    /* USER CODE END HAL_GPIO_EXTI_Callback */
}


/* ============================================================================
 * 第三部分：USART 串口通信示例
 * ============================================================================
 */

/*
 * 3.1 串口初始化
 * CubeMX配置:
 *   Connectivity -> USART1 -> Mode: Asynchronous
 *   Parameter Settings: 115200 baud, 8-bit, No parity, 1 stop bit
 *   NVIC -> 勾选 USART1 global interrupt
 */

// 外部声明 (CubeMX会在main.c中生成)
extern UART_HandleTypeDef huart1;

/*
 * 3.2 发送数据
 */
void USART_Send_Example(void)
{
    /* USER CODE BEGIN USART_Send_Example */

    // 方式1: 阻塞发送 (简单常用)
    char msg[] = "Hello HAL UART!\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 1000);
    // 参数: 句柄, 数据指针, 长度, 超时ms

    // 方式2: 中断发送
    HAL_UART_Transmit_IT(&huart1, (uint8_t*)msg, strlen(msg));

    // 方式3: DMA发送 (需在CubeMX中启用DMA)
    // HAL_UART_Transmit_DMA(&huart1, (uint8_t*)msg, strlen(msg));

    /* USER CODE END USART_Send_Example */
}

/*
 * 3.3 接收数据
 */
void USART_Receive_Example(void)
{
    /* USER CODE BEGIN USART_Receive_Example */

    uint8_t rx_data = 0;

    // 方式1: 阻塞接收 (不推荐在主循环中使用)
    if (HAL_UART_Receive(&huart1, &rx_data, 1, 100) == HAL_OK) {
        // 收到一个字节
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    }

    // 方式2: 中断接收 (推荐)
    HAL_UART_Receive_IT(&huart1, &rx_data, 1);

    // 方式3: DMA接收
    // HAL_UART_Receive_DMA(&huart1, rx_buf, 64);

    /* USER CODE END USART_Receive_Example */
}

/*
 * 3.4 重定向 printf 到串口 (GCC/Clang)
 * 在 main.c 或单独的文件中添加:
 */
#ifdef __GNUC__
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, 100);
    return ch;
}
#else
// Keil MDK: 重定向 fputc
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, 100);
    return ch;
}
#endif

/*
 * 3.5 串口接收回调
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    /* USER CODE BEGIN HAL_UART_RxCpltCallback */

    if (huart->Instance == USART1) {
        // USART1 接收完成
        // 处理接收到的数据...

        // 继续接收下一个字节
        // HAL_UART_Receive_IT(&huart1, &rx_data, 1);
    }

    /* USER CODE END HAL_UART_RxCpltCallback */
}


/* ============================================================================
 * 第四部分：TIM 定时器示例
 * ============================================================================
 */

extern TIM_HandleTypeDef htim2;  // CubeMX生成

/*
 * 4.1 基本定时器 - 定时中断
 * CubeMX配置:
 *   Timers -> TIM2 -> Clock Source: Internal Clock
 *   Parameter Settings:
 *     Prescaler: 71  (72MHz/(71+1) = 1MHz = 1us)
 *     Counter Period: 9999  ((9999+1) * 1us = 10ms)
 *     NVIC -> 勾选 TIM2 global interrupt
 */
void TIM_Basic_Start_Example(void)
{
    /* USER CODE BEGIN TIM_Basic_Start_Example */

    HAL_TIM_Base_Start_IT(&htim2);  // 启动定时器中断

    /* USER CODE END TIM_Basic_Start_Example */
}

/*
 * 4.2 定时器中断回调
 * 在 stm32f1xx_it.c 中:
 *   void TIM2_IRQHandler(void) {
 *       HAL_TIM_IRQHandler(&htim2);
 *   }
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    /* USER CODE BEGIN HAL_TIM_PeriodElapsedCallback */

    if (htim->Instance == TIM2) {
        // TIM2溢出 (每10ms触发一次)
        // 适合处理: 扫描按键、刷新LED、喂狗等
    }

    /* USER CODE END HAL_TIM_PeriodElapsedCallback */
}

/*
 * 4.3 PWM 输出示例
 * CubeMX配置:
 *   Timers -> TIM3 -> Channels: CH1 -> PWM Generation CH1
 *   Prescaler: 71    (1MHz)
 *   Counter Period: 999  (1kHz PWM)
 *   Pin: PA6 (TIM3_CH1)
 */
void TIM_PWM_Example(void)
{
    /* USER CODE BEGIN TIM_PWM_Example */

    // 启动PWM
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

    // 设置占空比 (0 ~ Period值)
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 500);  // 50% 占空比

    // 呼吸灯效果示例
    // for (int i = 0; i < 1000; i += 10) {
    //     __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, i);
    //     HAL_Delay(5);
    // }
    // for (int i = 1000; i > 0; i -= 10) {
    //     __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, i);
    //     HAL_Delay(5);
    // }

    /* USER CODE END TIM_PWM_Example */
}


/* ============================================================================
 * 第五部分：ADC 模数转换示例
 * ============================================================================
 */

extern ADC_HandleTypeDef hadc1;  // CubeMX生成

/*
 * 5.1 ADC 单次转换
 * CubeMX配置:
 *   Analog -> ADC1 -> IN0 (PA0)
 *   Resolution: 12 bits
 *   Conversion mode: Single
 */
uint16_t ADC_Read_Channel(void)
{
    /* USER CODE BEGIN ADC_Read_Channel */

    uint16_t value = 0;

    HAL_ADC_Start(&hadc1);                          // 启动转换
    HAL_ADC_PollForConversion(&hadc1, 100);        // 等待完成
    value = HAL_ADC_GetValue(&hadc1);               // 读取结果

    // 转换为电压值 (12位ADC, 参考电压3.3V)
    // float voltage = value * 3.3f / 4096;

    return value;

    /* USER CODE END ADC_Read_Channel */
}

/*
 * 5.2 ADC 多通道扫描 + DMA (进阶)
 * CubeMX配置:
 *   ADC1 -> Scan Conversion Mode: Enabled
 *   Channel Configuration: 添加多个通道
 *   DMA Settings: 勾选 Circular
 */
void ADC_DMA_Example(void)
{
    /* USER CODE BEGIN ADC_DMA_Example */

    uint16_t adc_buf[4];  // 4个通道的结果

    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buf, 4);
    // 之后可以通过 adc_buf[0], adc_buf[1]... 读取各通道值

    /* USER CODE END ADC_DMA_Example */
}


/* ============================================================================
 * 第六部分：实用技巧
 * ============================================================================
 */

/*
 * 6.1 非阻塞延时 (使用SysTick)
 * HAL_Delay() 是阻塞的，在中断中不能使用
 * 可以用 DWT 计数器实现非阻塞延时
 */
void Delay_us(uint32_t us)
{
    /* USER CODE BEGIN Delay_us */
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < ticks);
    /* USER CODE END Delay_us */
}

// 在主函数中初始化DWT:
//   CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
//   DWT->CYCCNT = 0;
//   DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

/*
 * 6.2 读取系统运行时间 (ms)
 */
uint32_t GetTick_ms(void)
{
    return HAL_GetTick();
}

/*
 * 6.3 简单的软件定时器
 */
typedef struct {
    uint32_t start_tick;
    uint32_t period;
    uint8_t  active;
} SoftTimer_t;

void SoftTimer_Start(SoftTimer_t *timer, uint32_t period_ms)
{
    timer->start_tick = HAL_GetTick();
    timer->period = period_ms;
    timer->active = 1;
}

uint8_t SoftTimer_Check(SoftTimer_t *timer)
{
    if (!timer->active) return 0;
    if ((HAL_GetTick() - timer->start_tick) >= timer->period) {
        timer->active = 0;
        return 1;  // 定时到达
    }
    return 0;
}

/* 使用示例:
 *   SoftTimer_t led_timer;
 *   SoftTimer_Start(&led_timer, 500);  // 500ms
 *   while (1) {
 *       if (SoftTimer_Check(&led_timer)) {
 *           HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
 *           SoftTimer_Start(&led_timer, 500);
 *       }
 *   }
 */