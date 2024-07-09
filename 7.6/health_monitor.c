#include "public.h"

#define heartThreshold  60  // 心率阈值，低于这个值判断为睡眠状态
#define breathThreshold  12 // 呼吸频率阈值，低于这个值判断为睡眠状态

uint8_t diff_heart;
uint8_t diff_breath;

enum SleepState
{
    NOT_SLEEPING,
    MAYBE_SLEEPING,
    ASLEEP
};



// 定义环形缓冲区结构体

CircularBuffer *pheart = &curve_heart;
CircularBuffer *pbreath = &curve_breath;

// 初始化环形缓冲区
void CircularBuffer_init(CircularBuffer *buffer)
{
    buffer->head = 0;
    buffer->tail = 0;
    buffer->full = false;
}

// 插入新值
void CircularBuffer_insert(CircularBuffer *buffer, uint32_t value)
{
    buffer->values[buffer->head] = value;            // 插入新值到头指针位置
    buffer->head = (buffer->head + 1) % CURVE_BUFFER_SIZE; // 更新头指针位置，循环移动
    if (buffer->full)
    {
        buffer->tail = (buffer->tail + 1) % CURVE_BUFFER_SIZE; // 如果缓冲区已满，更新尾指针位置
    }
    buffer->full = (buffer->head == buffer->tail); // 判断缓冲区是否已满
}

// 获取缓冲区中第n个最新的值（n=0表示最新的值）
uint8_t CircularBuffer_get(CircularBuffer *buffer, size_t n)
{
    if (n >= CURVE_BUFFER_SIZE)
    {
        // 如果n超出缓冲区大小，返回0或者错误码，这里假设返回0
        return 0;
    }
    size_t index = (buffer->head - 1 - n + CURVE_BUFFER_SIZE) % CURVE_BUFFER_SIZE; // 计算相对位置
    return buffer->values[index];
}
void monitor_task()
{

    CircularBuffer_init(pheart);
    CircularBuffer_init(pbreath);
    while (1)
    {
        CircularBuffer_insert(pheart, sensorData.heart);
        CircularBuffer_insert(pbreath, sensorData.breath);

        sleep(2);
    }
}

/*
int main()
{

    // 时间相关变量
    time_t startTime = 0;
    enum SleepState currentState = NOT_SLEEPING;

    // 模拟监测，每2秒更新一次数据
    for (int i = 0; i < 300; ++i)
    { // 模拟10分钟，每2秒一次，共150次更新
        // 模拟传感器数据更新
        sensorData.heart = 65;  // 例子中心率设为65
        sensorData.breath = 10; // 例子中呼吸频率设为10

        // 检查当前状态
        bool asleep = checkSleep(sensorData, heartThreshold, breathThreshold);

        // 根据状态更新状态机
        switch (currentState)
        {
        case NOT_SLEEPING:
            if (asleep)
            {
                currentState = MAYBE_SLEEPING;
                startTime = time(NULL); // 记录开始检测睡眠的时间
            }
            break;
        case MAYBE_SLEEPING:
            if (!asleep)
            {
                currentState = NOT_SLEEPING;
                startTime = 0; // 重置开始时间
            }
            else
            {
                // 检查是否已经睡眠超过10分钟
                time_t currentTime = time(NULL);
                if (currentTime - startTime >= 600)
                { // 10分钟 = 600秒
                    currentState = ASLEEP;
                    printf("Person is asleep!\n");
                    return 0; // 可以根据实际情况进行其他操作
                }
            }
            break;
        case ASLEEP:
            // 已经判断为睡眠，可以继续执行其他操作或者退出
            return 0;
        }

        // 模拟每2秒更新一次数据
        sleep(2); // Unix系统下的延时函数，用于模拟2秒更新一次传感器数据
    }

    // 如果到了这里，表示在10分钟内没有进入睡眠状态
    printf("Person is not asleep.\n");
    return 0;
}
uint8_t sleep_judge()
{
    //10分钟内位置没有变化 且呼吸心跳平稳，判断为入睡

}
*/
