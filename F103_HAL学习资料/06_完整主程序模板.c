/**
 * ============================================================================
 * STM32F103C8T6 HAL库 完整主程序模板
 * ============================================================================
 * 功能:
 *   - LED闪烁 (PC13)
 *   - 按键检测 (PA0, 轮询方式)
 *   - 串口打印 (USART1, 115200)
 *   - 定时器中断 (TIM2, 100ms)
 *   - 按键中断 (PA0, EXTI)
 *
 * 使用方法:
 *   1. 在CubeMX中使能: GPIO, USART1, TIM2, EXTI
 *   2. 将本文件中的代码复制到对应位置
 *   3. 注意: 本模板代码写在 USER CODE 区域外，仅供参考
 * ============================================================================
 */

/* ======================== main.c ======================== */

#include "main.h"
#include "gpio.h"
#include "usart.h"
#include "tim.h"

/* 标准库包含 (用于printf的字符串格式化) */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/* ======================== 宏定义 ======================== */
#define LED_ON()    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET)
#define LED_OFF()   HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET)
#define LED_TOGGLE() HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13)

#define KEY_READ()  HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)

/* ======================== 全局变量 ======================== */

// 串口接收缓冲
#define RX_BUF_SIZE 1
uint8_t rx_buf[RX_BUF_SIZE];
volatile uint8_t rx_flag = 0;

// 定时器标志 (由中断设置)
volatile uint8_t timer_100ms_flag = 0;
volatile uint32_t timer_count = 0;

// 按键状态
volatile uint8_t key_pressed = 0;
volatile uint8_t key_long_press = 0;

/* ======================== 函数声明 ======================== */
void SystemClock_Config(void);

/* 串口printf重定向 */
int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, 1000);
    return len;
}

/* ======================== 初始化 ======================== */

void App_Init(void)
{
    /* 1. 启动定时器中断 */
    HAL_TIM_Base_Start_IT(&htim2);

    /* 2. 启动串口接收中断 */
    HAL_UART_Receive_IT(&huart1, rx_buf, RX_BUF_SIZE);

    /* 3. 打印启动信息 */
    printf("\r\n========================================\r\n");
    printf("STM32F103C8T6 HAL Template\r\n");
    printf("System Clock: %lu MHz\r\n", SystemCoreClock / 1000000);
    printf("========================================\r\n\r\n");
}

/* ======================== 主循环 ======================== */

void App_Loop(void)
{
    /* 100ms 定时任务 */
    if (timer_100ms_flag) {
        timer_100ms_flag = 0;
        timer_count++;

        // LED状态闪烁 (1秒)
        if ((timer_count % 10) == 0) {
            LED_TOGGLE();
        }

        // 按键扫描
        Key_Scan();
    }

    /* 按键处理 */
    if (key_pressed) {
        key_pressed = 0;
        printf("Key pressed!\r\n");
        LED_TOGGLE();
    }

    if (key_long_press) {
        key_long_press = 0;
        printf("Key long pressed!\r\n");
    }

    /* 串口命令处理 */
    if (rx_flag) {
        rx_flag = 0;
        Process_UART_Command(rx_buf[0]);
        // 继续接收
        HAL_UART_Receive_IT(&huart1, rx_buf, RX_BUF_SIZE);
    }
}

/* ======================== 按键扫描 ======================== */

void Key_Scan(void)
{
    static uint8_t key_state = 1;      // 当前按键状态 (1=未按下)
    static uint8_t key_cnt = 0;        // 稳定计数器
    static uint8_t press_time = 0;     // 按下时长计数

    uint8_t cur_state = KEY_READ();

    if (cur_state != key_state) {
        key_cnt++;
        if (key_cnt >= 3) {  // 300ms去抖 (3 x 100ms)
            key_state = cur_state;
            key_cnt = 0;

            if (key_state == 0) {       // 按下
                press_time = 0;
            } else {                     // 松开
                if (press_time >= 20) {  // >= 2秒长按
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

void Process_UART_Command(uint8_t cmd)
{
    switch (cmd) {
        case '1':
            LED_ON();
            printf("LED ON\r\n");
            break;
        case '0':
            LED_OFF();
            printf("LED OFF\r\n");
            break;
        case 't':
            LED_TOGGLE();
            printf("LED TOGGLE\r\n");
            break;
        case 's':
            printf("System Tick: %lu ms\r\n", HAL_GetTick());
            break;
        case '?':
            printf("Commands: 1=ON, 0=OFF, t=Toggle, s=Status\r\n");
            break;
        default:
            printf("Unknown: %c\r\n", cmd);
            break;
    }
}

/* ======================== 中断回调函数 ======================== */

/* 定时器溢出回调 (TIM2) */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        timer_100ms_flag = 1;
    }
}

/* EXTI按键中断回调 (PA0) */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_0) {
        // 延时消抖
        HAL_Delay(20);
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) {
            LED_TOGGLE();
            // 注意: 中断中不宜做太多事情
            // 应设置标志，在主循环中处理
        }
    }
}

/* 串口接收完成回调 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        rx_flag = 1;
    }
}

/* 错误回调 */
void HAL_ErrorCallback(void)
{
    __disable_irq();
    while (1) {
        // 错误死循环
    }
}

/* ======================== main函数 ======================== */

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_TIM2_Init();

    App_Init();

    while (1) {
        App_Loop();
    }
}