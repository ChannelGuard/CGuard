#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>  

extern bool gLightBulbState;
extern pthread_mutex_t gMutexLightBulb;

void OperateDevice(bool value);

bool ReadDeviceStatus(void);