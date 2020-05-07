
/**
 * The callback function that stops the Apple HAPServer.
 *
 * @param      pHAPServer               Pointer to HAPServer.
 * @param      contextSize              sizeof the Server. //Unused, but not none
 */
void doHAPAccessoryServerStop(void* _Nullable pHAPServer, size_t contextSize);
int PrepareNewSetupCode(void);