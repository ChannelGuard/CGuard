/**
 * @file LightBulb.c
 * @brief simple AWS Light Bulb
 *
 * This example takes the parameters from the LightBulb.h file and establishes a connection to the AWS IoT MQTT Platform.
 * It subscribes to the topic - "myLightBulb"
 *
 * If all the certs are correct, you should see the messages received by the application in a loop.
 *
 * The application takes in the certificate path, host name , port and the number of times the publish should happen.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <string.h> 
#include <pthread.h>  
#include <time.h>
#include <stdbool.h>

#include "AWSMain.h"
#include "VendorFunction.h"
#include "CGuard.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"
#include "VendorFunction.h"
#include "base64.h"//external_libs/mbedTLS/include/mbedtls

#define HOST_ADDRESS_SIZE 255
/**
 * @brief the state of light bulb, on: ture; off: false
 */
extern bool gLightBulbState;
extern unsigned short gChoosedChannel;
extern pthread_mutex_t gMutexLightBulb;
extern pthread_t gHomeKitChannel;
extern bool gisHomeKitEnabled;    

//ad should be base64 ad you mean? NOTE base64ad length should be given or end with \0
void add_ACL_records(typeAuthorizationData ad, typeRight * rightArray, int numberofRights) {
	int i = 0;
	int ret = 0;
	
	for (i = 0; i < numberofRights; i++){
		ret = CC_AppendACL(ad, rightArray[i]);
		if ( 0 != ret){
			printf("[Error:] AppendACL error, %d.\n", ret);
			exit(-1);
		}
	}
}

void remove_ACL_records(typeAuthorizationData ad,  typeRight * rightArray, int numberofRights) {
	int i = 0;
	int ret = 0;

	for (i = 0; i < numberofRights; i++){	
		ret = CC_DeleteACL(ad, rightArray[i]);
		if ( 0 != ret){
			printf("[Error:] DeleteACL error,%d.\n", ret);
			exit(-1);
		}
	}
}

void get_ad_rightArray(typeAuthorizationData ad, typebase64AuthorizationData base64ad, typeRight * rightArray, char * payload, int numberofRights){
	int i = 0;
	char * rightSplit = NULL;
	size_t len = 0;

	strncpy(base64ad, payload + strlen("* ad: "), base64AuthorizationDataLength );

	if (mbedtls_base64_decode(ad, 
							AuthorizationDataLength, 
							&len, 
							base64ad, 
							base64AuthorizationDataLength) != 0) {
		printf("[Error:] Invalid base64 data.\n");
	} 

	rightSplit = strtok(payload + strlen("* ad: ") + base64AuthorizationDataLength + strlen(" rights: "), " ,{}");
	rightSplit = strtok(NULL, " ,{}");

	for ( i = 0; i < numberofRights; i ++) {
		rightArray[i] = atoi(rightSplit);
		rightSplit = strtok(NULL, " ,{}");
	}


}


void iot_subscribe_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
									IoT_Publish_Message_Params *params, void *pData) {
    char cmd = 0;
    typeAuthorizationData ad;
	typebase64AuthorizationData base64ad;
	int numberofRights = 0;
	typeRight * rightArray = NULL;

    
    int HomeKitThreadSuccess = 0;
    
	IOT_UNUSED(pData);
	IOT_UNUSED(pClient);
    
    cmd = ((char*) params->payload)[0];

    switch(cmd) {
        case 'n':
			OperateDevice(true);
            break;
        case 'f':
			OperateDevice(false);
            break;
        case 'q':
            pthread_mutex_lock(&gMutexLightBulb);
			ReadDeviceStatus();
            pthread_mutex_unlock(&gMutexLightBulb);
            break;
        case 'm':
			CM_GetChannelStatus();
            break;
        case 'e':
            IOT_INFO("Exiting from AWSChannel.");
            pthread_exit(NULL);
            break;
        case 'H':
            IOT_INFO("Enable the HomeKit channel.");
            
            if(true == gisHomeKitEnabled){
                IOT_INFO("\tHomeKit has already been enabled, nothing to do.");
                break;
            }
            
            // create a new thread for the HomeKit channel
            gChoosedChannel = 1;
            HomeKitThreadSuccess = pthread_create(&gHomeKitChannel, NULL, (void *)CS_EnableChannel, &gChoosedChannel); 
            	
            if(0 != HomeKitThreadSuccess){
                printf("[Error:] Create HomeKit pthread error.\n");
                pthread_exit(NULL);
            }
            
            IOT_INFO("\tHomeKit channel Enabled.");
            gisHomeKitEnabled = true;
            
            break;
        case 'h':
            IOT_INFO("Disable the HomeKit channel.");
            
            if(false == gisHomeKitEnabled){
                IOT_INFO("\tHomeKit has not been enabled, nothing to do.");
                break;
            }
            
            // call the disable homekit channel function here
			CS_DisableChannel(CHANNEL_HOMEKIT);

            
            IOT_INFO("\tHomeKit channel Disableed.");
            gisHomeKitEnabled = false;
            
            break;

		case 'A':
			printf("Add ACL records.\n");

			sscanf(((char*) params->payload) + strlen("* ad: ") + base64AuthorizationDataLength + strlen(" rights: "), "%d", &numberofRights);
			if(rightArray != NULL){
				free(rightArray);
				rightArray = NULL;
			}
			rightArray = (typeRight *) malloc(numberofRights);

			get_ad_rightArray(ad, base64ad, rightArray, (char *)params->payload, numberofRights);

			add_ACL_records(ad, rightArray, numberofRights);


			free(rightArray);
			rightArray = NULL;
			

			break;

		case 'a':
			printf("Remove ACL records.\n");

			sscanf(((char*) params->payload) + strlen("* ad: ") + base64AuthorizationDataLength + strlen(" rights: "), "%d", &numberofRights);
			if(rightArray != NULL){
				free(rightArray);
				rightArray = NULL;
			}
			rightArray = (typeRight *) malloc(numberofRights);

			get_ad_rightArray(ad, base64ad, rightArray, (char *)params->payload, numberofRights);

			remove_ACL_records(ad, rightArray, numberofRights);

			free(rightArray);
			rightArray = NULL;
	

			break;

        default:
            printf("[Error:] Unsupported command.\n");
            break;
		}
}

void disconnectCallbackHandler(AWS_IoT_Client *pClient, void *data) {
	IOT_WARN("MQTT Disconnect");
	IoT_Error_t rc = FAILURE;

	if(NULL == pClient) {
		return;
	}

	IOT_UNUSED(data);

	if(aws_iot_is_autoreconnect_enabled(pClient)) {
		IOT_INFO("Auto Reconnect is enabled, Reconnecting attempt will start now");
	} else {
		IOT_WARN("Auto Reconnect not enabled. Starting manual reconnect...");
		rc = aws_iot_mqtt_attempt_reconnect(pClient);
		if(NETWORK_RECONNECTED == rc) {
			IOT_WARN("Manual Reconnect Successful");
		} else {
			IOT_WARN("Manual Reconnect Failed - %d", rc);
		}
	}
}

void AWSMainFunction(void) {    
    /**
     * @brief Default cert location
     */
    char certDirectory[PATH_MAX + 1] = "./AWSApp/certs_AWS";

    /**
     * @brief Default MQTT HOST URL is pulled from the LightBulb.h
     */
    char HostAddress[HOST_ADDRESS_SIZE] = AWS_IOT_MQTT_HOST;

    /**
     * @brief Default MQTT port is pulled from the LightBulb.h
     */
    uint32_t port = AWS_IOT_MQTT_PORT;

	char rootCA[PATH_MAX + 1];
	char clientCRT[PATH_MAX + 1];
	char clientKey[PATH_MAX + 1];
	char CurrentWD[PATH_MAX + 1];
	char cPayload[100];

	IoT_Error_t rc = FAILURE;

	AWS_IoT_Client client;
	IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
	IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

	getcwd(CurrentWD, sizeof(CurrentWD));
	snprintf(rootCA, PATH_MAX + 1, "%s/%s/%s", CurrentWD, certDirectory, AWS_IOT_ROOT_CA_FILENAME);
	snprintf(clientCRT, PATH_MAX + 1, "%s/%s/%s", CurrentWD, certDirectory, AWS_IOT_CERTIFICATE_FILENAME);
	snprintf(clientKey, PATH_MAX + 1, "%s/%s/%s", CurrentWD, certDirectory, AWS_IOT_PRIVATE_KEY_FILENAME);

	mqttInitParams.enableAutoReconnect = false; // We enable this later below
	mqttInitParams.pHostURL = HostAddress;
	mqttInitParams.port = port;
	mqttInitParams.pRootCALocation = rootCA;
	mqttInitParams.pDeviceCertLocation = clientCRT;
	mqttInitParams.pDevicePrivateKeyLocation = clientKey;
	mqttInitParams.mqttCommandTimeout_ms = 20000;
	mqttInitParams.tlsHandshakeTimeout_ms = 5000;
	mqttInitParams.isSSLHostnameVerify = true;
	mqttInitParams.disconnectHandler = disconnectCallbackHandler;
	mqttInitParams.disconnectHandlerData = NULL;

	rc = aws_iot_mqtt_init(&client, &mqttInitParams);
	if(SUCCESS != rc) {
		IOT_ERROR("aws_iot_mqtt_init returned error : %d ", rc);
		return;
	}

	connectParams.keepAliveIntervalInSec = 600;
	connectParams.isCleanSession = true;
	connectParams.MQTTVersion = MQTT_3_1_1;
	connectParams.pClientID = AWS_IOT_MQTT_CLIENT_ID;
	connectParams.clientIDLen = (uint16_t) strlen(AWS_IOT_MQTT_CLIENT_ID);
	connectParams.isWillMsgPresent = false;

	IOT_INFO("Connecting...");
	rc = aws_iot_mqtt_connect(&client, &connectParams);
	if(SUCCESS != rc) {
		IOT_ERROR("Error(%d) connecting to %s:%d", rc, mqttInitParams.pHostURL, mqttInitParams.port);
		return;
	}
	/*
	 * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in LightBulb.h
	 *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
	 *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
	 */
	rc = aws_iot_mqtt_autoreconnect_set_status(&client, true);
	if(SUCCESS != rc) {
		IOT_ERROR("Unable to set Auto Reconnect to true - %d", rc);
		return;
	}

	IOT_INFO("Subscribing to topic: myLightBulb");
	rc = aws_iot_mqtt_subscribe(&client, "myLightBulb", strlen("myLightBulb"), QOS0, iot_subscribe_callback_handler, NULL);
	if(SUCCESS != rc) {
		IOT_ERROR("Error subscribing : %d ", rc);
		return;
	}
    
    IOT_INFO("CMD type:");
    IOT_INFO("\tn: Turn on the bulb");
    IOT_INFO("\tf: Turn off the bulb");
    IOT_INFO("\tq: Query the state of the light bulb");
	IOT_INFO("\tm: List the authorization data")
    IOT_INFO("\te: Exit the AWS control channel");
    IOT_INFO("\tH: Enable the HomeKit control channel");
    IOT_INFO("\th: Disable the HomeKit control channel");
    IOT_INFO("\tA: Add authorization record");
    IOT_INFO("\ta: Delete authorization record");
    
	InitSecurityEnforcement();

	while(NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || SUCCESS == rc) {

		//Max time the yield function will wait for read messages
		rc = aws_iot_mqtt_yield(&client, 100);
        
		if(NETWORK_ATTEMPTING_RECONNECT == rc) {
			// If the client is attempting to reconnect we will skip the rest of the loop.
			continue;
		}
        
		sleep(1);
	}

	// Wait for all the messages to be received
	aws_iot_mqtt_yield(&client, 100);

	if(SUCCESS != rc) {
		IOT_ERROR("An error occurred in the loop.\n");
	} else {
		IOT_INFO("LightBulb done\n");
	}

	return;
}
