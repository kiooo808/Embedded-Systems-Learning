# ESP32-S3 学习路径 - 第二章：ESP-IDF 外设开发

## 学习目标
- 掌握 ESP32-S3 GPIO、UART、SPI、I2C、ADC 开发
- 学会使用 LEDC、RMT、Pulse Counter 等外设
- 实现完整的 ESP32-S3 外设应用

---

## 第一节：GPIO 开发

### 1.1 基础 GPIO 输出
```c
// main.c - LED 闪烁
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_GPIO GPIO_NUM_48  // ESP32-S3-WROOM-1 LED (或 GPIO_2/45/46)

void app_main(void)
{
    // 配置 GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1 << LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    // 闪烁
    while (1)
    {
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
```

### 1.2 GPIO 输入（按键检测）
```c
// main.c - 按键控制 LED
#include "driver/gpio.h"
#include "esp_log.h"

#define BUTTON_GPIO GPIO_NUM_0   // BOOT 按键
#define LED_GPIO    GPIO_NUM_48

static const char *TAG = "GPIO";

// 中断处理函数
void IRAM_ATTR button_isr(void *arg)
{
    // 注意：ISR 中不要执行耗时操作
    // 使用队列或事件标志通知任务
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // xEventGroupSetBitsFromISR(event_group, BIT_0, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void app_main(void)
{
    // 配置 LED
    gpio_config_t led_conf = {
        .pin_bit_mask = (1 << LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&led_conf);

    // 配置按键（上拉输入）
    gpio_config_t button_conf = {
        .pin_bit_mask = (1 << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_POSEDGE,  // 上升沿中断
    };
    gpio_config(&button_conf);

    // 配置中断
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_GPIO, button_isr, NULL);

    // 轮询方式
    int last_state = 1;  // 上拉，初始为高
    while (1)
    {
        int state = gpio_get_level(BUTTON_GPIO);
        if (state != last_state)
        {
            ESP_LOGI(TAG, "Button: %d", state);
            last_state = state;
            
            if (state == 0)  // 按下
            {
                gpio_set_level(LED_GPIO, 1);
            }
            else  // 释放
            {
                gpio_set_level(LED_GPIO, 0);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
```

### 1.3 GPIO 扩展（PCF8574 I2C I/O 扩展器）
```c
#include "driver/i2c.h"

#define PCF8574_ADDR 0x20  // ADDR 引脚决定

void pcf8574_init(void)
{
    // 配置 I2C
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GPIO_NUM_6,
        .scl_io_num = GPIO_NUM_5,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    i2c_param_config(I2C_NUM_0, &conf);
    i2c_driver_install(I2C_NUM_0, 0, 0, 0);
}

void pcf8574_write(uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (PCF8574_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000);
    i2c_cmd_delete(cmd);
}
```

---

## 第二节：UART 开发

### 2.1 UART 基础收发
```c
#include "driver/uart.h"
#include "esp_log.h"

#define UART_NUM   UART_NUM_1
#define TX_PIN     GPIO_NUM_17
#define RX_PIN     GPIO_NUM_18
#define BAUD_RATE  115200
#define BUF_SIZE   1024

static const char *TAG = "UART";

void uart_init(void)
{
    uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
        .source_clk = UART_SCLK_DEFAULT,
    };
    
    uart_driver_install(UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void uart_example(void)
{
    uart_init();
    
    // 发送数据
    const char *msg = "Hello UART!\r\n";
    uart_write_bytes(UART_NUM, msg, strlen(msg));
    
    // 接收数据
    uint8_t data[256];
    int len = uart_read_bytes(UART_NUM, data, sizeof(data), 1000);
    if (len > 0)
    {
        data[len] = '\0';
        ESP_LOGI(TAG, "Received: %s", data);
    }
}
```

### 2.2 UART 中断 + 环形缓冲
```c
// 使用 UART 的 event queue 实现异步接收
QueueHandle_t uart_event_queue;

void uart_event_task(void *param)
{
    uart_event_t event;
    uint8_t data[BUF_SIZE];
    
    while (1)
    {
        // 等待 UART 事件
        if (xQueueReceive(uart_event_queue, &event, pdMS_TO_TICKS(1000)))
        {
            switch (event.type)
            {
                case UART_DATA:
                    // 读取数据
                    int len = uart_read_bytes(UART_NUM, data, 
                               event.size, 20);
                    ESP_LOGI(TAG, "Received %d bytes", len);
                    // 处理数据...
                    break;
                    
                case UART_FIFO_OVF:
                    // FIFO 溢出
                    uart_flush_input(UART_NUM);
                    break;
                    
                case UART_BUFFER_FULL:
                    // 缓冲区满
                    uart_flush_input(UART_NUM);
                    break;
                    
                case UART_BREAK:
                    ESP_LOGI(TAG, "UART break detected");
                    break;
            }
        }
    }
}

void uart_init_with_events(void)
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    
    // 安装驱动，使用事件队列
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 
                        20, &uart_event_queue, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, TX_PIN, RX_PIN, 
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    
    // 启动事件处理任务
    xTaskCreate(uart_event_task, "uart_task", 4096, 
                NULL, 5, NULL);
}
```

### 2.3 UART 蓝牙透传（BT SPP）
```c
// ESP32-S3 BLE 蓝牙串口透传
#include "esp_bt.h"
#include "esp_hidh.h"
#include "esp_bt_main.h"
#include "gatts_demo.h"  // GATT 服务示例

// 或使用 BLE SPP 示例
// ESP-IDF 路径: examples/bluetooth/bluedroid/bt_spp_initiator

void ble_init(void)
{
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_bt_controller_init(&bt_cfg);
    esp_bt_controller_enable(ESP_BT_MODE_BLE);
    
    esp_bluedroid_init();
    esp_bluedroid_enable();
    
    // GATT 服务注册
    // 参考 esp-idf/examples/bluetooth/bluedroid/gatt_server
}
```

---

## 第三节：SPI 开发

### 3.1 SPI 基础（OLED 显示屏）
```c
#include "driver/spi_master.h"
#include "driver/gpio.h"

#define SPI_HOST    SPI2_HOST
#define PIN_MOSI    GPIO_NUM_7
#define PIN_MISO    GPIO_NUM_8  // 不使用
#define PIN_SCK     GPIO_NUM_9
#define PIN_CS      GPIO_NUM_10
#define PIN_DC      GPIO_NUM_11  // 数据/命令选择
#define PIN_RST     GPIO_NUM_12

#define OLED_WIDTH  128
#define OLED_HEIGHT 64

static spi_device_handle_t oled_handle;

void oled_init(void)
{
    // 配置 GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1 << PIN_DC) | (1 << PIN_RST),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io_conf);
    
    // 复位 OLED
    gpio_set_level(PIN_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(PIN_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // 配置 SPI
    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = PIN_MISO,
        .sclk_io_num = PIN_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 128,
    };
    spi_bus_initialize(SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 10 * 1000 * 1000,  // 10MHz
        .mode = 0,
        .spics_io_num = PIN_CS,
        .queue_size = 7,
    };
    spi_bus_add_device(SPI_HOST, &devcfg, &oled_handle);
    
    // 初始化 OLED (SSD1306)
    // 发送初始化命令序列
    oled_cmd(0xAE);  // 关闭显示
    oled_cmd(0xD5);  // 显示时钟
    oled_cmd(0x80);  // 设置时钟分频
    oled_cmd(0xA8);  // 设置多路复用
    oled_cmd(0x3F);  // 1/64 duty
    // ... 更多初始化命令
    oled_cmd(0xAF);  // 开启显示
}

void oled_cmd(uint8_t cmd)
{
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8;
    t.tx_buffer = &cmd;
    gpio_set_level(PIN_DC, 0);  // 命令模式
    spi_device_polling_transmit(oled_handle, &t);
}

void oled_data(uint8_t *data, int len)
{
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = len * 8;
    t.tx_buffer = data;
    gpio_set_level(PIN_DC, 1);  // 数据模式
    spi_device_polling_transmit(oled_handle, &t);
}
```

### 3.2 SPI DMA 高速传输
```c
// 使用 DMA 实现高速 SPI 传输
void spi_dma_example(void)
{
    uint8_t *tx_buf = heap_caps_malloc(4096, DMA_CAP_DMA);
    uint8_t *rx_buf = heap_caps_malloc(4096, DMA_CAP_DMA);
    
    // 使用 DMA 传输
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 4096 * 8;  // 32768 bits
    t.tx_buffer = tx_buf;
    t.rx_buffer = rx_buf;
    
    // 队列传输
    spi_device_queue_trans(oled_handle, &t, portMAX_DELAY);
    
    // 等待完成
    spi_transaction_t *ret_t;
    spi_device_get_trans_result(oled_handle, &ret_t, portMAX_DELAY);
    
    free(tx_buf);
    free(rx_buf);
}
```

### 3.3 I2S 接口（音频输入）
```c
#include "driver/i2s_std.h"

#define I2S_NUM I2S_NUM_0
#define BCLK_PIN GPIO_NUM_41
#define LRCLK_PIN GPIO_NUM_42
#define DOUT_PIN GPIO_NUM_2
#define DIN_PIN GPIO_NUM_41

void i2s_init(void)
{
    i2s_chan_handle_t rx_handle;
    
    i2s_chan_config_t chan_cfg = {
        .clk_slot = I2S_CHANNEL_SLOT_0,
        .role = I2S_ROLE_MASTER,
        .dma_desc_num = 6,
        .dma_frame_num = 240,
    };
    i2s_new_channel(&chan_cfg, NULL, &rx_handle);
    
    i2s_std_config_t std_cfg = {
        .clk_cfg = {
            .clk_mode = I2S_CLK_MODE_STANDARD,
            .bclk_div = 4,
            .sclk_div = 100,
        },
        .slot_cfg = {
            .data_bit_width = I2S_DATA_BIT_WIDTH_16BIT,
            .slot_bit_width = I2S_SLOT_BIT_WIDTH_AUTO,
            .slot_mode = I2S_SLOT_MODE_STEREO,
            .slot_mask = I2S_SSLOT_MASK_BOTH,
            .msb_format = I2S_DATA_MSB_FIRST,
        },
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = BCLK_PIN,
            .lrclk = LRCLK_PIN,
            .dout = I2S_GPIO_UNUSED,
            .din = DIN_PIN,
        },
    };
    i2s_channel_init_std_mode(rx_handle, &std_cfg);
    i2s_channel_enable(rx_handle);
}
```

---

## 第四节：I2C 开发

### 4.1 I2C 扫描与基础
```c
#include "driver/i2c.h"

#define I2C_NUM   I2C_NUM_0
#define SDA_PIN   GPIO_NUM_6
#define SCL_PIN   GPIO_NUM_5
#define I2C_SPEED 400000

void i2c_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_SPEED,
    };
    i2c_param_config(I2C_NUM, &conf);
    i2c_driver_install(I2C_NUM, 0, 0, 0);
}

// 扫描 I2C 设备
void i2c_scan(void)
{
    esp_log_level_set("I2C", ESP_LOG_INFO);
    
    for (uint8_t addr = 0; addr < 127; addr++)
    {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        
        esp_err_t ret = i2c_master_cmd_begin(I2C_NUM, cmd, pdMS_TO_TICKS(10));
        i2c_cmd_delete(cmd);
        
        if (ret == ESP_OK)
        {
            ESP_LOGI("I2C", "Found device at 0x%02X", addr);
        }
    }
}
```

### 4.2 SHT30 温湿度传感器
```c
#define SHT30_ADDR 0x44  // SHT30 I2C 地址

typedef struct {
    float temperature;
    float humidity;
} sht30_data_t;

esp_err_t sht30_read(sht30_data_t *data)
{
    uint8_t cmd[2] = {0x2C, 0x06};  // 单次测量，高重复性
    uint8_t raw[6];
    
    // 发送命令
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SHT30_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd_handle, cmd, 2, true);
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_NUM, cmd_handle, pdMS_TO_TICKS(10));
    i2c_cmd_delete(cmd_handle);
    
    // 等待测量完成
    vTaskDelay(pdMS_TO_TICKS(20));
    
    // 读取数据
    cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SHT30_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd_handle, raw, 6, false);
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_NUM, cmd_handle, pdMS_TO_TICKS(10));
    i2c_cmd_delete(cmd_handle);
    
    // 转换数据
    uint16_t temp_raw = (raw[0] << 8) | raw[1];
    uint16_t hum_raw = (raw[3] << 8) | raw[4];
    
    data->temperature = -45.0f + 175.0f * ((float)temp_raw / 65535.0f);
    data->humidity = -6.0f + 125.0f * ((float)hum_raw / 65535.0f);
    
    return ESP_OK;
}
```

### 4.3 BH1750 光照传感器
```c
#define BH1750_ADDR 0x23  // BH1750 I2C 地址

float bh1750_read_lux(void)
{
    // 发送测量命令
    uint8_t cmd = 0x20;  // 单次测量，高分辨率
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (BH1750_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd_handle, cmd, true);
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_NUM, cmd_handle, pdMS_TO_TICKS(10));
    i2c_cmd_delete(cmd_handle);
    
    vTaskDelay(pdMS_TO_TICKS(20));
    
    // 读取结果
    uint8_t raw[2];
    cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (BH1750_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd_handle, raw, 2, false);
    i2c_master_stop(cmd_handle);
    i2c_master_cmd_begin(I2C_NUM, cmd_handle, pdMS_TO_TICKS(10));
    i2c_cmd_delete(cmd_handle);
    
    return (raw[0] << 8) | raw[1];
}
```

---

## 第五节：ADC 开发

### 5.1 ADC 单次采样
```c
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"

#define ADC_CHANNEL ADC_CHANNEL_0  // GPIO1

static adc_oneshot_unit_handle_t adc1_handle;
static adc_cali_handle_t adc1_cali_handle;

void adc_init(void)
{
    // 配置 ADC
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&init_cfg, &adc1_handle);
    
    // 配置通道
    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = ADC_ATTEN_DB_12,  // 0 ~ 3.3V
        .bitwidth = ADC_BITWIDTH_12,
    };
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL, &chan_cfg);
    
    // 初始化校准
    adc_cali_handle_t cali_handle;
    adc_cali_scheme_oneshot_config_t cali_cfg = {
        .adc_unit = ADC_UNIT_1,
    };
    adc_cali_create_scheme_oneshot(&cali_cfg, &cali_handle);
    adc1_cali_handle = cali_handle;
}

float adc_read_voltage(void)
{
    int raw;
    adc_oneshot_read(adc1_handle, ADC_CHANNEL, &raw);
    
    // 使用校准值转换为电压
    int voltage;
    adc_cali_raw_to_voltage_onehot(adc1_cali_handle, raw, &voltage);
    
    return voltage / 1000.0f;  // mV → V
}
```

### 5.2 ADC 连续采样 + DMA
```c
#include "esp_adc/adc_continuous.h"

#define ADC_CHANLS_MASK (1 << ADC_CHANNEL_0) | (1 << ADC_CHANNEL_1)

static adc_continuous_handle_t adc_cont_handle;

void adc_continuous_init(void)
{
    adc_continuous_handle_cfg_t handle_cfg = {
        .max_store_buf_size = 1024,
        .conv_mode = ADC_CONV_MODE_CONTINUOUS,
        .conv_frame_size = 100,  // 每帧 100 次采样
    };
    adc_continuous_new_handle(&handle_cfg, &adc_cont_handle);
    
    adc_continuous_config_t cont_cfg = {
        .adc_mask = ADC_CHANLS_MASK,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
        .sample_freq_hz = 20000,  // 20kHz
        .conv_mode = ADC_CONV_MODE_CONTINUOUS,
    };
    adc_continuous_config(adc_cont_handle, &cont_cfg);
}

void adc_continuous_example(void)
{
    adc_continuous_init();
    adc_continuous_start(adc_cont_handle);
    
    // 读取数据
    uint16_t data[100];
    while (1)
    {
        int len = adc_continuous_read(adc_cont_handle, 
                    (uint8_t*)data, sizeof(data), pdMS_TO_TICKS(100));
        if (len > 0)
        {
            // 处理数据
            float avg = 0;
            for (int i = 0; i < len / 2; i++)
                avg += data[i];
            avg /= (len / 2);
            ESP_LOGI("ADC", "Average: %.1f", avg);
        }
    }
}
```

---

## 第六节：LEDC（多路 PWM）

### 6.1 LEDC 基础
```c
#include "driver/ledc.h"

#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_GPIO    GPIO_NUM_48
#define LEDC_FREQ_HZ 5000   // 5kHz
#define LEDC_RESOLUTION LEDC_TIMER_12_BIT  // 12-bit: 4096 级

void ledc_init(void)
{
    // 配置定时器
    ledc_timer_config_t timer_cfg = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_RESOLUTION,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = LEDC_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer_cfg);
    
    // 配置通道
    ledc_channel_config_t chan_cfg = {
        .gpio_num = LEDC_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };
    ledc_channel_config(&chan_cfg);
}

void ledc_set_duty(int duty_percent)
{
    int max_duty = (1 << LEDC_RESOLUTION) - 1;
    int duty = duty_percent * max_duty / 100;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL);
}
```

### 6.2 RGB LED 呼吸灯
```c
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE  LEDC_LOW_SPEED_MODE
#define LEDC_RES   LEDC_TIMER_12_BIT
#define LEDC_FREQ  5000

#define RED_PIN    GPIO_NUM_47
#define GREEN_PIN  GPIO_NUM_48
#define BLUE_PIN   GPIO_NUM_45

#define RED_CH     LEDC_CHANNEL_0
#define GREEN_CH   LEDC_CHANNEL_1
#define BLUE_CH    LEDC_CHANNEL_2

void rgb_led_init(void)
{
    ledc_timer_config_t timer_cfg = {
        .speed_mode = LEDC_MODE,
        .duty_resolution = LEDC_RES,
        .timer_num = LEDC_TIMER,
        .freq_hz = LEDC_FREQ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer_cfg);
    
    ledc_channel_config_t red_cfg = {
        .gpio_num = RED_PIN,
        .speed_mode = LEDC_MODE,
        .channel = RED_CH,
        .timer_sel = LEDC_TIMER,
        .duty = 0,
        .hpoint = 0,
    };
    ledc_channel_config(&red_cfg);
    
    // 同样配置 GREEN_CH 和 BLUE_CH
}

void rgb_led_set(uint8_t r, uint8_t g, uint8_t b)
{
    int max = (1 << LEDC_RES) - 1;
    ledc_set_duty(LEDC_MODE, RED_CH, r * max / 255);
    ledc_set_duty(LEDC_MODE, GREEN_CH, g * max / 255);
    ledc_set_duty(LEDC_MODE, BLUE_CH, b * max / 255);
    ledc_update_duty(LEDC_MODE, RED_CH);
    ledc_update_duty(LEDC_MODE, GREEN_CH);
    ledc_update_duty(LEDC_MODE, BLUE_CH);
}

void breathing_effect(void)
{
    while (1)
    {
        for (int i = 0; i < 256; i++)
        {
            rgb_led_set(i, 0, 0);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        for (int i = 255; i >= 0; i--)
        {
            rgb_led_set(i, 0, 0);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}
```

---

## 第七节：RMT（远程控制发射器）

### 7.1 IR LED 发射（NEC 协议）
```c
#include "driver/rmt_tx.h"

#define RMT_TX_GPIO GPIO_NUM_38
#define RMT_RESOLUTION 1000000  // 1MHz (1us 精度)

static rmt_channel_handle_t rmt_tx_handle;

void rmt_tx_init(void)
{
    rmt_channel_config_t chan_cfg = {
        .gpio_num = RMT_TX_GPIO,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = RMT_RESOLUTION,
        .mem_block_symbols = 64,
        .task_stack_id = 0,
    };
    rmt_new_tx_channel(&chan_cfg, &rmt_tx_handle);
    rmt_enable(rmt_tx_handle);
}

// NEC 协议时序
// 引导码: 9ms 高 + 4.5ms 低
// "1": 560us 高 + 1690us 低
// "0": 560us 高 + 560us 低
void nec_send_code(uint8_t addr, uint8_t cmd)
{
    // 构建 RMT 传输数据
    // 引导码
    rmt_symbol_word_t symbols[68];  // NEC 总共需要 68 个符号
    int idx = 0;
    
    // 引导码: 9ms 高 + 4.5ms 低
    symbols[idx++] = (rmt_symbol_word_t) {
        .level0 = 1, .duration0 = 9000,
        .level1 = 0, .duration1 = 4500,
    };
    
    // 地址 + 反相地址 + 命令 + 反相命令
    uint8_t data[4] = {addr, ~addr, cmd, ~cmd};
    for (int i = 0; i < 4; i++)
    {
        for (int bit = 0; bit < 8; bit++)
        {
            uint8_t bit_val = (data[i] >> (7 - bit)) & 1;
            uint16_t high_time = 560;
            uint16_t low_time = bit_val ? 1690 : 560;
            
            symbols[idx++] = (rmt_symbol_word_t) {
                .level0 = 1, .duration0 = high_time,
                .level1 = 0, .duration1 = low_time,
            };
        }
    }
    
    // 停止码: 560us 高
    symbols[idx++] = (rmt_symbol_word_t) {
        .level0 = 1, .duration0 = 560,
        .level1 = 0, .duration1 = 0,
    };
    
    // 发送
    rmt_transmit_config_t tx_cfg = {
        .loop_count = 0,
    };
    rmt_tx(rmt_tx_handle, symbols, idx, &tx_cfg);
}
```

### 7.2 RMT 接收（NEC 解码）
```c
#include "driver/rmt_rx.h"

#define RMT_RX_GPIO GPIO_NUM_39
#define RMT_RESOLUTION 1000000

static rmt_channel_handle_t rmt_rx_handle;

void rmt_rx_init(void)
{
    rmt_channel_config_t chan_cfg = {
        .gpio_num = RMT_RX_GPIO,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = RMT_RESOLUTION,
        .mem_block_symbols = 64,
        .task_stack_id = 1,
    };
    rmt_new_rx_channel(&chan_cfg, &rmt_rx_handle);
    
    rmt_receive_config_t rx_cfg = {
        .loop_count = 0,
        .no_echo = true,
    };
    rmt_enable(rmt_rx_handle);
    rmt_receive(rmt_rx_handle, &rx_cfg);
}

// 在 task 中处理接收
void rmt_rx_task(void *param)
{
    rmt_symbol_word_t symbols[64];
    size_t received = 0;
    
    while (1)
    {
        rmt_receive(rmt_rx_handle, symbols, sizeof(symbols), 
                    pdMS_TO_TICKS(100));
        
        // 分析 NEC 数据
        int idx = 0;
        // 检查引导码
        if (symbols[idx].level0 == 1 && symbols[idx].duration0 > 8000)
        {
            idx++;  // 跳过引导码
            
            uint8_t addr = 0, cmd = 0;
            // 解析 32 位数据
            for (int byte = 0; byte < 4; byte++)
            {
                for (int bit = 0; bit < 8; bit++)
                {
                    uint16_t low_time = symbols[idx].duration1;
                    uint8_t bit_val = (low_time > 1000) ? 1 : 0;
                    
                    if (byte == 0) addr |= (bit_val << (7 - bit));
                    if (byte == 2) cmd |= (bit_val << (7 - bit));
                    idx++;
                }
            }
            
            ESP_LOGI("RMT", "NEC: addr=0x%02X cmd=0x%02X", addr, cmd);
        }
    }
}
```

---

## 第八节：PCNT（脉冲计数）与触摸

### 8.1 PCNT 脉冲计数
```c
#include "driver/pulse_cnt.h"

#define PCNT_GPIO GPIO_NUM_20

void pcnt_init(void)
{
    pcnt_unit_config_t unit_cfg = {
        .high_limit = 32767,
        .low_limit = -32768,
    };
    pcnt_unit_handle_t unit;
    pcnt_new_unit(&unit_cfg, &unit);
    
    pcnt_channel_config_t chan_cfg = {
        .edge_gpio_num = PCNT_GPIO,
        .level_gpio_num = -1,  // 不使用硬件控制
        .count_higher_edge = PCNT_CHANNEL_EDGE_INCREASE,
        .count_lower_edge = PCNT_CHANNEL_EDGE_DECREASE,
    };
    pcnt_channel_handle_t chan;
    pcnt_new_channel(unit, &chan_cfg, &chan);
    
    pcnt_set_filter_value(unit, 1000);  // 1000ns 滤波
    pcnt_unit_enable(unit);
}
```

### 8.2 电容触摸
```c
#include "driver/touch_sensor.h"

#define TOUCH_CHANNEL TOUCH_PAD_NUM1  // GPIO1

void touch_init(void)
{
    touch_pad_init();
    touch_pad_config(TOUCH_CHANNEL);
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    touch_pad_fsm_start();
}

uint32_t touch_read(void)
{
    uint32_t value = 0;
    touch_pad_read_raw_data(TOUCH_CHANNEL, &value);
    return value;
}
```

---

## 学习练习

### 练习 1：基础外设
- [ ] GPIO 输入/输出
- [ ] UART 透传（中断方式）
- [ ] SPI OLED 显示
- [ ] I2C 读取 SHT30/BH1750

### 练习 2：进阶外设
- [ ] ADC 连续采样
- [ ] LEDC RGB 呼吸灯
- [ ] RMT IR 发射/接收
- [ ] PCNT 脉冲计数

### 练习 3：综合应用
- [ ] 电子指南针（磁力计 + I2C）
- [ ] 简易示波器（ADC + SPI LCD）
- [ ] 红外遥控发射器
- [ ] 触摸按键控制

---

## 本章小结
1. ✅ 掌握 GPIO 输入/输出/中断
2. ✅ 掌握 UART 基础 + 事件驱动
3. ✅ 掌握 SPI + OLED 显示
4. ✅ 掌握 I2C 扫描与传感器读取
5. ✅ 掌握 ADC 单次/连续采样
6. ✅ 掌握 LEDC 多路 PWM
7. ✅ 掌握 RMT 红外通信
8. ✅ 了解 PCNT 和触摸传感器

**下一章**：ESP32-S3 Wi-Fi、BLE、FreeRTOS 开发
