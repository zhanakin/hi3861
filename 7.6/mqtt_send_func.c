#include "public.h"

/**
 * @brief 组JSON数据
 */
int Packaged_json_data(void)
{
    cJSON *root = NULL, *array = NULL, *services = NULL, *array2 = NULL;
    cJSON *properties = NULL, *properties2 = NULL;
    int ret = 0;

    // 组JSON数据
    root = cJSON_CreateObject(); // 创建一个对象
    services = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "services", services);
    array = cJSON_CreateObject();
    cJSON_AddStringToObject(array, "service_id", "stat");
    properties = cJSON_CreateObject();
    cJSON_AddItemToObject(array, "properties", properties);
    cJSON_AddNumberToObject(properties, "heart", (int)sensorData.heart);
    cJSON_AddNumberToObject(properties, "breath", (int)sensorData.breath);
    cJSON_AddNumberToObject(properties, "posx", (int)sensorData.pos[0]);
    cJSON_AddNumberToObject(properties, "posy", (int)sensorData.pos[1]);
    cJSON_AddNumberToObject(properties, "posz", (int)sensorData.pos[2]);
    cJSON_AddNumberToObject(properties, "alarm_type", (int)sensorData.alarm);

    // 创建一个JSON数组，用于存放呼吸心率曲线数据
    cJSON *arrayDataJson = cJSON_CreateArray();
    cJSON *arrayDataJson2 = cJSON_CreateArray();

    for (int i = 0; i < CURVE_BUFFER_SIZE; i++)
    {
        cJSON_AddItemToArray(arrayDataJson2, cJSON_CreateNumber(CircularBuffer_get(&curve_breath, i)));
    }
    cJSON_AddItemToObject(properties, "curve_breath", arrayDataJson2);

    for (int i = 0; i < CURVE_BUFFER_SIZE; i++)
    {
        cJSON_AddItemToArray(arrayDataJson, cJSON_CreateNumber(CircularBuffer_get(&curve_heart, i)));
    }
    cJSON_AddItemToObject(properties, "curve_heart", arrayDataJson);


    cJSON_AddItemToArray(services, array); // 将对象添加到数组中

    /* 格式化打印创建的带数组的JSON对象 */
    char *str_print = cJSON_PrintUnformatted(root);
    if (str_print != NULL)
    {
        //printf("%s\n", str_print);
        if (strcpy_s(mqtt_data, strlen(str_print) + 1, str_print) == 0)
        {
            ret = 0;
        }
        else
        {
            ret = -1;
        }
        cJSON_free(str_print);
    }
    else
    {
        ret = -1;
    }
    if (root != NULL)
    {
        cJSON_Delete(root);
        cJSON_Delete(arrayDataJson);
        // cJSON_Delete(arrayDataJson2);
    }
    else
    {
        ret = -1;
    }
    properties = properties2 = root = array = array2 = services = str_print = NULL; //= str_print

    return ret;
}

/**
 * @brief MQTT  发布消息任务
 */
void mqtt_send_task(void)
{
    while (1)
    {
        // 组Topic
        memset_s(publish_topic, MQTT_DATA_MAX, 0, MQTT_DATA_MAX);
        if (sprintf_s(publish_topic, MQTT_DATA_MAX, MQTT_TOPIC_PUB_PROPERTIES, DEVICE_ID) > 0)
        {
            // 组JSON数据
            Packaged_json_data();
            // 发布消息
            MQTTClient_pub(publish_topic, mqtt_data, strlen(mqtt_data));
        }
        sleep(MQTT_SEND_TASK_TIME);
    }
}