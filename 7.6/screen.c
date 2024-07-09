#include "public.h"

uint8_t endtxt[3]={0xff,0xff,0xff};
char txt[40];

void HMI_end(void)
{
    IoTUartWrite(WIFI_IOT_UART_IDX_1, (unsigned char *)endtxt, 3);
}

void HMI(void)
{
    char txt[40];
    char data[2];
   if(IoTUartRead(WIFI_IOT_UART_IDX_1, (unsigned char *)data, 1)>0)
    {
        if (data[0] == 1)
        {
            sensorData.alarm = 0;
            data[0] = 0;
        }
    }
    for (int i = 0; i < 4; i++)
    {
        if (drug[i].drug_time != 0)
        {
            sprintf(txt, "t%d.txt=\"%s %d:%d吃\"", i, drug[i].drug_name, drug[i].drug_time / 100, drug[i].drug_time % 100);
            IoTUartWrite(WIFI_IOT_UART_IDX_1, (unsigned char *)txt, strlen(txt));
            HMI_end();
        }
        else
        {
            sprintf(txt, "t%d.txt=\"\"", i);
            IoTUartWrite(WIFI_IOT_UART_IDX_1, (unsigned char *)txt, strlen(txt));
            HMI_end();
        }
    }
    if (sensorData.alarm == no)
    {
        sprintf(txt, "t10.txt=\"心率:%d\"", sensorData.heart);
        IoTUartWrite(WIFI_IOT_UART_IDX_1, (unsigned char *)txt, strlen(txt));
        HMI_end();

        sprintf(txt, "t11.txt=\"呼吸:\r%d\"", sensorData.breath);
        IoTUartWrite(WIFI_IOT_UART_IDX_1, (unsigned char *)txt, strlen(txt));
        HMI_end();
    }
    else if (sensorData.alarm == heart)
    {
        sprintf(txt, "t10.txt=\"心率异常！\"");
        IoTUartWrite(WIFI_IOT_UART_IDX_1, (unsigned char *)txt, strlen(txt));
        HMI_end();
    }
    else if (sensorData.alarm == breath)
    {
        sprintf(txt, "t11.txt=\"呼吸异常！\"");
        IoTUartWrite(WIFI_IOT_UART_IDX_1, (unsigned char *)txt, strlen(txt));
        HMI_end();
    }
}

void screen_task(void)
{
    while (1)
    {

        //printf("screen ok\n");
        HMI();
        sleep(1);
    }
}