/**
 * ============================================================================
 * ESP32-S3 ESP-IDF 完整主程序模板
 * ============================================================================
 * 功能模块：
 *   1. GPIO LED 闪烁任务
 *   2. GPIO 按键检测任务
 *   3. UART 串口通信
 *   4. ADC 电压采集
 *   5. Wi-Fi 连接
 *   6. NVS 参数存储
 *   7. FreeRTOS 多任务协作
 *
 * ESP32-S3 硬件连接：
 *   - LED: GPIO2 (板载)
 *   - 按键: GPIO0 (BOOT键) / GPIO5 (外接)
 *   - I2C: GPIO8(SDA) / GPIO9(SCL)
 *   - SPI: GPIO11(SCK) / GPIO12(MISO) / GPIO13(MOSI) / GPIO10(CS)
 *   - UART0: GPIO43(TX) / GPIO44(RX)
 *   - ADC: GPIO1(ADC1_CH0)
 *   - Touch: GPIO14(T1)
 *
 * 使用说明：
 *   1. 将此文件替换到 main/main.c
 *   2. 确保 main/CMakeLists.txt 中 REQUIRES 包含所有必要组件
 *   3. 根据实际硬件修改引脚定义和配置
 * ============================================================================
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_clk.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "driver/adc.h"
#include "driver/ledc.h"
#include "driver/touch_pad.h"
#include "driver/usb_serial_jtag.h"

#include <stdio.h>
#include <string.h>
#include <math.h>


/* ============================================================================
 * 配置区 - 根据实际硬件修改
 * ============================================================================
 */

/* LED 引脚 */
#define LED_PIN             GPIO_NUM_2
#define LED_LEVEL_ON        1       // LED 点亮电平（高电平点亮）
#define LED_LEVEL_OFF       0

/* 按键引脚 */
#define BUTTON_PIN          GPIO_NUM_5
#define BUTTON_ACTIVE_LEVEL 0       // 按键有效电平（低电平有效）

/* UART 配置 */
#define UART_PORT           UART_NUM_1
#define UART_TX_PIN         GPIO_NUM_43
#define UART_RX_PIN         GPIO_NUM_44
#define UART_BAUD_RATE      115200

/* I2C 配置 */
#define I2C_PORT            I2C_NUM_0
#define I2C_SDA_PIN         GPIO_NUM_8
#define I2C_SCL_PIN         GPIO_NUM_9
#define I2C_SPEED_HZ        400000

/* SPI 配置 */
#define SPI_HOST            SPI2_HOST
#define SPI_MOSI_PIN         GPIO_NUM_13
#define SPI_MISO_PIN         GPIO_NUM_12
#define SPI_SCLK_PIN        GPIO_NUM_11
#define SPI_CS_PIN          GPIO_NUM_10
#define SPI_SPEED_HZ         (10 * 1000 * 1000)

/* ADC 配置 */
#define ADC_CHANNEL         ADC_CHANNEL_0  // GPIO1
#define ADC_ATTEN           ADC_ATTEN_DB_12

/* LEDC (PWM) 配置 */
#define LEDC_TIMER_NUM      LEDC_TIMER_0
#define LEDC_CHANNEL_NUM    LEDC_CHANNEL_0
#define LEDC_PIN            GPIO_NUM_4
#define LEDC_FREQ_HZ        5000
#define LEDC_RESOLUTION     LEDC_TIMER_10_BIT
#define LEDC_MAX_DUTY       1023

/* Touch 配置 */
#define TOUCH_PAD           TOUCH_PAD_NUM1  // GPIO14

/* Wi-Fi 配置 */
#define WIFI_SSID           "YourWiFiSSID"
#define WIFI_PASS           "YourWiFiPassword"

/* 系统任务配置 */
#define TASK_LED_STACK      2048
#define TASK_LED_PRIORITY   5
#define TASK_KEY_STACK      2048
#define TASK_KEY_PRIORITY   4
#define TASK_UART_STACK     4096
#define TASK_UART_PRIORITY  3
#define TASK_ADC_STACK      2048
#define TASK_ADC_PRIORITY   2
#define TASK_WIFI_STACK     4096
#define TASK_WIFI_PRIORITY  4


/* ============================================================================
 * 全局变量
 * ============================================================================
 */

static const char *TAG = "ESP32S3_Main";

/* 事件组 */
static EventGroupHandle_t sys_event_group;
#define SYS_EVENT_WIFI_CONNECTED_BIT  (1 << 0)
#define SYS_EVENT_WIFI_DISCONNECTED_BIT (1 << 1)
#define SYS_EVENT_BUTTON_PRESSED_BIT (1 << 2)

/* 串口接收队列 */
static QueueHandle_t uart_rx_queue;

/* NVS 句柄 */
static nvs_handle_t nvs_handle;


/* ============================================================================
 * 函数前置声明
 * ============================================================================
 */

static void GPIO_Init(void);
static void UART_Init(void);
static void I2C_Init(void);
static void SPI_Init(void);
static void ADC_Init(void);
static void LEDC_Init(void);
static void Touch_Init(void);
static void NVS_Init(void);
static void WiFi_Init(void);
static void Print_Chip_Info(void);

static void LED_Task(void *arg);
static void Key_Task(void *arg);
static void UART_Receive_Task(void *arg);
static void ADC_Task(void *arg);
static void WiFi_Task(void *arg);

static void WiFi_Event_Handler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data);
static void IP_Event_Handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data);


/* ============================================================================
 * app_main - 程序入口
 * ============================================================================
 */

void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  ESP32-S3 Project Starting...");
    ESP_LOGI(TAG, "========================================");

    /* 打印芯片信息 */
    Print_Chip_Info();

    /* 创建事件组 */
    sys_event_group = xEventGroupCreate();

    /* 初始化各外设 */
    ESP_LOGI(TAG, "Initializing peripherals...");

    NVS_Init();
    GPIO_Init();
    UART_Init();
    I2C_Init();
    SPI_Init();
    ADC_Init();
    LEDC_Init();
    Touch_Init();
    WiFi_Init();

    /* 读取 NVS 中保存的参数 */
    int32_t boot_count = 0;
    nvs_get_i32(nvs_handle, "boot_count", &boot_count);
    boot_count++;
    nvs_set_i32(nvs_handle, "boot_count", boot_count);
    nvs_commit(nvs_handle);
    ESP_LOGI(TAG, "Boot count: %d", boot_count);

    /* 发送启动信息到串口 */
    UART_Send_String("\r\nESP32-S3 Ready!\r\n");
    UART_Send_String("================\r\n");

    /* 创建 FreeRTOS 任务 */
    ESP_LOGI(TAG, "Creating tasks...");

    xTaskCreatePinnedToCore(LED_Task, "LED", TASK_LED_STACK, NULL,
                            TASK_LED_PRIORITY, NULL, 0);

    xTaskCreatePinnedToCore(Key_Task, "Key", TASK_KEY_STACK, NULL,
                            TASK_KEY_PRIORITY, NULL, 0);

    xTaskCreatePinnedToCore(UART_Receive_Task, "UART_RX", TASK_UART_STACK,
                            NULL, TASK_UART_PRIORITY, NULL, 1);

    xTaskCreatePinnedToCore(ADC_Task, "ADC", TASK_ADC_STACK, NULL,
                            TASK_ADC_PRIORITY, NULL, 1);

    xTaskCreatePinnedToCore(WiFi_Task, "WiFi", TASK_WIFI_STACK, NULL,
                            TASK_WIFI_PRIORITY, NULL, 0);

    ESP_LOGI(TAG, "All tasks created. System ready!");
}


/* ============================================================================
 * 初始化函数
 * ============================================================================
 */

static void Print_Chip_Info(void)
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    ESP_LOGI(TAG, "Chip: ESP32-%s",
             chip_info.model == CHIP_ESP32S3 ? "S3" : "Unknown");
    ESP_LOGI(TAG, "Cores: %d", chip_info.cores);
    ESP_LOGI(TAG, "CPU Freq: %d MHz", esp_clk_cpu_freq() / 1000000);
    ESP_LOGI(TAG, "Revision: %d", chip_info.revision);
    ESP_LOGI(TAG, "Features: 0x%04X", chip_info.features);

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    ESP_LOGI(TAG, "MAC: %02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    size_t flash_size = 0;
    esp_flash_get_size(NULL, &flash_size);
    ESP_LOGI(TAG, "Flash: %zu KB", flash_size / 1024);

    ESP_LOGI(TAG, "Free heap: %lu bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "Free internal: %lu bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    ESP_LOGI(TAG, "Free SPIRAM: %lu bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
}

static void NVS_Init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS not initialized, erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* 打开 NVS 命名空间 */
    ret = nvs_open("config", NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "NVS open failed: %s", esp_err_to_name(ret));
    }
    ESP_LOGI(TAG, "NVS initialized");
}

static void GPIO_Init(void)
{
    /* 配置 LED 引脚 */
    gpio_config_t led_conf = {
        .pin_bit_mask = (1ULL << LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&led_conf);
    gpio_set_level(LED_PIN, LED_LEVEL_OFF);

    /* 配置按键引脚 */
    gpio_config_t key_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&key_conf);

    ESP_LOGI(TAG, "GPIO initialized");
}

static void UART_Init(void)
{
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    uart_driver_install(UART_PORT, 1024, 0, 20, &uart_rx_queue, 0);
    uart_param_config(UART_PORT, &uart_config);
    uart_set_pin(UART_PORT, UART_TX_PIN, UART_RX_PIN,
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    ESP_LOGI(TAG, "UART%d initialized at %d baud", UART_PORT, UART_BAUD_RATE);
}

static void I2C_Init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_SPEED_HZ,
    };
    i2c_param_config(I2C_PORT, &conf);
    i2c_driver_install(I2C_PORT, I2C_MODE_MASTER, 0, 0, 0);

    ESP_LOGI(TAG, "I2C%d initialized at %d Hz", I2C_PORT, I2C_SPEED_HZ);
}

static void SPI_Init(void)
{
    spi_bus_config_t buscfg = {
        .mosi_io_num = SPI_MOSI_PIN,
        .miso_io_num = SPI_MISO_PIN,
        .sclk_io_num = SPI_SCLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };
    spi_bus_initialize(SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = SPI_SPEED_HZ,
        .mode = 0,
        .spics_io_num = SPI_CS_PIN,
        .queue_size = 7,
    };
    spi_bus_add_device(SPI_HOST, &devcfg);

    ESP_LOGI(TAG, "SPI initialized at %d Hz", SPI_SPEED_HZ);
}

static void ADC_Init(void)
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN);

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_NUM_1),
        .mode = GPIO_MODE_ANALOG,
    };
    gpio_config(&io_conf);

    ESP_LOGI(TAG, "ADC initialized");
}

static void LEDC_Init(void)
{
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_RESOLUTION,
        .timer_num = LEDC_TIMER_NUM,
        .freq_hz = LEDC_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer_conf);

    ledc_channel_config_t chan_conf = {
        .gpio_num = LEDC_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_NUM,
        .timer_sel = LEDC_TIMER_NUM,
        .duty = 0,
        .hpoint = 0,
    };
    ledc_channel_config(&chan_conf);

    ESP_LOGI(TAG, "LEDC initialized at %d Hz", LEDC_FREQ_HZ);
}

static void Touch_Init(void)
{
    touch_pad_init();
    touch_pad_config(TOUCH_PAD);
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    touch_pad_fsm_start();

    ESP_LOGI(TAG, "Touch initialized");
}

static void WiFi_Init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* 注册事件处理 */
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                WiFi_Event_Handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                IP_Event_Handler, NULL);

    /* 配置并启动 Wi-Fi */
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi connecting to %s...", WIFI_SSID);
}


/* ============================================================================
 * 事件处理函数
 * ============================================================================
 */

static void WiFi_Event_Handler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "WiFi STA started");
                esp_wifi_connect();
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGW(TAG, "WiFi disconnected, reconnecting...");
                xEventGroupSetBits(sys_event_group, SYS_EVENT_WIFI_DISCONNECTED_BIT);
                esp_wifi_connect();
                break;
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "WiFi connected to AP");
                break;
            default:
                break;
        }
    }
}

static void IP_Event_Handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(sys_event_group, SYS_EVENT_WIFI_CONNECTED_BIT);
    }
}


/* ============================================================================
 * FreeRTOS 任务函数
 * ============================================================================
 */

/*
 * LED 闪烁任务
 */
static void LED_Task(void *arg)
{
    uint32_t blink_count = 0;

    ESP_LOGI(TAG, "LED task started on core %d", xPortGetCoreID());

    while (1) {
        gpio_set_level(LED_PIN, LED_LEVEL_ON);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_set_level(LED_PIN, LED_LEVEL_OFF);
        vTaskDelay(pdMS_TO_TICKS(500));
        blink_count++;

        /* 每10次切换发送一次状态 */
        if (blink_count % 10 == 0) {
            ESP_LOGI(TAG, "LED blinked %lu times", blink_count);
        }
    }
}

/*
 * 按键检测任务
 */
static void Key_Task(void *arg)
{
    int last_state = 1;
    uint32_t press_count = 0;

    ESP_LOGI(TAG, "Key task started on core %d", xPortGetCoreID());

    while (1) {
        int cur_state = gpio_get_level(BUTTON_PIN);

        if (cur_state != last_state) {
            vTaskDelay(pdMS_TO_TICKS(20));  // 软件消抖
            cur_state = gpio_get_level(BUTTON_PIN);

            if (cur_state == BUTTON_ACTIVE_LEVEL && last_state != BUTTON_ACTIVE_LEVEL) {
                /* 按键按下 */
                press_count++;
                ESP_LOGI(TAG, "Button pressed! (count: %lu)", press_count);

                /* 设置事件位，通知其他任务 */
                xEventGroupSetBits(sys_event_group, SYS_EVENT_BUTTON_PRESSED_BIT);

                /* 切换 LED 状态 */
                static int led_state = 0;
                led_state = !led_state;
                gpio_set_level(LED_PIN, led_state);

                /* 保存按键次数到 NVS */
                nvs_set_u32(nvs_handle, "press_count", press_count);
                nvs_commit(nvs_handle);

            } else if (cur_state != BUTTON_ACTIVE_LEVEL && last_state == BUTTON_ACTIVE_LEVEL) {
                /* 按键释放 */
                ESP_LOGD(TAG, "Button released");
            }

            last_state = cur_state;
        }

        vTaskDelay(pdMS_TO_TICKS(10));  // 轮询间隔
    }
}

/*
 * UART 接收任务
 */
static void UART_Receive_Task(void *arg)
{
    uart_event_t event;
    uint8_t rx_buf[256];

    ESP_LOGI(TAG, "UART receive task started on core %d", xPortGetCoreID());
    UART_Send_String("UART Ready. Send 'help' for commands.\r\n");

    while (1) {
        if (xQueueReceive(uart_rx_queue, &event, pdMS_TO_TICKS(100))) {
            switch (event.type) {
                case UART_DATA: {
                    int len = uart_read_bytes(UART_PORT, rx_buf,
                              event.size, pdMS_TO_TICKS(50));
                    if (len > 0) {
                        /* 处理接收到的命令 */
                        if (strncmp((char *)rx_buf, "help", 4) == 0) {
                            UART_Send_String("Commands: led, status, reboot\r\n");
                        } else if (strncmp((char *)rx_buf, "led", 3) == 0) {
                            gpio_set_level(LED_PIN, !gpio_get_level(LED_PIN));
                            UART_Send_String("LED toggled\r\n");
                        } else if (strncmp((char *)rx_buf, "status", 6) == 0) {
                            char msg[128];
                            snprintf(msg, sizeof(msg),
                                     "Free heap: %lu, Uptime: %lus\r\n",
                                     esp_get_free_heap_size(),
                                     (uint32_t)(esp_timer_get_time() / 1000000));
                            UART_Send_String(msg);
                        } else if (strncmp((char *)rx_buf, "reboot", 6) == 0) {
                            UART_Send_String("Rebooting...\r\n");
                            vTaskDelay(pdMS_TO_TICKS(100));
                            esp_restart();
                        } else {
                            char echo[64];
                            int echo_len = snprintf(echo, sizeof(echo),
                                                    "Echo: %s\r\n", rx_buf);
                            uart_write_bytes(UART_PORT, echo, echo_len);
                        }
                    }
                    break;
                }
                case UART_FIFO_OVF:
                case UART_BUFFER_FULL:
                    uart_flush_input(UART_PORT);
                    ESP_LOGW(TAG, "UART overflow, flushed");
                    break;
                case UART_BREAK:
                    ESP_LOGI(TAG, "UART break detected");
                    break;
                default:
                    break;
            }
        }
    }
}

/*
 * ADC 采集任务
 */
static void ADC_Task(void *arg)
{
    ESP_LOGI(TAG, "ADC task started on core %d", xPortGetCoreID());

    while (1) {
        /* 读取原始 ADC 值 */
        uint16_t raw = adc1_get_raw(ADC_CHANNEL);

        /* 转换为电压值 */
        float voltage = (float)raw * 3.3f / 4096.0f;

        /* 每秒打印一次 */
        static uint32_t last_print = 0;
        uint32_t now = esp_timer_get_time() / 1000000;
        if (now - last_print >= 1) {
            ESP_LOGI(TAG, "ADC: raw=%d, voltage=%.3fV", raw, voltage);
            last_print = now;
        }

        vTaskDelay(pdMS_TO_TICKS(100));  // 100ms 采集间隔
    }
}

/*
 * Wi-Fi 状态任务
 */
static void WiFi_Task(void *arg)
{
    ESP_LOGI(TAG, "WiFi task started on core %d", xPortGetCoreID());

    while (1) {
        /* 等待 Wi-Fi 连接 */
        EventBits_t bits = xEventGroupWaitBits(sys_event_group,
            SYS_EVENT_WIFI_CONNECTED_BIT | SYS_EVENT_WIFI_DISCONNECTED_BIT,
            pdTRUE, pdFALSE, portMAX_DELAY);

        if (bits & SYS_EVENT_WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG, "WiFi connected, ready for TCP/UDP");

            /* 在此可以创建 TCP/UDP 连接 */

        } else if (bits & SYS_EVENT_WIFI_DISCONNECTED_BIT) {
            ESP_LOGW(TAG, "WiFi disconnected, will auto-reconnect");
        }
    }
}


/* ============================================================================
 * 工具函数
 * ============================================================================
 */

/* UART 发送字符串 */
void UART_Send_String(const char *str)
{
    uart_write_bytes(UART_PORT, str, strlen(str));
}

/* UART 发送格式化字符串 */
void UART_Send_Printf(const char *format, ...)
{
    char buf[256];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    if (len > 0) {
        uart_write_bytes(UART_PORT, buf, len);
    }
}

/* I2C 扫描总线设备 */
void I2C_Scan(void)
{
    ESP_LOGI(TAG, "Scanning I2C bus...");
    for (uint8_t addr = 1; addr < 127; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(10));
        i2c_cmd_link_delete(cmd);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Found device at 0x%02X", addr);
        }
    }
    ESP_LOGI(TAG, "Scan complete");
}

/* LEDC 设置占空比 */
void LEDC_Set_Duty(uint16_t duty)
{
    if (duty > LEDC_MAX_DUTY) duty = LEDC_MAX_DUTY;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_NUM, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_NUM);
}

/* 读取 Touch 原始值 */
uint32_t Touch_Read_Value(void)
{
    uint32_t value = 0;
    touch_pad_read_raw_data(TOUCH_PAD, &value);
    return value;
}
