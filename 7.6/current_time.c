#include "public.h"

#define SNTP_SERVERS 4
// #define ARRAY_SIZE(a) sizeof(a)/sizeof(a[0])
#define STACK_SIZE (4096)
#define DELAY_TICKS_500 (500)

static int g_netId = -1;

#if SNTP_SERVER_DNS
static const char *g_ntpServerList[] = {
    // refers from https://dns.icoa.cn/ntp/#china
    "cn.ntp.org.cn",        // 中国 NTP 快速授时服务
    "ntp.ntsc.ac.cn",       // 国家授时中心 NTP 服务器
    "time.pool.aliyun.com", // 阿里云公共 NTP 服务器
    "cn.pool.ntp.org",      // 国际 NTP 快速授时服务
};
// #define SNTP_SERVERS ARRAY_SIZE(g_ntpServerList)

void SntpSetServernames(void)
{
    for (size_t i = 0; i < SNTP_SERVERS; i++)
    {
        sntp_setservername(i, g_ntpServerList[i]);
    }
}

#else

// ip4_addr_t g_ntpServerList[SNTP_MAX_SERVERS];
ip4_addr_t g_ntpServerList[SNTP_SERVERS];

void SntpSetServers(void)
{
    IP4_ADDR(&g_ntpServerList[0], 114, 67, 237, 130); // cn.ntp.org.cn
    IP4_ADDR(&g_ntpServerList[1], 114, 118, 7, 163);  // ntp.ntsc.ac.cn
    IP4_ADDR(&g_ntpServerList[2], 182, 92, 12, 11);   // time.pool.aliyun.com
    IP4_ADDR(&g_ntpServerList[3], 193, 182, 111, 12); // cn.pool.ntp.org

    for (size_t i = 0; i < SNTP_SERVERS; i++)
    {
        sntp_setserver(i, (ip_addr_t *)&g_ntpServerList[i]);
    }
}
#endif

void SntpTask(void)
{
    sleep(2);

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_init();

    char timestr[40];
    while (1)
    {
        // time_info = localtime(&current_time);
        // printf("Current local time:%d.%d.%d,%d:%d:%d\n", time_info->tm_year+1900, time_info->tm_mon+1, time_info->tm_mday, time_info->tm_hour+8, time_info->tm_min, time_info->tm_sec);
        ++current_time;
        sleep(1);
    }
}