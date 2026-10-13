/**
 * ============================================================================
 * STM32F407VET6 标准外设库 (StdPeriph) 常用外设示例
 * ============================================================================
 * 本文件包含以下示例：
 *   1. GPIO 输入输出 (LED + 按键)
 *   2. EXTI 外部中断 (按键中断)
 *   3. USART 串口通信 (重定向printf)
 *   4. TIM 定时器 (PWM输出 + 定时中断)
 *   5. ADC 模数转换
 *   6. SPI 通信
 *   7. I2C 通信
 *   8. Flash 读写
 *   9. DMA 传输
 *
 * F407VET6硬件资源:
 *   - Flash: 512KB (0x08000000 - 0x0807FFFF)
 *   - SRAM: 192KB (0x20000000 - 0x2002FFFF)
 *   - 常用LED引脚: PF9 (绿色), PF10 (橙色)
 *   - 常用按键引脚: PA0 (WKUP)
 *   - 常用串口: USART3 (PD8-TX, PD9-RX)
 *   - TIM2时钟: 84MHz, TIM3时钟: 84MHz
 *   - ADC时钟: 21MHz (84MHz/4)
 *
 * 使用方法：将需要的示例代码复制到你的工程对应位置
 * ============================================================================
 */

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_i2c.h"
#include "stm32f4xx_flash.h"
#include "stm32f4xx_misc.h"
#include "stm32f4xx_syscfg.h"

#include <stdio.h>
#include <string.h>

/* ============================================================================
 * 第一部分：GPIO 示例
 * ============================================================================
 */

/*
 * 1.1 GPIO 输出 - LED初始化与闪烁
 * 硬件连接: PF9 -> LED (低电平点亮)
 */
void GPIO_LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    // 配置PF9为推挽输出
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;      // F4新增: 推挽
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;   // F4新增: 无上下拉
    GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_SetBits(GPIOF, GPIO_Pin_9 | GPIO_Pin_10);  // 默认灭
}

void GPIO_LED_Blink(void)
{
    // 方式1: 置位/复位
    GPIO_ResetBits(GPIOF, GPIO_Pin_9);  // LED亮
    Delay_ms(500);
    GPIO_SetBits(GPIOF, GPIO_Pin_9);    // LED灭
    Delay_ms(500);

    // 方式2: 翻转 (F4新增)
    GPIO_ToggleBits(GPIOF, GPIO_Pin_9);
    Delay_ms(500);

    // 方式3: 寄存器操作
    // GPIOF->ODR ^= GPIO_Pin_9;
}

/*
 * 1.2 GPIO 输入 - 读取按键
 * 硬件连接: PA0 -> 按键 (另一端接地，内部上拉)
 */
void GPIO_Key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // 配置PA0为上拉输入
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;  // F4新增: 上拉
    GPIO_Init(GPIOA, &GPIO_InitStruct);
}

uint8_t GPIO_Key_Read(void)
{
    // 按键未按下: 高电平 (Bit_SET = 1)
    // 按键按下:   低电平 (Bit_RESET = 0)
    return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
}

/*
 * 1.3 GPIO 配置多引脚
 */
void GPIO_Multi_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOF, ENABLE);

    // PA0: 输入 (按键)
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // PF9, PF10: 推挽输出 (LED)
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOF, &GPIO_InitStruct);

    // PD8, PD9: 复用推挽 (USART3)
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOD, &GPIO_InitStruct);
    // F4必须配置AF
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);
}


/* ============================================================================
 * 第二部分：EXTI 外部中断示例
 * ============================================================================
 */

/*
 * 2.1 EXTI 初始化
 * 硬件连接: PA0 -> 按键 (下降沿触发)
 *
 * 注意: F4中需要使能SYSCFG时钟 (不是AFIO)
 */
void EXTI_Key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    // 1. 使能时钟 (F4使用SYSCFG，不是AFIO)
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    // 2. 配置GPIO为输入
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 3. 配置EXTI
    // F4中EXTI配置方式:
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

    EXTI_InitStruct.EXTI_Line = EXTI_Line0;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    // 4. 配置NVIC
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/*
 * 2.2 EXTI 中断处理
 * 在 stm32f4xx_it.c 中调用:
 *   void EXTI0_IRQHandler(void) { Exti_Key_ISR(); }
 */
void Exti_Key_ISR(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
        // 消抖
        Delay_ms(20);
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET) {
            // 确认按键按下
            GPIO_ToggleBits(GPIOF, GPIO_Pin_9);
        }
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}


/* ============================================================================
 * 第三部分：USART 串口通信示例
 * ============================================================================
 */

/*
 * 3.1 USART3 初始化
 * 硬件连接: PD8(TX), PD9(RX)
 * 时钟: APB1 = 42MHz, USART3 = 42MHz
 */
void USART3_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    // 1. 使能时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    // 2. 配置GPIO为AF复用
    // TX (PD8): 复用推挽
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &GPIO_InitStruct);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);

    // RX (PD9): 复用推挽
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
    GPIO_Init(GPIOD, &GPIO_InitStruct);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);

    // 3. 配置USART
    USART_InitStruct.USART_BaudRate = 115200;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Over = USART_Over_16;  // F4新增
    USART_Init(USART3, &USART_InitStruct);

    // 4. 使能USART
    USART_Cmd(USART3, ENABLE);

    // 5. 使能接收中断
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

    // 6. 配置NVIC
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/*
 * 3.2 发送数据
 */
void USART3_SendByte(uint8_t data)
{
    USART_SendData(USART3, data);
    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
}

void USART3_SendString(char *str)
{
    while (*str != '\0') {
        USART3_SendByte(*str);
        str++;
    }
}

void USART3_SendData_Buf(uint8_t *buf, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        USART_SendData(USART3, buf[i]);
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
    }
}

/*
 * 3.3 接收数据
 */
uint8_t USART3_ReceiveByte(void)
{
    while (USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == RESET);
    return USART_ReceiveData(USART3);
}

/*
 * 3.4 printf 重定向 (GCC/Clang)
 */
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

int fgetc(FILE *f)
{
    while (USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == RESET);
    return USART_ReceiveData(USART3);
}
#endif

/*
 * 3.5 USART3 中断处理
 * 在 stm32f4xx_it.c 中调用:
 *   void USART3_IRQHandler(void) { USART3_ISR(); }
 */
volatile uint8_t usart3_rx_data = 0;
volatile uint8_t usart3_rx_flag = 0;

void USART3_ISR(void)
{
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) {
        usart3_rx_data = USART_ReceiveData(USART3);
        usart3_rx_flag = 1;
    }
}

void USART3_Process_Command(void)
{
    if (usart3_rx_flag) {
        usart3_rx_flag = 0;
        switch (usart3_rx_data) {
            case '1':
                GPIO_ResetBits(GPIOF, GPIO_Pin_9);
                USART3_SendString("LED ON\r\n");
                break;
            case '0':
                GPIO_SetBits(GPIOF, GPIO_Pin_9);
                USART3_SendString("LED OFF\r\n");
                break;
            case 't':
                GPIO_ToggleBits(GPIOF, GPIO_Pin_9);
                USART3_SendString("LED TOGGLE\r\n");
                break;
            default:
                USART3_SendString("Unknown cmd\r\n");
                break;
        }
    }
}


/* ============================================================================
 * 第四部分：TIM 定时器示例
 * ============================================================================
 */

/*
 * 4.1 基本定时器 - 定时中断
 * 硬件时钟: 84MHz (APB1=42MHz, 定时器自动倍频)
 */
void TIM2_Int_Init(void)
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

    // 使能更新中断
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    // 配置NVIC
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    TIM_Cmd(TIM2, ENABLE);
}

/*
 * 4.2 TIM2 中断处理
 * 在 stm32f4xx_it.c 中调用:
 *   void TIM2_IRQHandler(void) { TIM2_ISR(); }
 */
volatile uint8_t tim2_flag = 0;
volatile uint32_t tim2_count = 0;

void TIM2_ISR(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        tim2_flag = 1;
        tim2_count++;
    }
}

/*
 * 4.3 PWM 输出示例
 * 硬件连接: PA6 (TIM3_CH1)
 * PWM频率: 1kHz, 占空比: 50%
 * 时钟: 84MHz
 */
void TIM3_PWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    TIM_TimeBaseInitTypeDef TIM_InitStruct;
    TIM_OCInitTypeDef TIM_OCInitStruct;

    // 1. 使能时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // 2. 配置GPIO (PA6: TIM3_CH1, 复用推挽)
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM3);

    // 3. 配置定时器基础参数
    // PWM频率: 84MHz / (83+1) / (999+1) = 1kHz
    TIM_InitStruct.TIM_Period = 999;
    TIM_InitStruct.TIM_Prescaler = 83;
    TIM_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_InitStruct);

    // 4. 配置PWM输出
    // 占空比: 500 / 999 = 50%
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_Pulse = 500;
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStruct.TIM_OCNState = TIM_OCNState_Disable;
    TIM_OCInitStruct.TIM_OCNPolarity = TIM_OCNPolarity_High;
    TIM_OCInitStruct.TIM_OCIdleState = TIM_OCIdleState_Reset;
    TIM_OCInitStruct.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
    TIM_OC1Init(TIM3, &TIM_OCInitStruct);

    // 5. 使能预装载
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);

    // 6. 启动定时器
    TIM_Cmd(TIM3, ENABLE);
}

/*
 * 4.4 PWM 占空比设置
 */
void TIM3_PWM_SetDuty(uint16_t duty)
{
    // duty: 0~100 (%)
    uint16_t compare = (uint16_t)((TIM3->ARR + 1) * duty / 100);
    TIM_SetCompare1(TIM3, compare);
}

/*
 * 4.5 呼吸灯效果
 */
void TIM3_PWM_Breathing(void)
{
    uint16_t i;
    // 渐亮
    for (i = 0; i <= 100; i++) {
        TIM3_PWM_SetDuty(i);
        Delay_ms(20);
    }
    // 渐暗
    for (i = 100; i > 0; i--) {
        TIM3_PWM_SetDuty(i);
        Delay_ms(20);
    }
}


/* ============================================================================
 * 第五部分：ADC 模数转换示例
 * ============================================================================
 */

/*
 * 5.1 ADC 单次转换
 * 硬件连接: PA0 (ADC1_IN0)
 * 时钟: APB2=84MHz, ADC时钟=84/4=21MHz
 */
void ADC1_Init_Single(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    ADC_InitTypeDef ADC_InitStruct;

    // 1. 使能时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB2PeriphClkConfig(RCC_APB2Periph_ADCCLK, RCC_PCLK2_Div4);

    // 2. 配置GPIO为模拟输入
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 3. 配置ADC
    ADC_InitStruct.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStruct.ADC_ScanConvMode = DISABLE;
    ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStruct.ADC_ChannelCount = 1;  // F4新增
    ADC_Init(ADC1, &ADC_InitStruct);

    // 4. 配置通道
    // F4使用: ADC_RegularChannelConfig
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_84Cycles5);

    // 5. 使能ADC
    ADC_Cmd(ADC1, ENABLE);

    // 6. 校准
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1) == SET);
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1) == SET);
}

uint16_t ADC1_Read_Value(void)
{
    ADC_SoftwareStartConv(ADC1);
    while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
    return ADC_GetConversionValue(ADC1);
}

float ADC1_Read_Voltage(void)
{
    uint16_t value = ADC1_Read_Value();
    return (float)value * 3.3f / 4096;
}

/*
 * 5.2 ADC 多通道扫描
 */
void ADC1_Init_Scan(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    ADC_InitTypeDef ADC_InitStruct;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB2PeriphClkConfig(RCC_APB2Periph_ADCCLK, RCC_PCLK2_Div4);

    // 配置多个模拟输入引脚
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 扫描模式
    ADC_InitStruct.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStruct.ADC_ScanConvMode = ENABLE;
    ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStruct.ADC_ChannelCount = 3;  // F4: 3个通道
    ADC_Init(ADC1, &ADC_InitStruct);

    // 配置3个通道
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_84Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_84Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 3, ADC_SampleTime_84Cycles5);

    ADC_Cmd(ADC1, ENABLE);
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1) == SET);
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1) == SET);
}

void ADC1_Read_MultiChannel(uint16_t *values)
{
    uint8_t i;
    ADC_SoftwareStartConv(ADC1);
    for (i = 0; i < 3; i++) {
        while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
        values[i] = ADC_GetConversionValue(ADC1);
    }
}


/* ============================================================================
 * 第六部分：SPI 通信示例
 * ============================================================================
 */

/*
 * 6.1 SPI1 初始化 (TFT/Flash)
 * 硬件连接: PA5(SCK), PA6(MISO), PA7(MOSI)
 * 时钟: APB2=84MHz, SPI1=84MHz
 */
void SPI1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    SPI_I2S_InitTypeDef SPI_InitStruct;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    // SCK (PA5), MOSI (PA7): 复用推挽
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);

    // MISO (PA6): 复用推挽 (输入用推挽即可)
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);

    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStruct.SPI_CRCPolynomial = 7;  // F4新增
    SPI_I2S_Init(SPI1, &SPI_InitStruct);

    SPI_Cmd(SPI1, ENABLE);
}

uint8_t SPI1_Transfer(uint8_t data)
{
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI1, data);
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    return SPI_I2S_ReceiveData(SPI1);
}

void SPI1_Transfer_Buf(uint8_t *tx_buf, uint8_t *rx_buf, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        rx_buf[i] = SPI1_Transfer(tx_buf[i]);
    }
}


/* ============================================================================
 * 第七部分：I2C 通信示例
 * ============================================================================
 */

/*
 * 7.1 I2C1 初始化 (OLED/Sensor)
 * 硬件连接: PB6(SCL), PB7(SDA)
 * 时钟: APB1=42MHz, I2C1=42MHz
 */
void I2C1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    I2C_InitTypeDef I2C_InitStruct;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    // SCL (PB6), SDA (PB7): 复用开漏
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1);

    I2C_InitStruct.I2C_ClockSpeed = 400000;
    I2C_InitStruct.I2C_Timing = 0x40912735;  // F4: CubeMX计算的400kHz时序
    I2C_InitStruct.I2C_OwnAddress1 = 0x00;
    I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStruct.I2C_DualAddressCmd = I2C_DualAddressCmd_Disable;
    I2C_InitStruct.I2C_GeneralCallCmd = I2C_GeneralCallCmd_Disable;
    I2C_InitStruct.I2C_NoStretchCmd = I2C_NoStretchCmd_Disable;
    I2C_Init(I2C1, &I2C_InitStruct);

    I2C_Cmd(I2C1, ENABLE);
}

/*
 * 7.2 I2C 主模式写数据
 */
uint8_t I2C1_Write(uint8_t slave_addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    uint32_t timeout = 0;

    // 等待总线空闲
    timeout = 0xFFFF;
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) != RESET) {
        if (--timeout == 0) return 1;
    }

    // 发送START
    I2C_GenerateSTART(I2C1, ENABLE);
    timeout = 0xFFFF;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) {
        if (--timeout == 0) return 2;
    }

    // 发送从机地址 (写)
    I2C_Send7bitAddress(I2C1, slave_addr, I2C_Direction_Transmitter);
    timeout = 0xFFFF;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
        if (--timeout == 0) return 3;
    }

    // 发送寄存器地址
    I2C_SendData(I2C1, reg);
    timeout = 0xFFFF;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
        if (--timeout == 0) return 4;
    }

    // 发送数据
    for (uint16_t i = 0; i < len; i++) {
        I2C_SendData(I2C1, data[i]);
        timeout = 0xFFFF;
        while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
            if (--timeout == 0) return 5;
        }
    }

    // 发送STOP
    I2C_GenerateSTOP(I2C1, ENABLE);
    return 0;
}

/*
 * 7.3 I2C 主模式读数据
 */
uint8_t I2C1_Read(uint8_t slave_addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    uint32_t timeout = 0;

    timeout = 0xFFFF;
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) != RESET) {
        if (--timeout == 0) return 1;
    }

    I2C_GenerateSTART(I2C1, ENABLE);
    timeout = 0xFFFF;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) {
        if (--timeout == 0) return 2;
    }

    // 写寄存器地址
    I2C_Send7bitAddress(I2C1, slave_addr, I2C_Direction_Transmitter);
    timeout = 0xFFFF;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
        if (--timeout == 0) return 3;
    }
    I2C_SendData(I2C1, reg);
    timeout = 0xFFFF;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
        if (--timeout == 0) return 4;
    }

    // 重新START
    I2C_GenerateSTART(I2C1, ENABLE);
    timeout = 0xFFFF;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)) {
        if (--timeout == 0) return 5;
    }

    // 读数据
    I2C_Send7bitAddress(I2C1, slave_addr, I2C_Direction_Receiver);
    timeout = 0xFFFF;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) {
        if (--timeout == 0) return 6;
    }

    for (uint16_t i = 0; i < len; i++) {
        if (i == len - 1) {
            I2C_AcknowledgeConfig(I2C1, DISABLE);
            I2C_GenerateSTOP(I2C1, ENABLE);
        }
        timeout = 0xFFFF;
        while (I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE) == RESET) {
            if (--timeout == 0) return 7;
        }
        data[i] = I2C_ReceiveData(I2C1);
    }

    I2C_AcknowledgeConfig(I2C1, ENABLE);
    return 0;
}


/* ============================================================================
 * 第八部分：Flash 读写示例
 * ============================================================================
 */

// F407VET6 Flash结构:
// Sector 0-3: 16KB each (0x08000000 - 0x0800FFFF)
// Sector 4: 64KB (0x08010000 - 0x0801FFFF)
// Sector 5-7: 128KB each
#define FLASH_USER_ADDR  0x08060000  // Sector 6, 128KB

void Flash_WriteData(uint32_t addr, uint32_t *data, uint16_t len)
{
    uint16_t i;

    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP);

    // F4使用扇区擦除 (不是页擦除)
    FLASH_EraseSector(FLASH_Sector_6, FLASH_VOLTAGE_RANGE_3);
    while (FLASH_GetStatus() == FLASH_BUSY);

    // 写入数据 (按双字写)
    for (i = 0; i < len; i++) {
        FLASH_ProgramWord(addr + i * 4, data[i]);
        while (FLASH_GetStatus() == FLASH_BUSY);
    }

    FLASH_Lock();
}

void Flash_ReadData(uint32_t addr, uint32_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        data[i] = *(__IO uint32_t*)(addr + i * 4));
    }
}


/* ============================================================================
 * 第九部分：工具函数
 * ============================================================================
 */

/*
 * 精确延时 (SysTick)
 */
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

/*
 * 软件定时器
 */
typedef struct {
    uint32_t start_tick;
    uint32_t period;
    uint8_t active;
} SoftTimer_t;

static SoftTimer_t led_timer;

void SoftTimer_Start(SoftTimer_t *timer, uint32_t period_ms)
{
    timer->start_tick = 0;  // 使用简单计数
    timer->period = period_ms / 10;  // 假设10ms调用一次
    timer->active = 1;
}

uint8_t SoftTimer_Check(SoftTimer_t *timer)
{
    if (!timer->active) return 0;
    timer->start_tick++;
    if (timer->start_tick >= timer->period) {
        timer->active = 0;
        return 1;
    }
    return 0;
}

/*
 * 按键扫描 (轮询方式)
 */
uint8_t Key_Scan(void)
{
    static uint8_t key_state = 1;
    static uint8_t key_cnt = 0;
    uint8_t cur_state = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);

    if (cur_state != key_state) {
        key_cnt++;
        if (key_cnt >= 3) {  // 30ms去抖 (3 x 10ms)
            key_state = cur_state;
            key_cnt = 0;
            if (key_state == 0) return 1;  // 按下
        }
    } else {
        key_cnt = 0;
    }
    return 0;
}
