/*VendorFunction.c is implemented by vendor*/


#include "VendorFunction.h"


void OperateDevice(bool value)
{
    pthread_mutex_lock(&gMutexLightBulb); 
    gLightBulbState = value;
    pthread_mutex_unlock(&gMutexLightBulb);
    printf("\n****Set light status to %d ****\n", gLightBulbState);
}

bool ReadDeviceStatus(void)
{
    printf("\n****Light status is %d ****\n", gLightBulbState);
    return gLightBulbState;
}