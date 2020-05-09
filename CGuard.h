#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CGuardRightConfig.h"

#define CHANNEL_HOMEKIT 1
#define CHANNEL_ZIGBEE 2
/**
 *END
*/
#define AuthorizationDataLength 32
#define base64AuthorizationDataLength ( (AuthorizationDataLength%3) ? ( (AuthorizationDataLength/3+1)*4 ) : (AuthorizationDataLength/3*4) )

typedef char typeAuthorizationData[AuthorizationDataLength];
typedef char typebase64AuthorizationData[base64AuthorizationDataLength];
typedef unsigned short typeRight;
typedef struct ACLListNode
{
    typeAuthorizationData ad;
    struct ACLListNode *next;
} ACLListNode;

/**
 * Enable the channel. This function may block the caller.
 *
 * @param      channel      the channel to enable   
 */
int CS_EnableChannel(unsigned short * channel);

/**
 * Disable the channel.
 *
 * @param      channel      the channel to disable  
 */
int CS_DisableChannel(unsigned short channel);

/**
 *  Implemented by device manufacturer.
 *  Homekit main function
 */
int HomekitMainFunction(int argc, char*  argv[]);

int InitSecurityEnforcement(void);

/**
 * Security: Check the right of authorization data.
 *
 * @param ad                        original additional authorization data.
 * @param right                     The Right subject is going to access.
 *
 * @return 0                        If checking successfully passed.
 * @return 1                        If checking failed.
 */
int CC_CheckAuthData(typeAuthorizationData ad, typeRight right);


/**
 * Security: Add authorization data to ACL.
 *
 * @param ad                        original additional authorization data.
 * @param right                     The Right of ad to be added.
 *
 * @return 0                        If  successful.
 */
int CC_AppendACL(typeAuthorizationData ad, typeRight right);

/**
 * Security: Delete right of authorization data from ACL.
 *
 * @param ad                        original additional authorization data.
 * @param right                     The Right of ad to be deleted.
 *
 * @return 0                        If  successful.
 */
int CC_DeleteACL(typeAuthorizationData ad, typeRight right);

void CM_GetChannelStatus(void);


