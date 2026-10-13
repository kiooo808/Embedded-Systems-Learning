/**
 * ============================================================================
 * STM32F407VET6 标准外设库 (StdPeriph) 完整主程序模板
 * ============================================================================
 * 功能:
 *   - LED闪烁 (PF9绿色, PF10橙色)
 *   - 按键检测 (PA0, 轮询方式)
 *   - 串口打印 (USART3, 115200)
 *   - 定时器中断 (TIM2, 10ms)
 *   - 按键中断 (PA0, EXTI)
 *   - ADC采样 (PA0, 轮询方式)
 *   - PWM输出 (PA6, TIM3_CH1)
 *
 * F407VET6硬件资源:
 *   - 主频: 168MHz
 *   - Flash: 512KB
 *   - SRAM: 192KB (112KB主 + 64KB CCM + 16KB Backup)
 *   - LED: PF9 (绿色), PF10 (橙色)
 *   - 按键: PA0 (WKUP)
 *   - USART3: PD8(TX), PD9(RX)
 *   - TIM2: 84MHz
 *   - TIM3: 84MHz
 *   - ADC1: 21MHz
 *
 * 使用方法:
 *   1. 将本文件内容复制到 main.c
 *   2. 确保包含所有头文件
 *   3. 根据需要修改引脚和外设配置
 * ============================================================================
 */

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_misc.h"
#include "stm32f4xx_syscfg.h"

#include <stdio.h>
#include <string.h>

/* ======================== 宏定义 ======================== */
#define LED_GREEN_ON()    GPIO_ResetBits(GPIOF, GPIO_Pin_9)
#define LED_GREEN_OFF()   GPIO_SetBits(GPIOF, GPIO_Pin_9)
#define LED_GREEN_TOGGLE() GPIO_ToggleBits(GPIOF, GPIO_Pin_9)

#define LED_ORANGE_ON()   GPIO_ResetBits(GPIOF, GPIO_Pin_10)
#define LED_ORANGE_OFF()  GPIO_SetBits(GPIOF, GPIO_Pin_10)
#define LED_ORANGE_TOGGLE() GPIO_ToggleBits(GPIOF, GPIO_Pin_10)

#define KEY_READ()        GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)

/* ======================== 全局变量 ======================== */

// 串口接收
volatile uint8_t usart3_rx_data = 0;
volatile uint8_t usart3_rx_flag = 0;

// 定时器标志
volatile uint8_t tim2_flag = 0;
volatile uint32_t tim2_count = 0;

// 按键状态
volatile uint8_t key_pressed = 0;
volatile uint8_t key_long_press = 0;

// 系统状态
uint32_t system_uptime = 0;
uint16_t adc_value = 0;
float adc_voltage = 0.0f;

/* ======================== 函数声明 ======================== */
void SystemClock_Config(void);
void GPIO_Init(void);
void USART3_Init(void);
void TIM2_Init(void);
void TIM3_PWM_Init(void);
void ADC1_Init(void);
void EXTI_Init(void);
void App_Init(void);
void App_Loop(void);
void Key_Scan(void);
void Process_Command(void);
void ADC_Read(void);
void Delay_us(uint32_t us);
void Delay_ms(uint32_t ms);

/* ======================== 系统时钟配置 ======================== */
void SystemClock_Config(void)
{
    // 配置F407系统时钟为168MHz
    // HSE = 8MHz, PLL = 168MHz

    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    // 1. 使能HSE
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;   // VCO输入 = 8/8 = 1MHz
    RCC_OscInitStruct.PLL.PLLN = 336; // VCO输出 = 1 × 336 = 336MHz
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2; // SYSCLK = 336/2 = 168MHz
    RCC_OscInitStruct.PLL.PLLQ = 7;  // USB时钟 = 336/7 = 48MHz
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    // 2. 配置时钟
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                 | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;   // AHB = 168MHz
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;   // APB1 = 42MHz
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;   // APB2 = 84MHz
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

    // 3. 配置外设时钟
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART3
                                             | RCC_PERIPHCLK_ADC1;
    PeriphClkInitStruct.USART3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
    PeriphClkInitStruct.ADC1ClockSelection = RCC_ADCCLKSOURCE_PCLK2;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

    SystemCoreClock = 168000000;
}

/* ======================== GPIO初始化 ======================== */
void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // 1. 使能时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOF, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);  // F4: EXTI需要SYSCFG

    // 2. 配置LED引脚 (PF9, PF10)
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOF, &GPIO_InitStruct);

    // 默认关闭
    GPIO_SetBits(GPIOF, GPIO_Pin_9 | GPIO_Pin_10);

    // 3. 配置按键引脚 (PA0)
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 4. 配置USART3 GPIO (PD8, PD9)
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &GPIO_InitStruct);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);

    // 5. 配置TIM3 GPIO (PA6)
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM3);

    // 6. 配置ADC1 GPIO (PA0)
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/* ======================== USART3初始化 ======================== */
void USART3_Init(void)
{
    USART_InitTypeDef USART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    USART_InitStruct.USART_BaudRate = 115200;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Over = USART_Over_16;  // F4新增
    USART_Init(USART3, &USART_InitStruct);

    USART_Cmd(USART3, ENABLE);
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/* ======================== TIM2初始化 ======================== */
void TIM2_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // 定时10ms: 84MHz / (83+1) / (9999+1) = 10ms
    TIM_InitStruct.TIM_Period = 9999;
    TIM_InitStruct.TIM_Prescaler = 83;
    TIM_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_InitStruct);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    TIM_Cmd(TIM2, ENABLE);
}

/* ======================== TIM3 PWM初始化 ======================== */
void TIM3_PWM_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    TIM_OCInitTypeDef TIM_OCInitStruct;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    // PWM频率: 84MHz / (83+1) / (999+1) = 1kHz
    TIM_InitStruct.TIM_Period = 999;
    TIM_InitStruct.TIM_Prescaler = 83;
    TIM_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_InitStruct);

    // 50%占空比
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_Pulse = 500;
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStruct.TIM_OCNState = TIM_OCNState_Disable;
    TIM_OCInitStruct.TIM_OCNPolarity = TIM_OCNPolarity_High;
    TIM_OCInitStruct.TIM_OCIdleState = TIM_OCIdleState_Reset;
    TIM_OCInitStruct.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
    TIM_OC1Init(TIM3, &TIM_OCInitStruct);

    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}

/* ======================== ADC1初始化 ======================== */
void ADC1_Init(void)
{
    ADC_InitTypeDef ADC_InitStruct;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB2PeriphClkConfig(RCC_APB2Periph_ADCCLK, RCC_PCLK2_Div4);

    ADC_InitStruct.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStruct.ADC_ScanConvMode = DISABLE;
    ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStruct.ADC_ChannelCount = 1;
    ADC_Init(ADC1, &ADC_InitStruct);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_84Cycles5);
    ADC_Cmd(ADC1, ENABLE);

    // 校准
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1) == SET);
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1) == SET);
}

/* ======================== EXTI初始化 ======================== */
void EXTI_Init(void)
{
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    // PA0 -> EXTI0
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

    EXTI_InitStruct.EXTI_Line = EXTI_Line0;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/* ======================== 应用初始化 ======================== */
void App_Init(void)
{
    // 1. 初始化FPU (可选，用于浮点加速)
    #define FPU_INIT
    #ifdef FPU_INIT
    SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));
    #endif

    // 2. 启动定时器
    TIM2_Init();

    // 3. 启动PWM
    TIM3_PWM_Init();

    // 4. 启动串口接收
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

    // 5. 打印启动信息
    printf("\r\n");
    printf("============================================\r\n");
    printf("  STM32F407VET6 StdPeriph Template\r\n");
    printf("  System Clock: %lu MHz\r\n", SystemCoreClock / 1000000);
    printf("  Flash: 512KB, SRAM: 192KB\r\n");
    printf("============================================\r\n");
    printf("\r\n");
}

/* ======================== 主循环 ======================== */
void App_Loop(void)
{
    // 10ms 定时任务
    if (tim2_flag) {
        tim2_flag = 0;
        tim2_count++;
        system_uptime = tim2_count * 10;

        // 按键扫描
        Key_Scan();
    }

    // 100ms 任务
    if ((tim2_count % 10) == 0 && tim2_count > 0) {
        // LED状态指示 (每1秒)
        if ((tim2_count % 100) == 0) {
            LED_GREEN_TOGGLE();
        }

        // ADC采样
        ADC_Read();
    }

    // 1秒 任务
    if ((tim2_count % 100) == 0 && tim2_count > 0) {
        printf("Uptime: %lu s, ADC: %u (%.2fV)\r\n",
               system_uptime / 1000, adc_value, adc_voltage);
    }

    // 按键处理
    if (key_pressed) {
        key_pressed = 0;
        printf("Key pressed!\r\n");
        LED_ORANGE_TOGGLE();
    }

    // 串口命令处理
    if (usart3_rx_flag) {
        usart3_rx_flag = 0;
        Process_Command();
    }
}

/* ======================== 按键扫描 ======================== */
void Key_Scan(void)
{
    static uint8_t key_state = 1;
    static uint8_t key_cnt = 0;
    static uint8_t press_time = 0;

    uint8_t cur_state = KEY_READ();

    if (cur_state != key_state) {
        key_cnt++;
        if (key_cnt >= 3) {  // 30ms去抖
            key_state = cur_state;
            key_cnt = 0;

            if (key_state == 0) {
                press_time = 0;
            } else {
                if (press_time >= 20) {
                    key_long_press = 1;
                } else {
                    key_pressed = 1;
                }
            }
        }
    } else {
        key_cnt = 0;
    }

    if (key_state == 0) {
        press_time++;
    }
}

/* ======================== 串口命令处理 ======================== */
void Process_Command(void)
{
    uint8_t cmd = usart3_rx_data;

    switch (cmd) {
        case '1':
            LED_GREEN_ON();
            printf("Green LED ON\r\n");
            break;
        case '0':
            LED_GREEN_OFF();
            printf("Green LED OFF\r\n");
            break;
        case '3':
            LED_ORANGE_ON();
            printf("Orange LED ON\r\n");
            break;
        case '4':
            LED_ORANGE_OFF();
            printf("Orange LED OFF\r\n");
            break;
        case 't':
            LED_GREEN_TOGGLE();
            printf("Green LED TOGGLE\r\n");
            break;
        case 'b':
            for (int i = 0; i < 5; i++) {
                LED_GREEN_TOGGLE();
                Delay_ms(200);
            }
            printf("Blink done\r\n");
            break;
        case 's':
            printf("System Status:\r\n");
            printf("  Uptime: %lu ms\r\n", system_uptime);
            printf("  Counter: %lu\r\n", tim2_count);
            printf("  LED Green: %s\r\n",
                GPIO_ReadOutputDataBit(GPIOF, GPIO_Pin_9) == Bit_RESET ? "ON" : "OFF");
            printf("  Key: %s\r\n",
                KEY_READ() == Bit_RESET ? "PRESSED" : "RELEASED");
            break;
        case 'p':
            // PWM呼吸效果
            printf("PWM demo...\r\n");
            for (uint16_t i = 0; i <= 1000; i += 10) {
                TIM_SetCompare1(TIM3, i);
                Delay_ms(5);
            }
            for (uint16_t i = 1000; i > 0; i -= 10) {
                TIM_SetCompare1(TIM3, i);
                Delay_ms(5);
            }
            TIM_SetCompare1(TIM3, 0);
            printf("PWM done\r\n");
            break;
        case 'a':
            ADC_Read();
            printf("ADC: %u (%.2fV)\r\n", adc_value, adc_voltage);
            break;
        case '?':
            printf("Commands: 1=G_ON, 0=G_OFF, 3=O_ON, 4=O_OFF\r\n");
            printf("  t=Toggle, b=Blink, s=Status, p=PWM, a=ADC\r\n");
            break;
        default:
            printf("Unknown: %c\r\n", cmd);
            break;
    }
}

/* ======================== ADC读取 ======================== */
void ADC_Read(void)
{
    ADC_SoftwareStartConv(ADC1);
    if (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) != RESET) {
        adc_value = ADC_GetConversionValue(ADC1);
        adc_voltage = (float)adc_value * 3.3f / 4096.0f;
    }
}

/* ======================== 中断处理函数 ======================== */

// TIM2中断
void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        tim2_flag = 1;
    }
}

// USART3中断
void USART3_IRQHandler(void)
{
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) {
        usart3_rx_data = USART_ReceiveData(USART3);
        usart3_rx_flag = 1;
    }
}

// EXTI0中断 (按键)
void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
        // 简单消抖
        Delay_ms(20);
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET) {
            LED_ORANGE_TOGGLE();
        }
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

// SysTick
void SysTick_Handler(void)
{
    // 用于HAL_Delay等 (如果使用HAL)
}

/* ======================== 延时函数 ======================== */
void Delay_us(uint32_t us)
{
    uint32_t ticks = us * (SystemCoreClock / 1000000);
    SysTick->LOAD = ticks - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
    while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
    SysTick->CTRL = 0;
}

void Delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++) {
        Delay_us(1000);
    }
}

/* ======================== printf重定向 ======================== */

// GCC/Clang
#ifdef __GNUC__
int __io_putchar(int ch)
{
    USART_SendData(USART3, (uint8_t)ch);
    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
    return ch;
}

int _write(int file, char *ptr, int len)
{
    int i;
    for (i = 0; i < len; i++)
        __io_putchar(*ptr++);
    return len;
}
#else
// Keil MDK
struct __FILE { int handle; };
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f)
{
    USART_SendData(USART3, (uint8_t)ch);
    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
    return ch;
}
#endif

/* ======================== main函数 ======================== */
int main(void)
{
    // 1. 系统初始化
    SystemInit();
    SystemClock_Config();

    // 2. 初始化所有外设
    GPIO_Init();
    USART3_Init();
    TIM2_Init();
    TIM3_PWM_Init();
    ADC1_Init();
    EXTI_Init();

    // 3. 应用初始化
    App_Init();

    // 4. 主循环
    while (1) {
        App_Loop();
    }
}
