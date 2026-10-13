# ESP32-S3 学习路径 - 第三章：Wi-Fi、BLE、FreeRTOS 开发

## 学习目标
- 掌握 ESP32-S3 Wi-Fi Station/AP 模式
- 实现 BLE GATT Server/Client
- 学会使用 FreeRTOS 多任务、队列、信号量
- 实现 Wi-Fi + BLE 综合应用

---

## 第一节：Wi-Fi 开发

### 1.1 Wi-Fi Station 模式（连接路由器）
```c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define WIFI_SSID      "YourWiFiName"
#define WIFI_PASSWORD  "YourWiFiPassword"

#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAIL_BIT       BIT1

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;

static const char *TAG = "WIFI";

// Wi-Fi 事件处理
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < 5)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retry connecting to AP (%d)", s_retry_num);
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "WiFi disconnected. Retrying...");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    // 初始化 NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        nvs_flash_init();
    }
    
    // 初始化事件循环
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    esp_netif_create_default_wifi_sta();
    
    // 初始化 Wi-Fi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    // 注册事件处理
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                         ESP_EVENT_ANY_ID,
                                                         &wifi_event_handler,
                                                         NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                         IP_EVENT_STA_GOT_IP,
                                                         &wifi_event_handler,
                                                         NULL, NULL));
    
    // 配置 Wi-Fi
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    // 等待连接
    s_wifi_event_group = xEventGroupCreate();
    
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE, pdFALSE, portMAX_DELAY);
    
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "Connected to AP");
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to AP");
    }
}
```

### 1.2 Wi-Fi AP 模式（热点）
```c
void wifi_init_ap(void)
{
    // 初始化（同 Station 模式）
    
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "ESP32-S3-AP",
            .password = "12345678",
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "Wi-Fi AP started. SSID: %s", wifi_config.ap.ssid);
}
```

### 1.3 Wi-Fi + HTTP Server
```c
#include "esp_http_server.h"

httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.max_uri_handlers = 8;
    
    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // 注册 URI 处理器
        httpd_uri_t index_uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = index_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &index_uri);
        
        httpd_uri_t data_uri = {
            .uri = "/data",
            .method = HTTP_GET,
            .handler = data_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &data_uri);
    }
    return server;
}

// HTML 页面
static const char index_html[] =
    "<!DOCTYPE html><html><head>"
    "<meta charset='utf-8'>"
    "<meta http-equiv='refresh' content='2'>"
    "<title>ESP32-S3 Weather</title>"
    "<style>"
    "body {font-family:Arial;display:flex;justify-content:center;align-items:center;height:100vh;margin:0;background:#222;color:#eee;}"
    ".card {text-align:center;padding:40px;background:#333;border-radius:10px;}"
    "h1 {color:#4CAF50;}"
    ".val {font-size:48px;margin:20px 0;}"
    "</style></head><body>"
    "<div class='card'>"
    "<h1>🌤️ Weather Station</h1>"
    "<div class='val' id='temp'>-- °C</div>"
    "<div class='val' id='hum'>-- %</div>"
    "<a href='/data'>Refresh</a>"
    "</div>"
    "<script>"
    "setInterval(() => fetch('/data').then(r => r.json()).then(d => {"
    "document.getElementById('temp').textContent = d.temp + ' °C';"
    "document.getElementById('hum').textContent = d.hum + '%';"
    "}));","
    "</script>"
    "</body></html>";

static esp_err_t index_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, index_html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t data_handler(httpd_req_t *req)
{
    char resp[100];
    snprintf(resp, sizeof(resp), 
             "{\"temp\":%.1f,\"hum\":%.1f}", 
             temperature, humidity);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}
```

### 1.4 MQTT 客户端
```c
#include "mqtt_client.h"

#define MQTT_URI "mqtt://broker.emqx.io:1883"
#define MQTT_TOPIC "esp32s3/data"

static esp_mqtt_client_handle_t mqtt_client;

void mqtt_init(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = MQTT_URI,
        },
        .credentials = {
            .username = NULL,
            .client_id = "esp32s3_" MACSTR,
        },
    };
    
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    
    // 注册事件
    esp_mqtt_client_register_event(mqtt_client, 
        ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    
    esp_mqtt_client_start(mqtt_client);
}

static void mqtt_event_handler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data)
{
    esp_mqtt_event_t *event = (esp_mqtt_event_t *)event_data;
    
    switch (event_id)
    {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI("MQTT", "Connected!");
            esp_mqtt_client_subscribe(mqtt_client, "esp32s3/cmd", 0);
            break;
            
        case MQTT_EVENT_DATA:
            ESP_LOGI("MQTT", "Topic: %s, Data: %s", 
                     event->topic, event->data);
            break;
            
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW("MQTT", "Disconnected. Retrying...");
            break;
    }
}

void mqtt_publish_data(float temp, float hum)
{
    char payload[100];
    snprintf(payload, sizeof(payload), 
             "{\"temp\":%.1f,\"hum\":%.1f}", temp, hum);
    
    esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC, 
                             payload, 0, 0, false);
}
```

---

## 第二节：BLE 开发

### 2.1 BLE GATT Server
```c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "gatts_demo.h"

// GATT 服务定义
// ESP-IDF 示例: examples/bluetooth/bluedroid/gatt_server

// 或使用 ESP-NOW（更简单）
#include "esp_now.h"
#include "esp_mac.h"

// ESP-NOW 示例
void espnow_init(void)
{
    ESP_ERROR_CHECK(esp_now_init());
    
    esp_now_peer_info_t peer_info = {
        .peer_addr = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  // 广播地址
        .channel = 0,
        .encrypt = false,
    };
    esp_now_add_peer(&peer_info);
    
    esp_now_register_recv_cb(espnow_recv_cb);
    esp_now_register_send_cb(espnow_send_cb);
}

void espnow_recv_cb(const esp_now_recv_info_t *info, 
                     const uint8_t *incoming_data, int len)
{
    ESP_LOGI("ESPNOW", "Received %d bytes from " ESP_MACSTR, 
             len, ESP_MAC2STR(info->src_addr));
    // 处理数据
}

void espnow_send_data(uint8_t *data, int len)
{
    uint8_t broadcast_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    esp_now_send(broadcast_mac, data, len);
}
```

### 2.2 BLE 自定义 GATT 服务
```c
// 基于 ESP-IDF GATT Server 示例
// esp-idf/examples/bluetooth/bluedroid/gatt_server

#define GATTS_SERVICE_UUID   0x00FF
#define GATTS_CHAR_UUID_1    0xFFF1
#define GATTS_CHAR_UUID_2    0xFFF2

// 特征值值
uint16_t char1_value = 0;  // 可读写
uint16_t char2_value = 0;  // 只读（通知）

// GATT 数据库
struct gatts_db_t {
    esp_bt_uuid_t service_uuid;
    esp_gatt_id_t char_uuid_1;
    esp_gatt_id_t char_uuid_2;
    uint16_t *char1_val;
    uint16_t *char2_val;
    uint16_t char1_max_len;
    uint16_t char2_max_len;
};

// GATT 回调
void gatts_cb(esp_bt_gatt_cb_t cb_event)
{
    if (cb_event.gatt_cb_evt == ESP_BT_GATT_CB_EVT_CONNECT)
    {
        ESP_LOGI("GATT", "Client connected");
    }
    else if (cb_event.gatt_cb_evt == ESP_BT_GATT_CB_EVT_DISCONNECT)
    {
        ESP_LOGI("GATT", "Client disconnected");
    }
}

void gatts_write_cb(esp_bt_gatt_write_cb_t *param)
{
    if (param->status == ESP_BT_STATUS_SUCCESS)
    {
        ESP_LOGI("GATT", "Write successful");
    }
}

void gatts_init(void)
{
    // 初始化 BT Controller
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_bt_controller_init(&bt_cfg);
    esp_bt_controller_enable(ESP_BT_MODE_BLE);
    
    // 初始化 Bluedroid
    esp_bluedroid_init();
    esp_bluedroid_enable();
    
    // 注册 GATT 回调
    esp_bt_gatt_register_cb(gatts_cb);
    
    // 创建 GATT 服务
    esp_gatt_status_t status = esp_bt_gatt_server_register(
        &gatts_if);
    // ...
}
```

### 2.3 BLE 扫描（GATT Client）
```c
// 扫描附近的 BLE 设备
void ble_scan_init(void)
{
    // 初始化 BT（同上）
    
    esp_ble_gap_set_scan_params(&scan_params);
    esp_ble_gap_start_scanning(30);  // 扫描 30 秒
}

void esp_ble_gap_cb(esp_ble_gap_cb_param_t *param)
{
    switch (param->gap_cb_evt)
    {
        case ESP_BLE_GAP_DISC_RES_EVT:
            // 发现设备
            ESP_LOGI("BLE", "Device: " ESP_MACSTR, 
                     ESP_MAC2STR(param->disc_res.r_bda));
            ESP_LOGI("BLE", "RSSI: %d", param->disc_res.rssi);
            // 检查设备是否是目标设备
            break;
            
        case ESP_BLE_GAP_DISC_CMPL_EVT:
            ESP_LOGI("BLE", "Scan complete");
            break;
    }
}
```

---

## 第三节：FreeRTOS 多任务

### 3.1 任务创建与管理
```c
// 任务优先级（0~configMAX_PRIORITIES-1）
// 数值越大优先级越高
#define TASK_WIFI_PRIORITY    4
#define TASK_SENSOR_PRIORITY  3
#define TASK_LED_PRIORITY     2
#define TASK_DEBUG_PRIORITY   1

// 任务堆栈大小（字节）
#define TASK_WIFI_STACK       4096
#define TASK_SENSOR_STACK     2048
#define TASK_LED_STACK        1024
#define TASK_DEBUG_STACK      2048

// 全局变量
TaskHandle_t sensor_task_handle;
TaskHandle_t wifi_task_handle;
TaskHandle_t led_task_handle;
TaskHandle_t debug_task_handle;

// 传感器任务
void sensor_task(void *param)
{
    while (1)
    {
        // 读取传感器
        sht30_read(&temperature, &humidity);
        lux = bh1750_read_lux();
        
        // 通知其他任务数据已更新
        xEventGroupSetBits(data_event_group, DATA_READY_BIT);
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Wi-Fi 任务
void wifi_task(void *param)
{
    while (1)
    {
        // 等待数据就绪
        xEventGroupWaitBits(data_event_group, DATA_READY_BIT, 
                           pdTRUE, pdFALSE, portMAX_DELAY);
        
        // 发布到 MQTT
        mqtt_publish_data(temperature, humidity);
        
        // 更新网页
        update_web_page();
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// LED 任务
void led_task(void *param)
{
    int level = 0;
    while (1)
    {
        // 呼吸灯效果
        for (int i = 0; i < 256; i++)
        {
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, i);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
            vTaskDelay(pdMS_TO_TICKS(5));
        }
        for (int i = 255; i >= 0; i--)
        {
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, i);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
}

// Debug 任务
void debug_task(void *param)
{
    while (1)
    {
        ESP_LOGI("DEBUG", "Free heap: %lu bytes", esp_get_free_heap_size());
        ESP_LOGI("DEBUG", "Uptime: %lu seconds", esp_timer_get_time() / 1000000);
        vTaskDelay(pdMS_TO_TICKS(10000));  // 10 秒打印一次
    }
}

void app_main(void)
{
    // 初始化各外设
    wifi_init_sta();
    mqtt_init();
    sensor_init();
    ledc_init();
    
    // 创建事件组
    data_event_group = xEventGroupCreate();
    
    // 创建任务
    xTaskCreatePinnedToCore(sensor_task, "sensor", TASK_SENSOR_STACK, 
                             NULL, TASK_SENSOR_PRIORITY, 
                             &sensor_task_handle, 0);  // Core 0
                             
    xTaskCreatePinnedToCore(wifi_task, "wifi", TASK_WIFI_STACK, 
                             NULL, TASK_WIFI_PRIORITY, 
                             &wifi_task_handle, 0);  // Core 0
                             
    xTaskCreatePinnedToCore(led_task, "led", TASK_LED_STACK, 
                             NULL, TASK_LED_PRIORITY, 
                             &led_task_handle, 1);  // Core 1
                             
    xTaskCreatePinnedToCore(debug_task, "debug", TASK_DEBUG_STACK, 
                             NULL, TASK_DEBUG_PRIORITY, 
                             &debug_task_handle, 1);  // Core 1
}
```

### 3.2 队列与信号量
```c
// 使用队列传递数据
typedef struct {
    float temperature;
    float humidity;
    float lux;
} sensor_data_t;

QueueHandle_t data_queue;

void producer_task(void *param)
{
    sensor_data_t data;
    while (1)
    {
        sht30_read(&data.temperature, &data.humidity);
        data.lux = bh1750_read_lux();
        
        xQueueSend(data_queue, &data, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void consumer_task(void *param)
{
    sensor_data_t data;
    while (1)
    {
        if (xQueueReceive(data_queue, &data, portMAX_DELAY))
        {
            ESP_LOGI("CONSUMER", "%.1fC %.1f%% %.0flux", 
                     data.temperature, data.humidity, data.lux);
        }
    }
}

// 使用互斥锁保护共享资源
SemaphoreHandle_t uart_mutex;

void uart_print_safe(const char *format, ...)
{
    xSemaphoreTake(uart_mutex, portMAX_DELAY);
    // 执行 UART 操作
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    xSemaphoreGive(uart_mutex);
}
```

### 3.3 事件组与软件定时器
```c
// 事件组
EventGroupHandle_t system_events;

#define WIFI_CONNECTED   BIT0
#define MQTT_CONNECTED   BIT1
#define DATA_READY       BIT2
#define SYSTEM_ERROR     BIT3

void system_monitor_task(void *param)
{
    while (1)
    {
        EventBits_t bits = xEventGroupWaitBits(system_events,
            WIFI_CONNECTED | MQTT_CONNECTED | SYSTEM_ERROR,
            pdFALSE, pdFALSE, pdMS_TO_TICKS(5000));
        
        if (bits & SYSTEM_ERROR)
        {
            ESP_LOGE("SYS", "System error! Restarting...");
            esp_restart();
        }
        else if ((bits & WIFI_CONNECTED) && (bits & MQTT_CONNECTED))
        {
            ESP_LOGI("SYS", "System ready");
        }
    }
}

// 软件定时器
esp_timer_handle_t periodic_timer;

void timer_callback(void *arg)
{
    // 定时器回调（在定时器任务中执行）
    xEventGroupSetBits(system_events, DATA_READY);
}

void timer_init(void)
{
    esp_timer_create_args_t timer_args = {
        .callback = timer_callback,
        .name = "periodic",
    };
    esp_timer_create(&timer_args, &periodic_timer);
    esp_timer_start_periodic(periodic_timer, 1000000);  // 1 秒
}
```

---

## 第四节：Wi-Fi + BLE 综合应用

### 4.1 Wi-Fi 配置（BLE 配网）
```c
// 使用 BLE 进行 Wi-Fi 配网
// ESP-IDF 提供: examples/bluetooth/bluedroid/esp_ble_prov

#include "esp_bt_provisioning.h"

void ble_prov_init(void)
{
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_bt_controller_init(&bt_cfg);
    esp_bt_controller_enable(ESP_BT_MODE_BLE);
    
    esp_bluedroid_init();
    esp_bluedroid_enable();
    
    // 初始化配网服务
    esp_bt_prov_config_t prov_cfg = {
        .ssid = "",
        .password = "",
        .uuid = "esp32s3-prov",
    };
    esp_bt_prov_init(&prov_cfg);
    
    ESP_LOGI("PROV", "BLE Provisioning started. Use ESP BLE Provisioning App to configure.");
}
```

### 4.2 OTA 远程升级
```c
#include "esp_https_ota.h"

#define OTA_URL "https://example.com/firmware.bin"

void ota_init(void)
{
    // OTA 配置
    esp_https_ota_config_t ota_config = {
        .http_config = {
            .url = OTA_URL,
            .cert_pem = NULL,  // 使用内置证书
        },
        .partial_http_download = false,
    };
    
    // 启动 OTA
    esp_err_t ret = esp_https_ota(&ota_config);
    if (ret == ESP_OK)
    {
        ESP_LOGI("OTA", "Firmware updated. Restarting...");
        esp_restart();
    }
    else
    {
        ESP_LOGE("OTA", "Update failed: %s", esp_err_to_name(ret));
    }
}

// 使用 MQTT 触发 OTA
void ota_trigger_task(void *param)
{
    while (1)
    {
        // 等待 OTA 命令
        char cmd[64];
        if (xQueueReceive(ota_queue, cmd, portMAX_DELAY))
        {
            if (strcmp(cmd, "update") == 0)
            {
                ota_init();
            }
        }
    }
}
```

---

## 学习练习

### 练习 1：Wi-Fi
- [ ] Station 模式连接路由器
- [ ] AP 模式创建热点
- [ ] HTTP Server 网页显示
- [ ] MQTT 数据上报

### 练习 2：BLE
- [ ] ESP-NOW 点对点通信
- [ ] BLE GATT Server
- [ ] BLE 扫描与连接
- [ ] BLE 配网

### 练习 3：FreeRTOS
- [ ] 多任务创建
- [ ] 队列数据传递
- [ ] 互斥锁保护
- [ ] 事件组同步

### 练习 4：综合项目
- [ ] Wi-Fi 气象站（Station + HTTP）
- [ ] BLE 遥控小车
- [ ] MQTT 数据采集节点
- [ ] OTA 远程升级

---

## 本章小结
1. ✅ 掌握 Wi-Fi Station/AP 模式
2. ✅ 实现 HTTP Server 网页显示
3. ✅ 掌握 MQTT 数据上报
4. ✅ 了解 BLE GATT Server/Client
5. ✅ 掌握 ESP-NOW 点对点通信
6. ✅ 学会 FreeRTOS 多任务开发
7. ✅ 掌握队列、信号量、事件组
8. ✅ 实现 OTA 远程升级

**下一章**：ESP32-S3 综合项目实战
