#include "public.h"

Drug temp = {0};
int drug_state = 0; // 药物的状态
int drug_time = 0;  // 药物的是否到时间服用

/*打印药的信息*/
void drug_print(void)
{
    for (int i = 0; i < 10; i++)
    {
        printf("drug_name:%s\n", drug[i].drug_name);
        printf("drug_time:%d\n", drug[i].drug_time);
    }
}

/*从命令中读出药的信息以及是否到时间吃药*/
// int get_jsonData_value(const cJSON *const object, int paras_num)
// {
//     cJSON *value = NULL;
//     int ret = -1;
//     value = object;
//     if (value)
//     {
//         switch (paras_num)
//         {
//         case 1: // 药物名字
//             ret = 0;
//             if (drug_state == 1) // 增加药物
//             {
//                 for (int i = 0; i < 10; i++)
//                 {
//                     if (drug[i].drug_flag == 0)
//                     {
//                         strcpy(drug[i].drug_name, value->valuestring);
//                         strcpy(temp.drug_name, value->valuestring);
//                         drug[i].drug_flag = 1;
//                         value = NULL;
//                         break;
//                     }
//                 }
//             }
//             break;
//         case 2: // 吃药时间
//             ret = 0;
//             if (drug_state == 1) // 药物一次的量
//             {
//                 for (int i = 0; i < 10; i++)
//                 {
//                     if (strcmp(drug[i].drug_name, temp.drug_name) == 0)
//                     {
//                         drug[i].drug_num = value->valueint;
//                         value = NULL;
//                         break;
//                     }
//                 }
//             }
//             break;

//         }
//     }
//     return ret; // -1为失败
// }

/**
 * @brief 解析JSON数据
 */
int Parsing_json_data(const char *payload)
{
    cJSON *root = NULL, *command_name = NULL, *paras = NULL, *value = NULL;
    cJSON *json_drug_time = NULL, *json_drug_state = NULL, *json_drug_name = NULL, *json_drug_num = NULL;
    int ret_code = 1;
    int i=0;
    root = cJSON_Parse((const char *)payload);
    if (root)
    {
        // 解析JSON数据
        command_name = cJSON_GetObjectItem(root, "command_name");
        paras = cJSON_GetObjectItem(root, "paras");
        //json_drug_state = cJSON_GetObjectItem(paras, "drug_state");
        json_drug_name = cJSON_GetObjectItem(paras, "drug_name");
        json_drug_time = cJSON_GetObjectItem(paras, "drug_time");
        while(i<3)
        {
            if (drug[i].drug_time == 0)
            
                break;
            i++;
        }
        printf("i=%d\n", i);
        strcpy(drug[i].drug_name, json_drug_name->valuestring);
        drug[i].drug_time = json_drug_time->valueint;
        // ret_code = get_jsonData_value(json_drug_name, i);
        // ret_code = get_jsonData_value(json_drug_time, i);
        drug_print();
    }
    cJSON_Delete(root);
    root = command_name = paras = value = json_drug_state = json_drug_name = json_drug_num = json_drug_time = NULL;
    return ret_code;
}

// 向云端发送返回值
void send_cloud_request_code(const char *request_id, int ret_code, int request_len)
{
    char *request_topic = (char *)malloc(strlen(MALLOC_MQTT_TOPIC_PUB_COMMANDS_REQ) +
                                         strlen(DEVICE_ID) + request_len + 1);
    if (request_topic != NULL)
    {
        memset_s(request_topic,
                 strlen(DEVICE_ID) + strlen(MALLOC_MQTT_TOPIC_PUB_COMMANDS_REQ) + request_len + 1,
                 0,
                 strlen(DEVICE_ID) + strlen(MALLOC_MQTT_TOPIC_PUB_COMMANDS_REQ) + request_len + 1);
        if (sprintf_s(request_topic,
                      strlen(DEVICE_ID) + strlen(MALLOC_MQTT_TOPIC_PUB_COMMANDS_REQ) + request_len + 1,
                      MQTT_TOPIC_PUB_COMMANDS_REQ, DEVICE_ID, request_id) > 0)
        {
            if (MQTTClient_pub(request_topic, "{\"result_code\":1}", strlen("{\"result_code\":1}")) == 0)
                printf("result send\n");
        }
        free(request_topic);
        request_topic = NULL;
    }
}
/**
 * @brief MQTT接收数据的回调函数
 */
int8_t mqttClient_sub_callback(unsigned char *topic, unsigned char *payload)
{
    if ((topic == NULL) || (payload == NULL))
    {
        return -1;
    }
    else
    {
        printf("topic: %s\r\n", topic);
        printf("payload: %s\r\n", payload);

        // 提取出topic中的request_id
        char request_id[50] = {0};
        int ret_code = 1; // 1为失败
        if (0 == strcpy_s(request_id, sizeof(request_id),
                          topic + strlen(DEVICE_ID) + strlen("$oc/devices//sys/commands/request_id=")))
        {
            printf("request_id: %s\r\n", request_id);
            // 解析JSON数据
            ret_code = Parsing_json_data(payload);
            send_cloud_request_code(request_id, ret_code, sizeof(request_id));
        }
    }
    return 0;
}

/**
 * @brief MQTT  接收消息任务
 */
void mqtt_recv_task(void)
{
    // Packaged_json_data();
    // MQTTClient_pub(publish_topic, mqtt_data, strlen((char *)mqtt_data));
    while (1)
    {
        MQTTClient_sub();
        sleep(MQTT_RECV_TASK_TIME);
    }
}
