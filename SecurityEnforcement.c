#include "SecurityEnforcement.h"
#include <string.h>
#include <stdio.h>

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
            AppendACL(buff, RIGHT_lightBulbOnCharacteristic_WRITE);
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
    PreloadAuthorizationData("ad.txt");
    return 0;
}



int AppendACL(typeAuthorizationData ad, typeRight right)
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

int CheckAuthorization(typeAuthorizationData ad, typeRight right)
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

int DeleteACL(typeAuthorizationData ad, typeRight right)
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
