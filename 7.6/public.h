#ifndef _PUBLIC_H_
#define _PUBLIC_H_

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <hi_io.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_uart.h"

#include "hal_bsp_wifi.h"
#include "hal_bsp_mqtt.h"
#include "hal_bsp_ap3216c.h"
#include "hal_bsp_aw2013.h"
#include "hal_bsp_pcf8574.h"
#include "hal_bsp_sht20.h"
#include "hal_bsp_ssd1306.h"
#include "hal_bsp_structAll.h"

#include "cJSON.h"

#include "sntp.h"
#include "lwip/pbuf.h"
#include "lwip/dns.h"
#include "lwip/ip4_addr.h"
#include <time.h>

/*物联网配置*/
// 设备ID
#define DEVICE_ID "6684ff525830dc113eca9fc3_zzwzzw"
// MQTT客户端ID
#define MQTT_CLIENT_ID "6684ff525830dc113eca9fc3_zzwzzw_0_0_2024070412"
// MQTT用户名
#define MQTT_USER_NAME "6684ff525830dc113eca9fc3_zzwzzw"
// MQTT密码
#define MQTT_PASS_WORD "7c6e73c209a53024b3492c308b360840d18bb43db59ff5ac6f709fcd9d32e81b"
// 华为云平台的IP地址
#define SERVER_IP_ADDR "117.78.5.125"
// 华为云平台的IP端口号
#define SERVER_IP_PORT 1883
// 订阅 接收控制命令的主题
#define MQTT_TOPIC_SUB_COMMANDS "$oc/devices/%s/sys/commands/#"
// 发布 成功接收到控制命令后的主题
#define MQTT_TOPIC_PUB_COMMANDS_REQ "$oc/devices/%s/sys/commands/response/request_id=%s"
#define MALLOC_MQTT_TOPIC_PUB_COMMANDS_REQ "$oc/devices//sys/commands/response/request_id="

// 发布 设备属性数据的主题
#define MQTT_TOPIC_PUB_PROPERTIES "$oc/devices/%s/sys/properties/report"
#define MALLOC_MQTT_TOPIC_PUB_PROPERTIES "$oc/devices//sys/properties/report"

#define MQTT_DATA_MAX 512

/*wifi模块配置*/
#define PARAM_HOTSPOT_SSID "zzwlan"
#define PARAM_HOTSPOT_PWD "11111111"

// #define PARAM_HOTSPOT_SSID "passer"
// #define PARAM_HOTSPOT_PWD "17870492891"

/*外设配置*/
#define WIFI_IOT_UART_IDX_1 1   // 定义串口
#define WIFI_IOT_UART_IDX_2 2   // 定义串口

/*线程配置*/
#define MsgQueueObjectNumber 16 // 定义消息队列对象的个数
#define TASK_STACK_SIZE (1024 * 10)
#define MQTT_SEND_TASK_TIME 3 // s
#define MQTT_RECV_TASK_TIME 1 // s
#define RECV_RADAR_TASK_TIME 1
#define TASK_INIT_TIME 2 // s

/*结构体和变量定义*/
enum alarm_ENUM
{
    no = 0,
    heart,
    breath,
    pos,
};
typedef struct message_sensorData
{
    uint8_t heart;  // 心跳
    uint8_t breath; // 呼吸
    int amp_breath; //呼吸幅度
    int amp_heart;  //心跳幅度
    uint8_t exist;
    uint8_t speed;
    uint8_t distance;
    uint8_t pos[3];         // 位置
    enum alarm_ENUM alarm;  //
} msg_sensorData_t;

typedef struct
{
    char drug_name[25]; // 药的名字
    int drug_time;       // 吃药时间
} Drug;

#define CURVE_BUFFER_SIZE 20
typedef struct
{
    uint8_t values[CURVE_BUFFER_SIZE]; // 存储变量的值的数组
    size_t head;                 // 头指针，指向最新插入的值
    size_t tail;                 // 尾指针，指向最旧的值
    bool full;                   // 标志位，表示缓冲区是否已满
} CircularBuffer;


extern uint8_t radar_buff[20];
extern uint8_t publish_topic[MQTT_DATA_MAX];
extern uint8_t mqtt_data[MQTT_DATA_MAX];
extern msg_sensorData_t sensorData;
extern Drug drug[4];
extern time_t current_time;
extern CircularBuffer curve_heart;
extern CircularBuffer curve_breath;

#endif