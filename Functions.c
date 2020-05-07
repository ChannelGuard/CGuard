/*Functions provided by us to control other channels*/

#include "Functions.h"
#include "HAPPlatformRunLoop.h"
#include "HAP.h"
#include "HAPAccessorySetup.h"
#include "internalFunction.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>

//Global single Apple HAP Server.

extern HAPAccessoryServerRef gHAPaccessoryServer;
extern bool gHAPrequestedFactoryReset;
extern bool gisHomeKitEnabled;    
extern bool gLightBulbState;
extern pthread_mutex_t gMutexLightBulb;
extern pthread_t gAWSChannel;


int EnableChannel(unsigned short * Parachannel)
{
    unsigned short channel = * Parachannel;
    printf("*********\n**********\nEnableChannel %d\n\n\n", channel);
    switch (channel)
    {
    case CHANNEL_HOMEKIT:
        remove("./.HomeKitStore");
        PrepareNewSetupCode();
        gisHomeKitEnabled = true;
        gHAPrequestedFactoryReset = false;
        HomekitMainFunction(0, NULL); // This is a block function.
        break;
    
    default:
        break;
    }
    return 0;
}

int DisableChannel(unsigned short channel)
{
    HAPError e;

    switch (channel)
    {
    case CHANNEL_HOMEKIT:
        e = HAPPlatformRunLoopScheduleCallback(
                doHAPAccessoryServerStop,
                (void*) &gHAPaccessoryServer, 
                sizeof(void*));
        if (e != kHAPError_None) {
            //todo:error handle
            return -1;
        } else
        {
            printf("\nScheduled ServerStop\n");
            //gisHomeKitEnabled = false;
        }
        
        break;
    
    default:
        break;
    }
    return 0;
}

void doHAPAccessoryServerStop(void* _Nullable context, size_t contextSize) {
    if (context) {
        //HAPAccessoryServerRef *server = (HAPAccessoryServerRef*)context;
        //We use the global variable here directly.
        gHAPrequestedFactoryReset = true;
        HAPAccessoryServerStop(&gHAPaccessoryServer);
        printf("\nStart stoping the HAP server!\n");
    } else
    {
        ;
    }
    
}

int PrepareNewSetupCode(void)
{
    HAPSetupCode setupCode;
    HAPAccessorySetupGenerateRandomSetupCode(&setupCode);
    printf("\n****************HomeKit Setup Code is %s *********************\n", setupCode.stringValue);
    // Setup info.
    HAPSetupInfo setupInfo;
    HAPPlatformRandomNumberFill(setupInfo.salt, sizeof setupInfo.salt);
    const uint8_t srpUserName[] = "Pair-Setup";
    
    //***for debug purpose only
    //memcpy(setupCode.stringValue, "111-22-333", sizeof(setupCode.stringValue));
    //***end debug

    HAP_srp_verifier(
            setupInfo.verifier,
            setupInfo.salt,
            srpUserName,
            sizeof srpUserName - 1,
            (const uint8_t*) setupCode.stringValue,
            sizeof setupCode.stringValue - 1);

    FILE *fp = fopen("./.HomeKitStore/40.10", "wb");
    if (fp == NULL) {
        printf("Create setup file failed.\n");
        return -1;
    } else
    {
        fwrite(&setupInfo, sizeof(HAPSetupInfo), 1, fp);
        fclose(fp);
    }
    
    return 0;
}
