#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>

#include "CGuard.h"
#include "HAPPlatformRunLoop.h"
#include "HAP.h"
#include "HAPAccessorySetup.h"
//Global single Apple HAP Server.

extern HAPAccessoryServerRef gHAPaccessoryServer;
extern bool gHAPrequestedFactoryReset;
extern bool gisHomeKitEnabled;    
extern bool gLightBulbState;
extern pthread_mutex_t gMutexLightBulb;
extern pthread_t gAWSChannel;


/**
 * The callback function that stops the Apple HAPServer.
 *
 * @param      pHAPServer               Pointer to HAPServer.
 * @param      contextSize              sizeof the Server. //Unused, but not none
 */
void doHAPAccessoryServerStop(void* _Nullable pHAPServer, size_t contextSize);
int PrepareNewSetupCode(void);

int CS_EnableChannel(unsigned short * Parachannel)
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

int CS_DisableChannel(unsigned short channel)
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

static int PreloadAuthorizationData(const char * filename);

static ACLListNode* gACL[RIGHT_NUM];

//for evaluation
static int PreloadAuthorizationData(const char * filename)
{
    FILE *fp = NULL;
    char buff[AuthorizationDataLength+1+1];
    int i = 0;
    fp = fopen(filename, "r");
    printf("\n************Entering Preload AD*************\n");
    if (fp != NULL) {
        for ( i = 0; i < 10; i++) {
            fgets(buff, AuthorizationDataLength+1+1, fp);
            printf("%s", buff);
            CC_AppendACL(buff, RIGHT_lightBulbOnCharacteristic_WRITE);
        }
    } else
    {
        printf("[Error:] Preload Ad error! \n");
    }
    fclose(fp);
    return 0;
}

int InitSecurityEnforcement(void) {
    int i = 0;
    for (i = 0; i < RIGHT_NUM; i++) {
        gACL[i] = NULL;
    }
    //PreloadAuthorizationData("ad.txt");
    return 0;
}

int CC_AppendACL(typeAuthorizationData ad, typeRight right)
{
    ACLListNode *p = NULL;
    if (ad && right >=0 && right < RIGHT_NUM) {
        p = malloc(sizeof(ACLListNode));
        if (!p) {
            return -1;
        }
        memcpy(p->ad, ad, AuthorizationDataLength);
        p->next = gACL[right];
        gACL[right] = p;
        return 0;
    } else
    {
        printf("[Error] AuthorizationData Error\n");
        return -2;
    }
}

int CC_CheckAuthData(typeAuthorizationData ad, typeRight right)
{
    if (ad && right >=0 && right < RIGHT_NUM) {
        ACLListNode *p = NULL;
        p = gACL[right];
        while (p) {
            if (memcmp(p->ad, ad, AuthorizationDataLength) == 0) {//authorized ad
                return 0;
            } else {
                p = p->next;
            }
        }
        //ad not found
        return 1;
    } else {
        return -1;
    }   
}

int CC_DeleteACL(typeAuthorizationData ad, typeRight right)
{    
    if (ad && right >=0 && right < RIGHT_NUM) {
        ACLListNode *p1 = NULL;
        ACLListNode *p2 = NULL;

        p2 = gACL[right];
        if (memcmp(p2->ad, ad, AuthorizationDataLength) == 0) {
            gACL[right] = p2->next;
            free(p2);
            return 0;
        }
        p1 = p2;
        p2 = p2->next;
        while (p2) {
            if (memcmp(p2->ad, ad, AuthorizationDataLength) == 0) {//authorized ad
                p1->next = p2->next;
                free(p2);
                return 0;
            } else {
                p1 = p2;
                p2 = p2->next;
            }
        }
        printf("[Error:] Authorization data Not found!\n");
        return -1;
    } else {
        return -2;
    }
}


void CM_GetChannelStatus(void)
{
    unsigned int i;
    unsigned int j;
    ACLListNode *p1 = NULL;

    //Print channel working status
    if (gisHomeKitEnabled) {
        printf("[CGuard] HomeKit is enabled!\n");
    } else {
        printf("[CGuard] HomeKit is disabled!\n");
    }
    
    //Get ACL list. Note that each i stands for a right that is defined by the manufacturer.
    for (i = 0; i < RIGHT_NUM; i++) {
        printf("AADs granted Right %d :\n", i);
        p1 = gACL[i];
        while (p1!=NULL) {
            putchar('\t');
            for (j = 0; j < AuthorizationDataLength; j++)
                putchar(p1->ad[j]);
            putchar('\n');
            p1 = p1->next;
        }
    }
}