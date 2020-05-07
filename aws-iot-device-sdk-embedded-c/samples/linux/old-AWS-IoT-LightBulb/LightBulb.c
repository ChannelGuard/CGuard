/**
 * @file LightBulb.c
 * @brief simple AWS Light Bulb
 *
 * This example takes the parameters from the LightBulb.h file and establishes a connection to the AWS IoT MQTT Platform.
 * It subscribes to the topic - "AWSChannel/LightBulb"
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

#include "LightBulb.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"

#define HOST_ADDRESS_SIZE 255
    /**
     * @brief the state of light bulb, on: ture; off: false
     */
    bool LightBulbState = false;
    /**
     * mutex for changing the state of the light bulb
     */
    pthread_mutex_t MutexLightBulb;
    /**
     * the thread id of AWS control channel
     */
    pthread_t AWSChannel;
    /**
     * the thread id of HomeKit control channel
     */
    pthread_t HomeKitChannel;

int HomekitMainFunction(int argc, char** argv)
{
    int i = 0;
    //Can we just copy the Homekit Bulb main to here? Note that there'are some static variables ,e.g, Accessory server
    while(1){
        printf("%d\n",i);
        i++;
    }
    return 0;
}
    
void iot_subscribe_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
									IoT_Publish_Message_Params *params, void *pData) {
    char cmd = 0;
    int cmdLen = 0;
    static bool isHomeKitEnabled = false;
    
	IOT_UNUSED(pData);
	IOT_UNUSED(pClient);
    
	//IOT_INFO("Payload is: \n%s", (char *) params->payload);
    
    cmdLen = (int) params->payloadLen;
    
    if(1 != cmdLen) {
        IOT_INFO("Wrong cmd: %s", (char *) params->payload);
        return;
    }
    
    cmd = ((char*) params->payload)[0];

    switch(cmd) {
        case 'n':
            pthread_mutex_lock(&MutexLightBulb);
            LightBulbState = true;
            IOT_INFO("Light bulb turned on...");
            pthread_mutex_unlock(&MutexLightBulb);
            break;
        case 'f':
            pthread_mutex_lock(&MutexLightBulb);
            LightBulbState = false;
            IOT_INFO("Light bulb turned off...");
            pthread_mutex_unlock(&MutexLightBulb);
            break;
        case 'q':
            pthread_mutex_lock(&MutexLightBulb);
			if(LightBulbState == false) {
				IOT_INFO("Light bulb is off");
			} else {
				IOT_INFO("Light bulb is on");
			}
            pthread_mutex_unlock(&MutexLightBulb);
            break;
        case 'e':
            IOT_INFO("Exiting from AWSChannel...");
            pthread_exit(NULL);
            break;
        case 'h':
            IOT_INFO("Disable the HomeKit channel...");
            
            if(false == isHomeKitEnabled){
                IOT_INFO("\tHomeKit has not been enabled, nothing to do ...");
                break;
            }
            
            // call the disable homekit channel function here
            
            pthread_cancel(HomeKitChannel);
            
            IOT_INFO("\tHomeKit channel Disableed");
            isHomeKitEnabled = false;
            
            break;
        case 'H':
            IOT_INFO("Enable the HomeKit channel...");
            
            if(true == isHomeKitEnabled){
                IOT_INFO("\tHomeKit has already been enabled, nothing to do ...");
                break;
            }
            
            // create a new thread for the HomeKit channel
            pthread_create(&HomeKitChannel, NULL, (void *)HomekitMainFunction, NULL); 
            
            IOT_INFO("\tHomeKit channel Enabled");
            isHomeKitEnabled = true;
            
            break;;
        default:
            IOT_INFO("Unsupported command: %c",cmd);
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
    char certDirectory[PATH_MAX + 1] = "../../../certs";

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

	IOT_INFO("Subscribing...");
	rc = aws_iot_mqtt_subscribe(&client, "AWSChannel/LightBulb", 20, QOS0, iot_subscribe_callback_handler, NULL);
	if(SUCCESS != rc) {
		IOT_ERROR("Error subscribing : %d ", rc);
		return;
	}
    
    IOT_INFO("CMD type:");
    IOT_INFO("\tn: Turn on the bulb");
    IOT_INFO("\tf: Turn off the bulb");
    IOT_INFO("\tq: Query the state of the light bulb");
    IOT_INFO("\te: Exit the AWS control channel");
    IOT_INFO("\tH: Enable the HomeKit control channel");
    IOT_INFO("\th: Disable the HomeKit control channel");
    
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


int main(int argc, char **argv) {    
	pthread_mutex_init(&MutexLightBulb, NULL);
	pthread_create(&AWSChannel, NULL, (void *)AWSMainFunction, NULL); 
	
    
    pthread_join(HomeKitChannel, NULL);
    pthread_join(AWSChannel, NULL);
    IOT_INFO("End of main process...");
    return(0);
}
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    