/**
 * ============================================================================
 * STM32F407VET6 HAL库 完整主程序模板
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
 *   - 按键: PA0 (WKUP), PC13
 *   - USART3: PD8(TX), PD9(RX)
 *   - TIM2: 基本定时器
 *   - TIM3: PWM输出 (PA6)
 *   - ADC1: 模拟输入 (PA0)
 *
 * 使用方法:
 *   1. 在CubeMX中使能: GPIO, USART3, TIM2, TIM3, ADC1, EXTI
 *   2. 将本文件中的代码复制到对应位置
 *   3. 注意: 本模板代码写在 USER CODE 区域外，仅供参考
 * ============================================================================
 */

/* ======================== main.c ======================== */

#include "main.h"
#include "gpio.h"
#include "usart.h"
#include "tim.h"
#include "adc.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/* ======================== 宏定义 ======================== */
#define LED_GREEN_ON()    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_RESET)
#define LED_GREEN_OFF()   HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_SET)
#define LED_GREEN_TOGGLE() HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_9)

#define LED_ORANGE_ON()   HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_RESET)
#define LED_ORANGE_OFF()  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_SET)
#define LED_ORANGE_TOGGLE() HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_10)

#define KEY_READ()        HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)

#define PWM_SET_DUTY(duty) __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, duty)

/* ======================== 全局变量 ======================== */

// 串口接收
#define RX_BUF_SIZE 64
uint8_t rx_buf[RX_BUF_SIZE];
volatile uint8_t rx_flag = 0;
volatile uint8_t rx_idx = 0;

// 定时器标志 (由中断设置)
volatile uint8_t timer_10ms_flag = 0;
volatile uint32_t timer_count = 0;

// 按键状态
volatile uint8_t key_pressed = 0;
volatile uint8_t key_long_press = 0;
volatile uint8_t key_double_click = 0;

// 系统状态
uint32_t system_uptime = 0;
uint16_t adc_value = 0;
float adc_voltage = 0.0f;

// FPU状态
uint8_t fpu_enabled = 0;

/* ======================== 函数声明 ======================== */
void SystemClock_Config(void);
void FPU_Init(void);
void App_Init(void);
void App_Loop(void);
void Key_Scan(void);
void Process_UART_Command(void);
void ADC_Read_Process(void);

/* ======================== FPU初始化 ======================== */
void FPU_Init(void)
{
    // 使能Cortex-M4 FPU (CP10/CP11)
    SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));
    
    // 设置FPU上下文保存 (lazy stacking)
    FPU->FPCCR |= FPU_FPCCR_ASPEN_Msk | FPU_FPCCR_LSPEN_Msk;
    
    fpu_enabled = 1;
}

/* ======================== printf重定向 ======================== */
int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart3, (uint8_t*)ptr, len, 1000);
    return len;
}

// 或使用:
// int __io_putchar(int ch)
// {
//     HAL_UART_Transmit(&huart3, (uint8_t*)&ch, 1, 100);
//     return ch;
// }

/* ======================== 初始化 ======================== */
void App_Init(void)
{
    // 1. 初始化FPU
    FPU_Init();
    
    // 2. 启动定时器中断 (10ms)
    HAL_TIM_Base_Start_IT(&htim2);
    
    // 3. 启动PWM (TIM3 CH1, PA6)
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    PWM_SET_DUTY(0);  // 初始占空比0%
    
    // 4. 启动串口接收中断
    HAL_UART_Receive_IT(&huart3, &rx_buf[0], 1);
    
    // 5. 配置NVIC优先级分组
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_2);
    
    // 6. 打印启动信息
    printf("\r\n");
    printf("============================================\r\n");
    printf("  STM32F407VET6 HAL Template\r\n");
    printf("  System Clock: %lu MHz\r\n", SystemCoreClock / 1000000);
    printf("  Flash: 512KB, SRAM: 192KB\r\n");
    printf("  FPU: %s\r\n", fpu_enabled ? "Enabled" : "Disabled");
    printf("============================================\r\n");
    printf("\r\n");
    printf("Commands:\r\n");
    printf("  1=LED_GREEN_ON, 2=LED_GREEN_OFF\r\n");
    printf("  3=LED_ORANGE_ON, 4=LED_ORANGE_OFF\r\n");
    printf("  t=Toggle, b=Blink, s=Status\r\n");
    printf("  p=PWM, a=ADC, ?=Help\r\n");
    printf("\r\n");
}

/* ======================== 主循环 ======================== */
void App_Loop(void)
{
    // 10ms 定时任务
    if (timer_10ms_flag) {
        timer_10ms_flag = 0;
        timer_count++;
        system_uptime = timer_count * 10;
        
        // 按键扫描 (每10ms)
        Key_Scan();
    }
    
    // 100ms 任务
    if ((timer_count % 10) == 0) {
        // LED状态指示
        if ((timer_count % 100) == 0) {
            LED_GREEN_TOGGLE();  // 每1秒闪烁
        }
        
        // ADC采样
        ADC_Read_Process();
    }
    
    // 1秒 任务
    if ((timer_count % 100) == 0 && timer_count > 0) {
        printf("Uptime: %lu s, ADC: %u (%.2fV)\r\n", 
               system_uptime / 1000, adc_value, adc_voltage);
    }
    
    // 按键处理
    if (key_pressed) {
        key_pressed = 0;
        printf("Key pressed!\r\n");
        LED_ORANGE_TOGGLE();
    }
    
    if (key_long_press) {
        key_long_press = 0;
        printf("Key long pressed!\r\n");
        // 长按功能: 切换运行模式
    }
    
    if (key_double_click) {
        key_double_click = 0;
        printf("Key double clicked!\r\n");
    }
    
    // 串口命令处理
    if (rx_flag) {
        rx_flag = 0;
        Process_UART_Command();
    }
}

/* ======================== 按键扫描 ======================== */
void Key_Scan(void)
{
    static uint8_t key_state = 1;       // 当前按键状态 (1=未按下)
    static uint8_t key_cnt = 0;         // 稳定计数器
    static uint8_t press_time = 0;      // 按下时长计数
    static uint8_t last_press_time = 0; // 上次按下时间
    static uint8_t double_click_flag = 0;
    static uint8_t double_click_window = 0;
    
    uint8_t cur_state = KEY_READ();
    
    // 双击检测窗口
    if (double_click_window > 0) {
        double_click_window--;
    }
    
    if (cur_state != key_state) {
        key_cnt++;
        if (key_cnt >= 3) {  // 30ms去抖 (3 x 10ms)
            key_state = cur_state;
            key_cnt = 0;
            
            if (key_state == 0) {       // 按下
                press_time = 0;
                
                // 检测双击
                if (double_click_flag && double_click_window > 0) {
                    key_double_click = 1;
                    double_click_flag = 0;
                }
            } else {                     // 松开
                if (press_time >= 20) {  // >= 200ms长按
                    key_long_press = 1;
                } else {
                    key_pressed = 1;
                    last_press_time = timer_count;
                    double_click_flag = 1;
                    double_click_window = 30;  // 300ms双击窗口
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
void Process_UART_Command(void)
{
    // 处理单个字符接收
    uint8_t cmd = rx_buf[0];
    
    switch (cmd) {
        case '1':
            LED_GREEN_ON();
            printf("Green LED ON\r\n");
            break;
        case '2':
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
            // 闪烁5次
            for (int i = 0; i < 5; i++) {
                LED_GREEN_TOGGLE();
                HAL_Delay(200);
            }
            printf("Blink done\r\n");
            break;
        case 's':
            printf("System Status:\r\n");
            printf("  Uptime: %lu ms\r\n", HAL_GetTick());
            printf("  Timer: %lu counts\r\n", timer_count);
            printf("  LED Green: %s\r\n", 
                HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_9) == GPIO_PIN_RESET ? "ON" : "OFF");
            printf("  LED Orange: %s\r\n", 
                HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_10) == GPIO_PIN_RESET ? "ON" : "OFF");
            printf("  Key: %s\r\n",
                HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET ? "PRESSED" : "RELEASED");
            break;
        case 'p':
            // PWM渐亮
            printf("PWM demo...\r\n");
            for (uint16_t i = 0; i <= 1000; i += 10) {
                PWM_SET_DUTY(i);
                HAL_Delay(5);
            }
            for (uint16_t i = 1000; i > 0; i -= 10) {
                PWM_SET_DUTY(i);
                HAL_Delay(5);
            }
            PWM_SET_DUTY(0);
            printf("PWM done\r\n");
            break;
        case 'a':
            // 手动读取ADC
            ADC_Read_Process();
            printf("ADC: %u (%.2fV)\r\n", adc_value, adc_voltage);
            break;
        case '?':
            printf("Commands:\r\n");
            printf("  1-4: LED control\r\n");
            printf("  t: Toggle LED\r\n");
            printf("  b: Blink LED\r\n");
            printf("  s: System status\r\n");
            printf("  p: PWM demo\r\n");
            printf("  a: Read ADC\r\n");
            printf("  ?: Help\r\n");
            break;
        default:
            printf("Unknown: %c (0x%02X)\r\n", cmd, cmd);
            break;
    }
    
    // 继续接收下一个字节
    HAL_UART_Receive_IT(&huart3, &rx_buf[0], 1);
}

/* ======================== ADC处理 ======================== */
void ADC_Read_Process(void)
{
    if (HAL_ADC_Start(&hadc1) == HAL_OK) {
        if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK) {
            adc_value = HAL_ADC_GetValue(&hadc1);
            adc_voltage = (float)adc_value * 3.3f / 4096.0f;
        }
    }
}

/* ======================== 中断回调函数 ======================== */

/* 定时器溢出回调 (TIM2) */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        timer_10ms_flag = 1;
    }
}

/* EXTI按键中断回调 (PA0) */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_0) {
        // 注意: 中断中不宜做太多事情
        // 只设置标志，在主循环中处理
        key_pressed = 1;
    }
}

/* 串口接收完成回调 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3) {
        rx_flag = 1;
    }
}

/* ADC转换完成回调 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1) {
        adc_value = HAL_ADC_GetValue(hadc);
        adc_voltage = (float)adc_value * 3.3f / 4096.0f;
    }
}

/* 错误回调 */
void HAL_ErrorCallback(void)
{
    __disable_irq();
    while (1) {
        // 错误死循环
        // 可以在这里添加错误指示 (如LED快速闪烁)
        for (volatile uint32_t i = 0; i < 100000; i++);
        HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_9);
    }
}

/* ======================== main函数 ======================== */
int main(void)
{
    // 1. HAL库初始化 (SysTick, NVIC)
    HAL_Init();
    
    // 2. 系统时钟配置 (168MHz)
    SystemClock_Config();
    
    // 3. 初始化外设 (CubeMX生成)
    MX_GPIO_Init();
    MX_USART3_UART_Init();
    MX_TIM2_Init();
    MX_TIM3_Init();
    MX_ADC1_Init();
    
    // 4. 应用初始化
    App_Init();
    
    // 5. 主循环
    while (1) {
        App_Loop();
    }
}
