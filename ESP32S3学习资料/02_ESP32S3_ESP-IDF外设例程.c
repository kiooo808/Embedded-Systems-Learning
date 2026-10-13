/**
 * ============================================================================
 * ESP32-S3 ESP-IDF 常用外设示例
 * ============================================================================
 * 本文件包含以下示例：
 *   1. GPIO 输入输出 (LED + 按键)
 *   2. UART 串口通信 (重定向printf)
 *   3. I2C 通信 (传感器/OLED)
 *   4. SPI 通信 (Flash/屏幕)
 *   5. ADC 模数转换
 *   6. LEDC PWM (调光)
 *   7. Touch 触摸传感器
 *   8. Wi-Fi 连接
 *   9. NVS 非易失性存储
 *  10. SPIFFS/LittleFS 文件系统
 *  11. FreeRTOS 多任务
 *  12. USB CDC
 *
 * ESP32-S3 常用引脚:
 *   - LED: GPIO2 (板载), GPIO4 (外接)
 *   - 按键: GPIO0 (BOOT), GPIO5 (外接)
 *   - I2C: GPIO8(SDA), GPIO9(SCL)
 *   - SPI: GPIO11(SCK), GPIO12(MISO), GPIO13(MOSI)
 *   - UART0: GPIO43(TX), GPIO44(RX)
 *   - ADC: GPIO1(ADC1_CH0), GPIO2(ADC1_CH1)
 *   - Touch: GPIO14(T1), GPIO15(T2)
 *
 * 使用方法：将需要的示例代码复制到 main 组件对应位置
 *   - 初始化代码放入 app_main() 或 init 函数
 *   - 任务函数放入 task 实现
 *   - 头文件包含需在 CMakeLists.txt 中声明对应依赖
 * ============================================================================
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "driver/adc.h"
#include "driver/ledc.h"
#include "driver/touch_pad.h"
#include "driver/usb_serial_jtag.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#include <stdio.h>
#include <string.h>

static const char *TAG = "ESP32S3_Example";


/* ============================================================================
 * 第一部分：GPIO 示例
 * ============================================================================
 */

/*
 * 1.1 GPIO 初始化
 */
void GPIO_Init(void)
{
    /* 配置LED引脚 (GPIO2 - 推挽输出) */
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_NUM_2),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    /* 配置按键引脚 (GPIO5 - 上拉输入) */
    io_conf.pin_bit_mask = (1ULL << GPIO_NUM_5);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.intr_type = GPIO_INTR_NEGEDGE;  // 下降沿中断
    gpio_config(&io_conf);
}

/*
 * 1.2 GPIO 输出 - LED闪烁 (FreeRTOS任务)
 */
void LED_Task(void *arg)
{
    while (1) {
        gpio_set_level(GPIO_NUM_2, 1);  // LED亮
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_set_level(GPIO_NUM_2, 0);  // LED灭
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/*
 * 1.3 GPIO 输入 - 按键轮询
 */
void Key_Task(void *arg)
{
    int last_state = 1;
    while (1) {
        int cur_state = gpio_get_level(GPIO_NUM_5);
        if (cur_state != last_state) {
            vTaskDelay(pdMS_TO_TICKS(20));  // 消抖
            cur_state = gpio_get_level(GPIO_NUM_5);
            if (cur_state == 0) {
                ESP_LOGI(TAG, "Key pressed!");
                gpio_set_level(GPIO_NUM_2, 1);
            } else {
                ESP_LOGI(TAG, "Key released!");
                gpio_set_level(GPIO_NUM_2, 0);
            }
            last_state = cur_state;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/*
 * 1.4 GPIO 中断方式
 */
void IRAM_ATTR gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    ESP_EARLY_LOGI(TAG, "GPIO intr: %d", gpio_num);
    // 注意：中断中不要使用阻塞函数
    // 建议通过队列通知任务处理
}

void GPIO_Interrupt_Init(void)
{
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_NUM_5, gpio_isr_handler, (void *)GPIO_NUM_5);
}


/* ============================================================================
 * 第二部分：UART 串口通信
 * ============================================================================
 */

#define UART_PORT_NUM UART_NUM_1
#define UART_TX_PIN GPIO_NUM_43
#define UART_RX_PIN GPIO_NUM_44
#define UART_BAUD_RATE 115200

static QueueHandle_t uart_rx_queue;

/*
 * 2.1 UART 初始化
 */
void UART_Init(void)
{
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    uart_driver_install(UART_PORT_NUM, 1024, 0, 20, &uart_rx_queue, 0);
    uart_param_config(UART_PORT_NUM, &uart_config);
    uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN,
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

/*
 * 2.2 UART 发送数据
 */
void UART_Send_Example(void)
{
    const char *msg = "Hello ESP32-S3!\r\n";
    uart_write_bytes(UART_PORT_NUM, msg, strlen(msg));

    // 发送二进制数据
    uint8_t data[] = {0xAA, 0x55, 0x01, 0x02};
    uart_write_bytes(UART_PORT_NUM, data, sizeof(data));
}

/*
 * 2.3 UART 接收任务
 */
void UART_Receive_Task(void *arg)
{
    uart_event_t event;
    uint8_t data[128];

    while (1) {
        if (xQueueReceive(uart_rx_queue, &event, pdMS_TO_TICKS(100))) {
            switch (event.type) {
                case UART_DATA:
                    int len = uart_read_bytes(UART_PORT_NUM, data,
                              event.size, pdMS_TO_TICKS(50));
                    ESP_LOGI(TAG, "Received %d bytes", len);
                    // 处理接收到的数据...
                    break;
                case UART_FIFO_OVF:
                    uart_flush_input(UART_PORT_NUM);
                    break;
                case UART_BUFFER_FULL:
                    uart_flush_input(UART_PORT_NUM);
                    break;
                case UART_BREAK:
                case UART_PARITY_ERR:
                case UART_FRAME_ERR:
                    break;
                default:
                    break;
            }
        }
    }
}

/*
 * 2.4 重定向 printf 到 USB CDC (ESP32-S3 专用)
 */
#include "driver/usb_serial_jtag.h"

void USB_CDC_Init(void)
{
    usb_serial_jtag_config_t usb_serial_jtag_config = {
        .baud_rate = 115200,
        .data_bits = USB_SERIAL_JTAG_DATA_BITS_8,
        .parity = USB_SERIAL_JTAG_PARITY_DISABLE,
        .stop_bits = USB_SERIAL_JTAG_STOP_BITS_1,
    };
    usb_serial_jtag_driver_install(&usb_serial_jtag_config);

    // 设置为默认串口输出
    esp_err_t err = usb_serial_jtag_driver_config_uart_port(USB_SERIAL_JTAG_USB_PORT_NUM);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "USB CDC initialized");
    }
}


/* ============================================================================
 * 第三部分：I2C 通信
 * ============================================================================
 */

#define I2C_PORT_NUM I2C_NUM_0
#define I2C_SDA_PIN GPIO_NUM_8
#define I2C_SCL_PIN GPIO_NUM_9
#define I2C_SPEED_HZ 400000

/*
 * 3.1 I2C 初始化
 */
void I2C_Init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_SPEED_HZ,
    };
    i2c_param_config(I2C_PORT_NUM, &conf);
    i2c_driver_install(I2C_PORT_NUM, I2C_MODE_MASTER, 0, 0, 0);
}

/*
 * 3.2 I2C 写数据
 */
esp_err_t I2C_Write(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write(cmd, data, len, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_PORT_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}

/*
 * 3.3 I2C 读数据
 */
esp_err_t I2C_Read(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);  // Repeated START
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, len - 1, true);   // 最后一个字节不发ACK
    i2c_master_read_byte(cmd, data + len - 1, false);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_PORT_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}

/*
 * 3.4 扫描 I2C 总线设备
 */
void I2C_Scan(void)
{
    ESP_LOGI(TAG, "Scanning I2C bus...");
    for (uint8_t addr = 1; addr < 127; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        esp_err_t ret = i2c_master_cmd_begin(I2C_PORT_NUM, cmd, pdMS_TO_TICKS(10));
        i2c_cmd_link_delete(cmd);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Found device at 0x%02X", addr);
        }
    }
    ESP_LOGI(TAG, "Scan complete");
}


/* ============================================================================
 * 第四部分：SPI 通信
 * ============================================================================
 */

#define SPI_HOST_NUM SPI2_HOST
#define SPI_MOSI_PIN GPIO_NUM_13
#define SPI_MISO_PIN GPIO_NUM_12
#define SPI_SCLK_PIN GPIO_NUM_11
#define SPI_CS_PIN GPIO_NUM_10

/*
 * 4.1 SPI 初始化
 */
void SPI_Init(void)
{
    spi_bus_config_t buscfg = {
        .mosi_io_num = SPI_MOSI_PIN,
        .miso_io_num = SPI_MISO_PIN,
        .sclk_io_num = SPI_SCLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };
    spi_bus_initialize(SPI_HOST_NUM, &buscfg, SPI_DMA_CH_AUTO);

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 10 * 1000 * 1000,  // 10MHz
        .mode = 0,
        .spics_io_num = SPI_CS_PIN,
        .queue_size = 7,
    };
    spi_bus_add_device(SPI_HOST_NUM, &devcfg);
}

/*
 * 4.2 SPI 数据传输
 */
uint8_t SPI_Transfer(uint8_t data)
{
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &data,
        .rx_buffer = &data,
    };
    spi_device_polling_transmit(SPI_HOST_NUM, &t);
    return data;
}

void SPI_Transfer_Buf(uint8_t *tx, uint8_t *rx, int len)
{
    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = tx,
        .rx_buffer = rx,
    };
    spi_device_polling_transmit(SPI_HOST_NUM, &t);
}


/* ============================================================================
 * 第五部分：ADC 模数转换
 * ============================================================================
 */

/*
 * 5.1 ADC 初始化（单次采样）
 */
void ADC_Init(void)
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL_0, ADC_ATTEN_DB_12);

    // 配置 GPIO1 为模拟输入
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_NUM_1),
        .mode = GPIO_MODE_ANALOG,
    };
    gpio_config(&io_conf);
}

/*
 * 5.2 ADC 读取
 */
uint16_t ADC_Read(void)
{
    return adc1_get_raw(ADC_CHANNEL_0);
}

float ADC_Read_Voltage(void)
{
    uint16_t raw = adc1_get_raw(ADC_CHANNEL_0);
    // ESP32-S3 ADC参考电压3.3V，12位
    return (float)raw * 3.3f / 4096.0f;
}


/* ============================================================================
 * 第六部分：LEDC PWM
 * ============================================================================
 */

#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHAN LEDC_CHANNEL_0
#define LEDC_PIN GPIO_NUM_4
#define LEDC_FREQ_HZ 5000    // 5kHz
#define LEDC_RESOLUTION LEDC_TIMER_10_BIT  // 1024级

/*
 * 6.1 LEDC 初始化
 */
void LEDC_Init(void)
{
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_MODE,
        .duty_resolution = LEDC_RESOLUTION,
        .timer_num = LEDC_TIMER,
        .freq_hz = LEDC_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer_conf);

    ledc_channel_config_t chan_conf = {
        .gpio_num = LEDC_PIN,
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHAN,
        .timer_sel = LEDC_TIMER,
        .duty = 0,
        .hpoint = 0,
    };
    ledc_channel_config(&chan_conf);
}

/*
 * 6.2 LEDC 设置占空比
 */
void LEDC_SetDuty(uint16_t duty)
{
    ledc_set_duty(LEDC_MODE, LEDC_CHAN, duty);
    ledc_update_duty(LEDC_MODE, LEDC_CHAN);
}

/*
 * 6.3 呼吸灯效果
 */
void LEDC_Breathing_Task(void *arg)
{
    uint16_t duty = 0;
    int dir = 1;

    while (1) {
        LEDC_SetDuty(duty);
        vTaskDelay(pdMS_TO_TICKS(5));

        duty += dir * 5;
        if (duty >= 1023) dir = -1;
        if (duty <= 0) dir = 1;
    }
}


/* ============================================================================
 * 第七部分：Touch 触摸传感器
 * ============================================================================
 */

#define TOUCH_PAD_NUM TOUCH_PAD_NUM1  // GPIO14

/*
 * 7.1 Touch 初始化
 */
void Touch_Init(void)
{
    touch_pad_init();
    touch_pad_config(TOUCH_PAD_NUM);
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    touch_pad_fsm_start();
}

/*
 * 7.2 Touch 读取
 */
uint32_t Touch_Read(void)
{
    uint32_t value = 0;
    touch_pad_read_raw_data(TOUCH_PAD_NUM, &value);
    return value;
}


/* ============================================================================
 * 第八部分：Wi-Fi 连接
 * ============================================================================
 */

#define WIFI_SSID "YourWiFiSSID"
#define WIFI_PASS "YourWiFiPassword"

/*
 * 8.1 Wi-Fi 初始化与连接
 */
void WiFi_Init(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi connecting...");
}

/*
 * 8.2 Wi-Fi 事件处理
 */
void wifi_event_handler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "WiFi disconnected, reconnecting...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}


/* ============================================================================
 * 第九部分：NVS 非易失性存储
 * ============================================================================
 */

/*
 * 9.1 NVS 初始化
 */
void NVS_Init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

/*
 * 9.2 NVS 读写示例
 */
void NVS_Example(void)
{
    nvs_handle_t my_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &my_handle));

    // 写入
    ESP_ERROR_CHECK(nvs_set_i32(my_handle, "boot_count", 1));
    ESP_ERROR_CHECK(nvs_set_str(my_handle, "wifi_ssid", "MyWiFi")));
    ESP_ERROR_CHECK(nvs_set_u8(my_handle, "calibration", 42));

    // 提交
    ESP_ERROR_CHECK(nvs_commit(my_handle));
    nvs_close(my_handle);

    // 读取
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READONLY, &my_handle));
    int32_t boot_count;
    ESP_ERROR_CHECK(nvs_get_i32(my_handle, "boot_count", &boot_count));
    ESP_LOGI(TAG, "Boot count: %d", boot_count);

    char ssid[32];
    size_t len = sizeof(ssid);
    ESP_ERROR_CHECK(nvs_get_str(my_handle, "wifi_ssid", ssid, &len));
    ESP_LOGI(TAG, "WiFi SSID: %s", ssid);

    nvs_close(my_handle);
}


/* ============================================================================
 * 第十部分：SPIFFS/LittleFS 文件系统
 * ============================================================================
 */

/*
 * 10.1 SPIFFS 挂载与读写
 */
void SPIFFS_Example(void)
{
    esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
    };

    ESP_ERROR_CHECK(esp_vfs_fat_spiffs_mount(
        "/spiffs", "storage", &mount_config));

    // 写文件
    FILE *f = fopen("/spiffs/test.txt", "w");
    if (f) {
        fprintf(f, "Hello SPIFFS!\n");
        fclose(f);
    }

    // 读文件
    f = fopen("/spiffs/test.txt", "r");
    if (f) {
        char buf[64];
        fgets(buf, sizeof(buf), f);
        ESP_LOGI(TAG, "Read: %s", buf);
        fclose(f);
    }
}


/* ============================================================================
 * 第十一部分：FreeRTOS 多任务
 * ============================================================================
 */

/*
 * 11.1 多任务创建
 */
void FreeRTOS_Example(void)
{
    // 从核心0创建任务
    xTaskCreatePinnedToCore(
        LED_Task,       // 任务函数
        "LED",          // 任务名
        2048,           // 栈大小
        NULL,           // 参数
        5,              // 优先级
        NULL,           // 任务句柄
        0               // 核心0
    );

    xTaskCreatePinnedToCore(
        Key_Task,
        "Key",
        2048,
        NULL,
        4,
        NULL,
        0
    );

    xTaskCreatePinnedToCore(
        UART_Receive_Task,
        "UART_RX",
        4096,
        NULL,
        3,
        NULL,
        1  // 核心1
    );
}

/*
 * 11.2 队列通信示例
 */
void Queue_Example(void)
{
    QueueHandle_t data_queue = xQueueCreate(10, sizeof(int));

    // 发送任务
    void sender_task(void *arg) {
        int data = 0;
        while (1) {
            xQueueSend(data_queue, &data, portMAX_DELAY);
            data++;
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    // 接收任务
    void receiver_task(void *arg) {
        int data;
        while (1) {
            xQueueReceive(data_queue, &data, portMAX_DELAY);
            ESP_LOGI(TAG, "Received: %d", data);
        }
    }

    xTaskCreate(sender_task, "Sender", 2048, NULL, 5, NULL);
    xTaskCreate(receiver_task, "Receiver", 2048, NULL, 5, NULL);
}


/* ============================================================================
 * 第十二部分：读取芯片信息
 * ============================================================================
 */

void Print_Chip_Info(void)
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    ESP_LOGI(TAG, "Chip: ESP32-%s",
             chip_info.model == CHIP_ESP32S3 ? "S3" : "Other");
    ESP_LOGI(TAG, "Cores: %d", chip_info.cores);
    ESP_LOGI(TAG, "Features: 0x%x", chip_info.features);
    ESP_LOGI(TAG, "Revision: %d", chip_info.revision);
    ESP_LOGI(TAG, "CPU Freq: %d MHz", esp_clk_cpu_freq() / 1000000);

    // 获取 MAC 地址
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    ESP_LOGI(TAG, "MAC: %02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    // Flash 大小
    ESP_LOGI(TAG, "Flash size: %d KB",
             esp_flash_get_size(NULL) / 1024);
}
