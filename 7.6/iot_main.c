#include"public.h"
#include "current_time.h"
#include "drug_remind.h"
#include "mqtt_send_func.h"
#include "screen.h"
#include "health_monitor.h"

osThreadId_t mqtt_send_task_id;  // mqtt 发布数据任务ID
osThreadId_t mqtt_recv_task_id;  // mqtt 接收数据任务ID
osThreadId_t recv_radar_task_id; // 串口接收雷达任务ID
osThreadId_t print_task_id;      // 串口打印信息任务ID
osThreadId_t sntp_task_id;       // sntp时间校正任务ID
osThreadId_t screen_task_id;     // 串口屏任务ID
osThreadId_t monitor_task_id;     // 健康监测任务ID

time_t current_time;
struct tm *time_info;

msg_sensorData_t sensorData = {0}; // 传感器的数据
Drug drug[4] = {0}; // 药的数据
CircularBuffer curve_heart = {0};
CircularBuffer curve_breath = {0};

uint8_t radar_buff[20] = {0};
uint8_t *radar_buff_ptr = radar_buff;
uint8_t publish_topic[MQTT_DATA_MAX] = {0};
uint8_t mqtt_data[MQTT_DATA_MAX] = {0};
uint8_t distance1, distance2;

void uart_radar(void)
{
    uint8_t ret;

    IotUartAttribute uart_attr = {
        // baud_rate: 9600
        .baudRate = 115200,
        // data_bits: 8bits
        .dataBits = 8,
        .stopBits = 1,
        .parity = 0,
    };

    // Initialize uart driver
    hi_io_set_func(HI_IO_NAME_GPIO_11, HI_IO_FUNC_GPIO_11_UART2_TXD);
    hi_io_set_func(HI_IO_NAME_GPIO_12, HI_IO_FUNC_GPIO_12_UART2_RXD);
    ret = IoTUartInit(WIFI_IOT_UART_IDX_2, &uart_attr);
    if (ret != 0)
    {
        printf("Failed to init uart2! Err code = %d\n", ret);
        return;
    }
    else
    {
        printf("success to init uart2!\n");
    }

    uart_attr.baudRate = 9600;
    hi_io_set_func(HI_IO_NAME_GPIO_0, HI_IO_FUNC_GPIO_0_UART1_TXD);
    hi_io_set_func(HI_IO_NAME_GPIO_1, HI_IO_FUNC_GPIO_1_UART1_RXD);
    ret = IoTUartInit(WIFI_IOT_UART_IDX_1, &uart_attr);
    if (ret != 0)
    {
        printf("Failed to init uart1! Err code = %d\n", ret);
        return;
    }
    else
    {
        printf("success to init uart1!\n");
    }
}

void recv_radar_task(void)
{
    int aver_heart;
    int aver_breath;
    uint8_t alarm_flag = 0;
    printf("uart is running\n");
    while(1)
    {
       if(IoTUartRead(WIFI_IOT_UART_IDX_2, radar_buff_ptr, 20)>0)
       {
            //printf("%s\n", radar_buff_ptr);
            if (radar_buff[0] == 0x53 && radar_buff[1] == 0x59)
            {
                switch (radar_buff[2])
                {
                // case 0x01:
                //     break;
                case 0x80:  //位置
                    if (radar_buff[3] == 0x01)
                    {
                        sensorData.exist = radar_buff[6];
                    }
                    if (radar_buff[3] == 0x02)
                    {
                        sensorData.speed = radar_buff[6];
                    }
                    if (radar_buff[3] == 0x04)
                    {
                        //sensorData.distance = radar_buff[6]*256 + radar_buff[7];
                        distance1 = radar_buff[6];
                        distance2 = radar_buff[7];
                    }
                    if (radar_buff[3]==0x05)
                    {
                        sensorData.pos[0] = radar_buff[7];
                        if (radar_buff[6] != 0)
                            sensorData.pos[0] = -sensorData.pos[0]; // x
                        sensorData.pos[1] = radar_buff[9];
                        if (radar_buff[8] != 0)
                            sensorData.pos[1] = -sensorData.pos[1]; // y
                        sensorData.pos[2] = radar_buff[11];
                        if (radar_buff[10] != 0)
                            sensorData.pos[2] = -sensorData.pos[2]; // z
                        //if(sensorData.pos[2])
                    }
                    break;
                case 0x81:  //呼吸
                    if (radar_buff[3] == 0x02 || radar_buff[3] == 0x81)
                    // if (radar_buff[3] == 0x81)
                    {
                            sensorData.breath = radar_buff[6];
                        //printf("receive breath:%d\n", sensorData.breath);
                    }
                    if (radar_buff[3] == 0x05)
                    {
                        aver_breath = (radar_buff[6] + radar_buff[7] + radar_buff[8] + radar_buff[9] + radar_buff[10]) / 5;
                        sensorData.amp_breath = abs(radar_buff[6] - 128) + abs(radar_buff[7] - 128) + abs(radar_buff[8] - 128) + abs(radar_buff[9] - 128) + abs(radar_buff[10] - 128);
                        if (sensorData.amp_breath<50)
                        {
                            sensorData.alarm = breath;
                        }
                        //printf("receive breath wave:%d %d %d %d %d,sum:%d\n", radar_buff[6] - 128, radar_buff[7] - 128, radar_buff[8] - 128, radar_buff[9] - 128, radar_buff[10] - 128,sensorData.amp_breath);
                    }
                    break;
                case 0x85:  //心率
                    if (radar_buff[3] == 0x02 || radar_buff[3] == 0x82)
                    //if (radar_buff[3] == 0x82)
                    {
                        sensorData.heart = radar_buff[6];
                        //printf("receive heart:%d\n", sensorData.heart);
                    }
                    if (radar_buff[3] == 0x05)
                    {
                        aver_heart = (radar_buff[6] + radar_buff[7] + radar_buff[8] + radar_buff[9] + radar_buff[10]) / 5;
                        sensorData.amp_heart = abs(radar_buff[6] - 128) + abs(radar_buff[7] - 128) + abs(radar_buff[8] - 128) + abs(radar_buff[9] - 128) + abs(radar_buff[10] - 128);
                        if (sensorData.amp_heart < 50)
                        {
                            sensorData.alarm = heart;
                        }
                        
                        // printf("receive heart wave:%d %d %d %d %d,sum:%d\n", radar_buff[6] - 128, radar_buff[7] - 128, radar_buff[8] - 128, radar_buff[9] - 128, radar_buff[10] - 128,
                        //        abs(radar_buff[6] - 128) + abs(radar_buff[7] - 128) + abs(radar_buff[8] - 128) + abs(radar_buff[9] - 128) + abs(radar_buff[10] - 128));
                    }
                    break;
                }
                // if (sensorData.amp_heart >= 100 || sensorData.amp_breath>=100)
                // {
                //     sensorData.alarm = no;
                // }
            }
            memset(radar_buff, 0, 20);
       }

       //printf("uart is running\n");
       //sleep(RECV_RADAR_TASK_TIME);
    }
}

void print_task(void)
{
    uint8_t sum1 = (0x53 + 0x59 + 0x85 + 0x82 + 0x00 + 0x01 + 0x0f) & 0xFF; // 0xc3;    //(0x53 + 0x59 + 0x85 + 0x82 + 0x00 + 0x01 + 0x0f) & 0xFF
    uint8_t sum2 = (0x53 + 0x59 + 0x81 + 0x81 + 0x00 + 0x01 + 0x0f) & 0xFF; //0xbe; //(0x53 + 0x59 + 0x81 + 0x81 + 0x00 + 0x01 + 0x0f) & 0xFF
    uint8_t sum3 = 0xc0;    //(0x53 + 0x59 + 0x80 + 0x84 + 0x00 + 0x01 + 0x0f) & 0xFF
    uint8_t sum4 = 0xc1;    //(0x53 + 0x59 + 0x80 + 0x85 + 0x00 + 0x01 + 0x0f) & 0xFF;
    uint8_t heartq[] = {0x53, 0x59, 0x85, 0x82, 0x00, 0x01, 0x0f, sum1, 0x54, 0x43};
    uint8_t breathq[] = {0x53, 0x59, 0x81, 0x81, 0x00, 0x01, 0x0f, sum2, 0x54, 0x43};
    uint8_t distanceq[] = {0x53, 0x59, 0x80, 0x84, 0x00, 0x01, 0x0f, sum3, 0x54, 0x43};
    uint8_t posq[] = {0x53, 0x59, 0x80, 0x85, 0x00, 0x01, 0x0f, sum4, 0x54, 0x43};
    uint8_t count=0;
    int num;
    //sscanf(txt, "aaa%d", &num);

    while (1)
    {
        //printf("\ndata:heart=%d,breath=%d,xyz=%d %d %d,d=%d,%d\n\n", sensorData.heart, sensorData.breath, sensorData.pos[0], sensorData.pos[1], sensorData.pos[2], distance1 ,distance2);
            
        // if(count)
        // {
        //     IoTUartWrite(WIFI_IOT_UART_IDX_2, heartq, strlen(heartq));
        //     IoTUartWrite(WIFI_IOT_UART_IDX_2, breathq, strlen(breathq));
        //     count = 0;
        // }
        // else
        // {
        //     IoTUartWrite(WIFI_IOT_UART_IDX_2, distanceq, strlen(heartq));
        //     IoTUartWrite(WIFI_IOT_UART_IDX_2, posq, strlen(breathq));
        //     count = 1;
        // }
        sleep(1);
    }
}

static void network_wifi_mqtt_example(void)
{
    // 外设的初始化
#if SNTP_SERVER_DNS
    ip4_addr_t dnsServerAddr;
    IP4_ADDR(&dnsServerAddr, 192, 168, 1, 1);
    dns_setserver(0, (struct ip_addr *)&dnsServerAddr);
    dns_init();

    SntpSetServernames();
#else
    SntpSetServers();
#endif
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_init();

    // printf("sntp_enabled: %d\r\n", sntp_enabled());
    // for (size_t i = 0; i < SNTP_SERVERS; i++)
    // {
    //     printf("sntp_getreachability(%d): %d\r\n", i, sntp_getreachability(i));
    // }

    // osDelay(500);
    // for (size_t i = 0; i < SNTP_SERVERS; i++)
    // {
    //     printf("sntp_getreachability(%d): %d\r\n", i, sntp_getreachability(i));
    // }

    IoTWatchDogDisable();
    uart_radar();
    p_MQTTClient_sub_callback = &mqttClient_sub_callback;

    // 连接WiFi
    if (WiFi_connectHotspots(PARAM_HOTSPOT_SSID, PARAM_HOTSPOT_PWD) != WIFI_SUCCESS)
    {
        printf("[error] connectWiFiHotspots\r\n");
    }
    sleep(TASK_INIT_TIME);

    // 连接MQTT服务器
    if (MQTTClient_connectServer(SERVER_IP_ADDR, SERVER_IP_PORT) != WIFI_SUCCESS) {
        printf("[error] mqttClient_connectServer\r\n");
    }
    sleep(TASK_INIT_TIME);

    // 初始化MQTT客户端
    if (MQTTClient_init(MQTT_CLIENT_ID, MQTT_USER_NAME, MQTT_PASS_WORD) != WIFI_SUCCESS) {
        printf("[error] mqttClient_init\r\n");
    }
    sleep(TASK_INIT_TIME);

    // 订阅主题
    /*if (MQTTClient_subscribe(MQTT_TOPIC_SUB_COMMANDS) != WIFI_SUCCESS) {
        printf("[error] mqttClient_subscribe\r\n");
    }
    sleep(TASK_INIT_TIME);*/

    //  创建线程
    osThreadAttr_t options;
    options.attr_bits = 0;
    options.cb_mem = NULL;
    options.cb_size = 0;
    options.stack_mem = NULL;
    options.priority = osPriorityNormal;

    options.name = "mqtt_send_task";
    options.stack_size = TASK_STACK_SIZE;
    mqtt_send_task_id = osThreadNew((osThreadFunc_t)mqtt_send_task, NULL, &options);
    if (mqtt_send_task_id != NULL) {
        printf("ID = %d, Create mqtt_send_task_id is OK!\r\n", mqtt_send_task_id);
    }

    options.name = "mqtt_recv_task";
    options.stack_size = TASK_STACK_SIZE;
    mqtt_recv_task_id = osThreadNew((osThreadFunc_t)mqtt_recv_task, NULL, &options);
    if (mqtt_recv_task_id != NULL) {
        printf("ID = %d, Create mqtt_recv_task_id is OK!\r\n", mqtt_recv_task_id);
    }

    options.name = "recv_radar_task";
    options.stack_size = TASK_STACK_SIZE;
    recv_radar_task_id = osThreadNew((osThreadFunc_t)recv_radar_task, NULL, &options);
    if (recv_radar_task_id != NULL) {
        printf("ID = %d, Create recv_radar_task_id is OK!\r\n", recv_radar_task_id);
    }
    options.name = "print_task";
    options.stack_size = TASK_STACK_SIZE/10;
    print_task_id = osThreadNew((osThreadFunc_t)print_task, NULL, &options);
    if (print_task_id != NULL) {
        printf("ID = %d, Create print_task_id is OK!\r\n", print_task_id);
    }
    options.name = "sntptask";
    options.stack_size = TASK_STACK_SIZE/5;
    sntp_task_id = osThreadNew((osThreadFunc_t)SntpTask, NULL, &options);
    if (sntp_task_id != NULL) {
        printf("ID = %d, Create sntp_task_id is OK!\r\n", sntp_task_id);
    }
    options.name = "screen_task";
    options.stack_size = TASK_STACK_SIZE;
    screen_task_id = osThreadNew((osThreadFunc_t)screen_task, NULL, &options);
    if (screen_task_id != NULL)
    {
        printf("ID = %d, Create screen_task_id is OK!\r\n", screen_task_id);
    }
    options.name = "monitor_task";
    options.stack_size = TASK_STACK_SIZE;
    monitor_task_id = osThreadNew((osThreadFunc_t)monitor_task, NULL, &options);
    if (monitor_task_id != NULL)
    {
        printf("ID = %d, Create monitor_task_id is OK!\r\n", monitor_task_id);
    }
}
SYS_RUN(network_wifi_mqtt_example);
