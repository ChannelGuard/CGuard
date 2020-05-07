
#define CHANNEL_HOMEKIT 1
#define CHANNEL_ZIGBEE 2


/**
 * Enable the channel. This function may block the caller.
 *
 * @param      channel      the channel to enable   
 */
int EnableChannel(unsigned short * channel);

/**
 * Disable the channel.
 *
 * @param      channel      the channel to disable  
 */
int DisableChannel(unsigned short channel);

/**
 *  Implemented by device manufacturer.
 *  Homekit main function
 */
int HomekitMainFunction(int argc, char*  argv[]);