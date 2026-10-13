# ESP32-S3 ESP-IDF API 速查表

## 一、GPIO API

### 头文件
```c
#include "driver/gpio.h"
```

### 初始化结构体
```c
gpio_config_t io_conf = {
    .pin_bit_mask = (1ULL << GPIO_NUM_2),  // 引脚位掩码
    .mode = GPIO_MODE_OUTPUT,               // 模式
    .pull_up_en = GPIO_PULLUP_DISABLE,      // 上拉
    .pull_down_en = GPIO_PULLDOWN_DISABLE,  // 下拉
    .intr_type = GPIO_INTR_DISABLE,        // 中断类型
};
```

### 模式选项
| 模式 | 说明 |
|------|------|
| `GPIO_MODE_INPUT` | 输入 |
| `GPIO_MODE_OUTPUT` | 输出 |
| `GPIO_MODE_OUTPUT_OD` | 开漏输出 |
| `GPIO_MODE_INPUT_OUTPUT_OD` | 输入+开漏输出 |
| `GPIO_MODE_INPUT_OUTPUT` | 输入+推挽输出 |

### 中断类型
| 类型 | 说明 |
|------|------|
| `GPIO_INTR_DISABLE` | 无中断 |
| `GPIO_INTR_RISING` | 上升沿 |
| `GPIO_INTR_FALLING` | 下降沿 |
| `GPIO_INTR_BOTH` | 双边沿 |

### 常用函数
```c
gpio_config(&io_conf);                    // 配置GPIO
gpio_set_level(gpio_num, level);          // 设置输出电平
gpio_get_level(gpio_num);                 // 读取输入电平
gpio_set_direction(gpio_num, mode);       // 动态切换方向
gpio_install_isr_service(flags);          // 安装中断服务
gpio_isr_handler_add(gpio_num, isr, arg); // 注册中断处理
gpio_wakeup_enable(gpio_num, intr_type);  // 启用深度睡眠唤醒
```

### 引脚定义
```c
GPIO_NUM_0, GPIO_NUM_1, ..., GPIO_NUM_48  // ESP32-S3
```

---

## 二、UART API

### 头文件
```c
#include "driver/uart.h"
```

### 初始化结构体
```c
uart_config_t uart_config = {
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_DEFAULT,
};
```

### 波特率选项
`1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600`

### 常用函数
```c
uart_driver_install(port_num, rx_buf_size, tx_buf_size, queue_size, queue, flags);
uart_param_config(port_num, &uart_config);
uart_set_pin(port_num, tx_pin, rx_pin, rts_pin, cts_pin);
uart_write_bytes(port_num, data, len);           // 阻塞发送
uart_write_bytes_bytes(port_num, data, len);     // 非阻塞发送
uart_read_bytes(port_num, data, len, timeout);   // 读取
uart_flush_input(port_num);                      // 清空接收缓冲
uart_flush_output(port_num);                     // 等待发送完成
uart_get_eventqueue_handle(port_num);             // 获取事件队列
uart_disable_rx_intr(port_num);                  // 禁用接收中断
uart_enable_pattern_det_intr(port_num, pattern, ...); // 模式检测中断
```

### 端口选择
```c
UART_NUM_0, UART_NUM_1, UART_NUM_2
```

---

## 三、I2C API

### 头文件
```c
#include "driver/i2c.h"
```

### 初始化结构体
```c
i2c_config_t conf = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = GPIO_NUM_8,
    .scl_io_num = GPIO_NUM_9,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = 400000,  // 400kHz
};
```

### 常用函数
```c
i2c_param_config(port_num, &conf);
i2c_driver_install(port_num, mode, rx_buf_size, tx_buf_size, flags);

// 主模式命令构建
i2c_cmd_link_create();
i2c_master_start(cmd);
i2c_master_write_byte(cmd, addr_w, ack);
i2c_master_write_byte(cmd, reg, ack);
i2c_master_read_byte(cmd, data, nack);
i2c_master_read(cmd, data, len-1, ack);
i2c_master_stop(cmd);
i2c_master_cmd_begin(port_num, cmd, timeout);
i2c_cmd_link_delete(cmd);

// 便捷函数 (ESP-IDF v5.0+)
i2c_master_transmit(port_num, data, len, timeout);
i2c_master_receive(port_num, data, len, timeout);
i2c_master_transmit_receive(port_num, tx, txlen, rx, rxlen, timeout);
```

### 速度模式
| 模式 | 速度 |
|------|------|
| 标准模式 | 100kHz |
| 快速模式 | 400kHz |
| 高速模式 | 1MHz |

---

## 四、SPI API

### 头文件
```c
#include "driver/spi_master.h"
#include "driver/spi_slave.h"
```

### 主机初始化
```c
spi_bus_config_t buscfg = {
    .mosi_io_num = GPIO_NUM_13,
    .miso_io_num = GPIO_NUM_12,
    .sclk_io_num = GPIO_NUM_11,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = 4096,
};
spi_bus_initialize(host, &buscfg, dma_channel);

spi_device_interface_config_t devcfg = {
    .clock_speed_hz = 10*1000*1000,
    .mode = 0,
    .spics_io_num = GPIO_NUM_10,
    .queue_size = 7,
};
spi_bus_add_device(host, &devcfg, &dev_handle);
```

### 常用函数
```c
spi_device_polling_transmit(dev, &t);   // 轮询方式
spi_device_transmit(dev, &t);           // 中断方式
spi_device_queue_trans(dev, &t, timeout); // 队列方式
spi_device_polling_fetch_trans(dev, &t, timeout); // 获取结果

// 传输结构体
spi_transaction_t t = {
    .length = 8,                     // 传输位数
    .tx_buffer = &tx_data,
    .rx_buffer = &rx_data,
};
```

### SPI 模式
| 模式 | CPOL | CPHA |
|------|------|------|
| 0 | 0 | 0 |
| 1 | 0 | 1 |
| 2 | 1 | 0 |
| 3 | 1 | 1 |

---

## 五、ADC API

### 头文件
```c
#include "driver/adc.h"
```

### 初始化
```c
adc1_config_width(ADC_WIDTH_BIT_12);                    // 12位分辨率
adc1_config_channel_atten(ADC_CHANNEL_0, ADC_ATTEN_DB_12); // 12dB衰减
```

### 分辨率选项
| 选项 | 说明 |
|------|------|
| `ADC_WIDTH_BIT_9` | 9位 (0~511) |
| `ADC_WIDTH_BIT_10` | 10位 (0~1023) |
| `ADC_WIDTH_BIT_11` | 11位 (0~2047) |
| `ADC_WIDTH_BIT_12` | 12位 (0~4095) |

### 衰减选项
| 选项 | 电压范围 |
|------|----------|
| `ADC_ATTEN_DB_0` | 0 ~ 1.1V |
| `ADC_ATTEN_DB_2_5` | 0 ~ 1.5V |
| `ADC_ATTEN_DB_6` | 0 ~ 2.2V |
| `ADC_ATTEN_DB_12` | 0 ~ 3.3V |

### 常用函数
```c
adc1_get_raw(channel);                 // 读取原始值
adc_oneshot_new_unit(adc_unit, &config, &handle);
adc_oneshot_config_channel(handle, channel, atten);
adc_oneshot_read(handle, channel, &out_raw);
adc_oneshot_del_unit(handle);
```

### 通道映射
| 通道 | GPIO |
|------|------|
| ADC_CHANNEL_0 | GPIO1 |
| ADC_CHANNEL_1 | GPIO2 |
| ADC_CHANNEL_2 | GPIO3 |
| ADC_CHANNEL_3 | GPIO4 |
| ADC_CHANNEL_4 | GPIO5 |
| ADC_CHANNEL_5 | GPIO6 |
| ADC_CHANNEL_6 | GPIO7 |
| ADC_CHANNEL_7 | GPIO8 |
| ADC_CHANNEL_8 | GPIO9 |
| ADC_CHANNEL_9 | GPIO10 |
| ADC_CHANNEL_10 | GPIO11 |
| ADC_CHANNEL_11 | GPIO12 |
| ADC_CHANNEL_12 | GPIO13 |
| ADC_CHANNEL_13 | GPIO14 |
| ADC_CHANNEL_14 | GPIO15 |
| ADC_CHANNEL_15 | GPIO16 |
| ADC_CHANNEL_16 | GPIO17 |
| ADC_CHANNEL_17 | GPIO18 |
| ADC_CHANNEL_18 | GPIO19 |
| ADC_CHANNEL_19 | GPIO20 |

---

## 六、LEDC (PWM) API

### 头文件
```c
#include "driver/ledc.h"
```

### 初始化
```c
ledc_timer_config_t timer_conf = {
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .duty_resolution = LEDC_TIMER_10_BIT,  // 1024级
    .timer_num = LEDC_TIMER_0,
    .freq_hz = 5000,                        // 5kHz
    .clk_cfg = LEDC_AUTO_CLK,
};
ledc_timer_config(&timer_conf);

ledc_channel_config_t chan_conf = {
    .gpio_num = GPIO_NUM_4,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .channel = LEDC_CHANNEL_0,
    .timer_sel = LEDC_TIMER_0,
    .duty = 0,
    .hpoint = 0,
};
ledc_channel_config(&chan_conf);
```

### 常用函数
```c
ledc_set_duty(speed_mode, channel, duty);    // 设置占空比
ledc_update_duty(speed_mode, channel);        // 更新生效
ledc_get_duty(speed_mode, channel);           // 获取当前占空比
ledc_stop(speed_mode, channel, idle_level);   // 停止PWM
ledc_set_freq(speed_mode, timer, freq_hz);     // 动态调频
```

### 速度模式
| 模式 | 频率范围 |
|------|----------|
| `LEDC_LOW_SPEED_MODE` | 1Hz ~ 40MHz |
| `LEDC_HIGH_SPEED_MODE` | 1Hz ~ 40MHz |

### 分辨率与最大频率
| 分辨率 | 最大频率(80MHz时钟) |
|--------|---------------------|
| 1-bit | 80MHz |
| 2-bit | 40MHz |
| 5-bit | 2.5MHz |
| 10-bit | 78kHz |
| 13-bit | 9.75kHz |

---

## 七、Touch API

### 头文件
```c
#include "driver/touch_pad.h"
```

### 初始化
```c
touch_pad_init();
touch_pad_config(TOUCH_PAD_NUM1);  // GPIO14
touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
touch_pad_fsm_start();
```

### 常用函数
```c
touch_pad_read_raw_data(pad_num, &value);    // 读取原始值
touch_pad_read(pad_num, &value);              // 读取校准值
touch_pad_set_trigger_source(TOUCH_TRIGGER_SOURCE); // 设置触发源
touch_pad_set_period(period);                 // 设置测量周期
```

### Touch 通道映射
| 通道 | GPIO |
|------|------|
| TOUCH_PAD_NUM1 | GPIO14 |
| TOUCH_PAD_NUM2 | GPIO15 |
| TOUCH_PAD_NUM3 | GPIO16 |
| TOUCH_PAD_NUM4 | GPIO17 |
| TOUCH_PAD_NUM5 | GPIO18 |
| TOUCH_PAD_NUM6 | GPIO19 |
| TOUCH_PAD_NUM7 | GPIO20 |
| TOUCH_PAD_NUM8 | GPIO21 |
| TOUCH_PAD_NUM9 | GPIO5 |
| TOUCH_PAD_NUM10 | GPIO6 |
| TOUCH_PAD_NUM11 | GPIO7 |
| TOUCH_PAD_NUM12 | GPIO8 |
| TOUCH_PAD_NUM13 | GPIO9 |
| TOUCH_PAD_NUM14 | GPIO10 |

---

## 八、Wi-Fi API

### 头文件
```c
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
```

### STA 模式连接
```c
ESP_ERROR_CHECK(nvs_flash_init());
esp_netif_init();
ESP_ERROR_CHECK(esp_event_loop_create_default());
esp_netif_create_default_wifi_sta();

wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
ESP_ERROR_CHECK(esp_wifi_init(&cfg));

wifi_config_t wifi_config = {
    .sta = {
        .ssid = "MyWiFi",
        .password = "password",
    },
};
esp_wifi_set_mode(WIFI_MODE_STA);
esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
esp_wifi_start();
esp_wifi_connect();
```

### AP 模式创建热点
```c
wifi_config_t wifi_config = {
    .ap = {
        .ssid = "ESP32-S3",
        .password = "12345678",
        .max_connection = 4,
        .authmode = WIFI_AUTH_WPA2_PSK,
    },
};
esp_wifi_set_mode(WIFI_MODE_AP);
esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
esp_wifi_start();
```

### 常用函数
```c
esp_wifi_init(&cfg);
esp_wifi_deinit();
esp_wifi_set_mode(mode);           // WIFI_MODE_STA / WIFI_MODE_AP / WIFI_MODE_APSTA
esp_wifi_start();
esp_wifi_stop();
esp_wifi_connect();
esp_wifi_disconnect();
esp_wifi_set_config(interface, &conf);
esp_wifi_get_config(interface, &conf);
esp_wifi_scan_start(&scan_config, block);
esp_wifi_scan_get_ap_num(&number);
esp_wifi_scan_get_ap_records(&number, &ap_records);
esp_wifi_get_mac(ifx, mac);
esp_wifi_get_protocol_mode(&protocol_bitmap);
```

### 事件处理
```c
esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                    wifi_event_handler, NULL, NULL);
esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID,
                                    ip_event_handler, NULL, NULL);
```

---

## 九、NVS API

### 头文件
```c
#include "nvs_flash.h"
#include "nvs.h"
```

### 初始化
```c
esp_err_t ret = nvs_flash_init();
if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    nvs_flash_erase();
    nvs_flash_init();
}
```

### 写入
```c
nvs_handle_t handle;
nvs_open("namespace", NVS_READWRITE, &handle);

nvs_set_i8(handle, "key", value);
nvs_set_u8(handle, "key", value);
nvs_set_i16(handle, "key", value);
nvs_set_u16(handle, "key", value);
nvs_set_i32(handle, "key", value);
nvs_set_u32(handle, "key", value);
nvs_set_i64(handle, "key", value);
nvs_set_u64(handle, "key", value);
nvs_set_str(handle, "key", "string_value");
nvs_set_blob(handle, "key", data, length);

nvs_commit(handle);
nvs_close(handle);
```

### 读取
```c
nvs_open("namespace", NVS_READONLY, &handle);

int8_t i8; nvs_get_i8(handle, "key", &i8);
uint8_t u8; nvs_get_u8(handle, "key", &u8);
int32_t i32; nvs_get_i32(handle, "key", &i32);
char str[64]; size_t len = sizeof(str); nvs_get_str(handle, "key", str, &len);
uint8_t buf[32]; size_t blen = sizeof(buf); nvs_get_blob(handle, "key", buf, &blen);

nvs_close(handle);
```

### 迭代
```c
nvs_iterator_t it;
nvs_entry_find("namespace", NULL, NVS_READWRITE, &it);
nvs_entry_info_t info;
while (nvs_entry_next(&it) == ESP_OK) {
    nvs_entry_info(&it, &info);
    // info.key, info.type
}
nvs_release_iterator(it);
```

---

## 十、FreeRTOS API

### 头文件
```c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
```

### 任务
```c
xTaskCreate(task_func, "name", stack_size, param, priority, &handle);
xTaskCreatePinnedToCore(task_func, "name", stack_size, param, priority, &handle, core_id);
xTaskGetHandle("name");
vTaskDelete(handle);
vTaskDelay(ticks);
vTaskDelayUntil(&last_wake_time, ticks);
xTaskGetTickCount();
```

### 队列
```c
QueueHandle_t queue = xQueueCreate(len, item_size);
xQueueSend(queue, &item, timeout);
xQueueSendToFront(queue, &item, timeout);
xQueueSendToBack(queue, &item, timeout);
xQueueReceive(queue, &item, timeout);
xQueuePeek(queue, &item, timeout);
uxQueueMessagesWaiting(queue);
```

### 信号量/互斥锁
```c
SemaphoreHandle_t sem = xSemaphoreCreateBinary();
SemaphoreHandle_t sem = xSemaphoreCreateCounting(max, initial);
SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
SemaphoreHandle_t rwlock = xSemaphoreCreateRecursiveMutex();

xSemaphoreGive(sem);
xSemaphoreGiveFromISR(sem, &woken);
xSemaphoreTake(sem, timeout);
xSemaphoreTakeRecursive(mutex, timeout);
```

### 事件组
```c
EventGroupHandle_t group = xEventGroupCreate();
EventBits_t bits = xEventGroupClearBits(group, bits_to_clear);
EventBits_t bits = xEventGroupSetBits(group, bits_to_set);
EventBits_t bits = xEventGroupWaitBits(group, bits_to_wait_for, clear_on_exit, wait_for_all, timeout);
```

### 软件定时器
```c
TimerHandle_t timer = xTimerCreate("name", period, auto_reload, timer_id, callback);
xTimerStart(timer, timeout);
xTimerReset(timer, timeout);
xTimerStop(timer, timeout);
xTimerDelete(timer, timeout);
```

---

## 十一、文件系统 API

### SPIFFS
```c
#include "esp_vfs_fat.h"
#include "spiffs.h"

esp_vfs_fat_mount_config_t cfg = {
    .format_if_mount_failed = true,
    .max_files = 5,
    .allocation_unit_size = 16 * 1024,
};
esp_vfs_fat_spiffs_mount("/spiffs", "partition_label", &cfg);

// 文件操作（使用标准 C API）
FILE *f = fopen("/spiffs/file.txt", "w");
fprintf(f, "Hello\n");
fclose(f);

f = fopen("/spiffs/file.txt", "r");
char buf[64];
fgets(buf, sizeof(buf), f);
fclose(f);

struct stat st;
stat("/spiffs/file.txt", &st);
remove("/spiffs/file.txt");
mkdir("/spiffs/newdir", 0755);
```

### LittleFS（推荐 ESP32-S3）
```c
#include "esp_littlefs.h"

esp_vfs_littlefs_conf_t cfg = {
    .base_path = "/littlefs",
    .partition_label = "littlefs",
    .format_if_mount_failed = true,
    .dont_mount = false,
};
esp_vfs_littlefs_register(&cfg);
```

### SD卡 (SDMMC)
```c
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"

sdmmc_host_init();
sdmmc_host_init_slot(SDMMC_HOST_SLOT_1, &slot_config);
sdmmc_card_t *card;
sdmmc_host_get_slot(slot, &card);
sdmmc_card_init(slot, &card);

esp_vfs_fat_sdmmc_mount("/sdcard", card, &mount_config);
```

---

## 十二、系统 API

### 头文件
```c
#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_clk.h"
#include "esp_flash.h"
```

### 常用函数
```c
// 芯片信息
esp_chip_info_t info; esp_chip_info(&info);
esp_clk_cpu_freq();
esp_flash_get_size(NULL);
esp_read_mac(mac, ESP_MAC_WIFI_STA);

// 重启
esp_restart();

// Heap
esp_get_free_heap_size();
esp_get_minimum_free_heap_size();
heap_caps_get_free_size(CAP_ANY);

// 时间
esp_timer_get_time();          // 微秒
esp_timer_get_time() / 1000;   // 毫秒
esp_timer_create(&create_args, &handle);
esp_timer_start_once(handle, timeout);

// 日志
ESP_LOGI(TAG, "message: %d", value);
ESP_LOGW(TAG, "warning");
ESP_LOGE(TAG, "error");
ESP_LOG_BUFFER_HEX(TAG, data, len);
esp_log_level_set("*", ESP_LOG_INFO);
```

### 日志级别
```c
ESP_LOG_NONE    // 不输出
ESP_LOG_ERROR   // 错误
ESP_LOG_WARN    // 警告
ESP_LOG_INFO    // 信息
ESP_LOG_DEBUG   // 调试
ESP_LOG_VERBOSE // 详细
```

---

## 十三、USB CDC API (ESP32-S3 专用)

### 头文件
```c
#include "driver/usb_serial_jtag.h"
```

### 初始化
```c
usb_serial_jtag_config_t usb_cfg = {
    .baud_rate = 115200,
    .data_bits = USB_SERIAL_JTAG_DATA_BITS_8,
    .parity = USB_SERIAL_JTAG_PARITY_DISABLE,
    .stop_bits = USB_SERIAL_JTAG_STOP_BITS_1,
};
usb_serial_jtag_driver_install(&usb_cfg);
```

### 常用函数
```c
usb_serial_jtag_driver_config_uart_port(port_num);
usb_serial_jtag_driver_config_tinyusb_port(port_num);
```

---

## 十四、HTTP Server API

### 头文件
```c
#include "esp_http_server.h"
```

### 创建 HTTP 服务器
```c
httpd_config_t config = HTTPD_DEFAULT_CONFIG();
httpd_handle_t server = httpd_start(&config);

// 注册 URI 处理
httpd_uri_t hello = {
    .uri = "/hello",
    .method = HTTP_GET,
    .handler = hello_get_handler,
};
httpd_register_uri_handler(server, &hello);
```

### 处理函数示例
```c
esp_err_t hello_get_handler(httpd_req_t *req)
{
    const char *resp = "Hello ESP32-S3!";
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}
```

### 常用函数
```c
httpd_start(&config);
httpd_stop(server);
httpd_register_uri_handler(server, &uri);
httpd_unregister_uri_handler(server, uri);
httpd_resp_set_type(req, content_type);
httpd_resp_set_status(req, status);
httpd_resp_send(req, buf, len);
httpd_resp_send_chunk(req, buf, len);
httpd_req_get_url_query_str(req, buf, buf_len);
httpd_req_query_param_name_by_index(req, idx, name, name_len);
httpd_req_query_param_value_by_index(req, idx, val, val_len);
```
