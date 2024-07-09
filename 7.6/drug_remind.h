#ifndef _DRUGREMIND_H_
#define _DRUGREMIND_H_

#include "public.h"

void mqtt_recv_task(void);
int8_t mqttClient_sub_callback(unsigned char *topic, unsigned char *payload);

#endif