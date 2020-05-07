/**
 * Specific characteristics to be protected should be defined by the manufacturer.
 * Here we give a sample for a Light, according to Apple ADK's app sample.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//must start from 0
#define RIGHT_lightBulbOnCharacteristic_READ 0
#define RIGHT_lightBulbOnCharacteristic_WRITE 1
//The number of total rights
#define RIGHT_NUM 2

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
int CheckAuthorization(typeAuthorizationData ad, typeRight right);


/**
 * Security: Add authorization data to ACL.
 *
 * @param ad                        original additional authorization data.
 * @param right                     The Right of ad to be added.
 *
 * @return 0                        If  successful.
 */
int AppendACL(typeAuthorizationData ad, typeRight right);

/**
 * Security: Delete right of authorization data from ACL.
 *
 * @param ad                        original additional authorization data.
 * @param right                     The Right of ad to be deleted.
 *
 * @return 0                        If  successful.
 */
int DeleteACL(typeAuthorizationData ad, typeRight right);

