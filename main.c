#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <string.h> 
#include <pthread.h>  
#include <time.h>
#include <stdbool.h>
#include <signal.h>


#define IP 1
#include "AWSMain.h"
#include "App.h"
#include "DB.h"
#include "HAP.h"
#include "HAPBase.h"
#include "HAPPlatform+Init.h"
#include "HAPPlatformAccessorySetup+Init.h"
#include "HAPPlatformBLEPeripheralManager+Init.h"
#include "HAPPlatformKeyValueStore+Init.h"
#include "HAPPlatformMFiHWAuth+Init.h"
#include "HAPPlatformMFiTokenAuth+Init.h"
#include "HAPPlatformRunLoop+Init.h"
#if IP
#include "HAPPlatformServiceDiscovery+Init.h"
#include "HAPPlatformTCPStreamManager+Init.h"
#endif
 


/**
 * @brief the state of light bulb, on: ture; off: false
 */
bool gLightBulbState = false;

unsigned short gChoosedChannel = 0;

pthread_mutex_t gMutexLightBulb;
pthread_t gAWSChannel;
pthread_t gHomeKitChannel;
//gHAPaccessoryServerRef gHAPaccessoryServer;
bool gHAPrequestedFactoryReset;
bool gisHomeKitEnabled;    

int main(int argc, char **argv) {
    int AWSThreadSuccess = 0;
    int HomeKitThreadSuccess = 0;
    int channel = 1;
    
	pthread_mutex_init(&gMutexLightBulb, NULL);
    
	AWSThreadSuccess = pthread_create(&gAWSChannel, NULL, (void *)AWSMainFunction, NULL); 
	if(0 != AWSThreadSuccess){
        printf("Create AWS pthread error\n");
        exit(1);
    }
    
    
    pthread_join(gAWSChannel, NULL);
    
    printf("End of main process.\n");
    return(0);
}

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
