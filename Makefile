#This target is to ensure accidental execution of Makefile as a bash script will not execute commands like rm in unexpected directories and exit gracefully.
.prevent_execution:
	exit 0

CC = clang

#remove @ for no make command prints
DEBUG = @

APP_DIR = .
#IoT client directory
IOT_CLIENT_DIR = ./aws-iot-device-sdk-embedded-c

APP_INCLUDE_DIRS += -I $(APP_DIR)
APP_NAME = main
APP_SRC_FILES = $(APP_NAME).c

# add the other source codes files
APP_SRC_FILES += $(shell find ./AWSApp/ -name '*.c')
APP_SRC_FILES += $(shell find ./HomeKitApp/ -name '*.c')
APP_SRC_FILES += Functions.c
APP_SRC_FILES += VendorFunction.c
APP_SRC_FILES += SecurityEnforcement.c


PLATFORM_DIR = $(IOT_CLIENT_DIR)/platform/linux/mbedtls
PLATFORM_COMMON_DIR = $(IOT_CLIENT_DIR)/platform/linux/common

IOT_INCLUDE_DIRS += -I ./AWSApp
IOT_INCLUDE_DIRS += -I $(IOT_CLIENT_DIR)/include
IOT_INCLUDE_DIRS += -I $(IOT_CLIENT_DIR)/external_libs/jsmn
IOT_INCLUDE_DIRS += -I $(IOT_CLIENT_DIR)/external_libs/mbedTLS/include/mbedtls
IOT_INCLUDE_DIRS += -I $(PLATFORM_COMMON_DIR)
IOT_INCLUDE_DIRS += -I $(PLATFORM_DIR)

IOT_SRC_FILES += $(shell find $(IOT_CLIENT_DIR)/src/ -name '*.c')
IOT_SRC_FILES += $(shell find $(IOT_CLIENT_DIR)/external_libs/jsmn -name '*.c')
IOT_SRC_FILES += $(shell find $(PLATFORM_DIR)/ -name '*.c')
IOT_SRC_FILES += $(shell find $(PLATFORM_COMMON_DIR)/ -name '*.c')

#TLS - mbedtls
MBEDTLS_DIR = $(IOT_CLIENT_DIR)/external_libs/mbedTLS
TLS_LIB_DIR = $(MBEDTLS_DIR)/library
TLS_INCLUDE_DIR = -I $(MBEDTLS_DIR)/include
EXTERNAL_LIBS += -L$(TLS_LIB_DIR)
LD_FLAG += -Wl,-rpath,$(TLS_LIB_DIR)
LD_FLAG += -ldl $(TLS_LIB_DIR)/libmbedtls.a $(TLS_LIB_DIR)/libmbedcrypto.a $(TLS_LIB_DIR)/libmbedx509.a -lpthread

#HomeKit include
HOMEKIT_INCLUDE_DIRS += -I ./HomeKitADK-master/External/JSON
HOMEKIT_INCLUDE_DIRS += -I ./HomeKitADK-master/External/Base64
HOMEKIT_INCLUDE_DIRS += -I ./HomeKitADK-master/External/HTTP
HOMEKIT_INCLUDE_DIRS += -I ./HomeKitADK-master/HAP
HOMEKIT_INCLUDE_DIRS += -I ./HomeKitADK-master/Tests/Harness
HOMEKIT_INCLUDE_DIRS += -I ./HomeKitADK-master/PAL
HOMEKIT_INCLUDE_DIRS += -I ./HomeKitADK-master/PAL/POSIX
HOMEKIT_INCLUDE_DIRS += -I ./HomeKitApp
HOMEKIT_INCLUDE_DIRS += -I ./HomeKitADK-master/PAL/Crypto/MbedTLS/Ed25519


#Aggregate all include and src directories
INCLUDE_ALL_DIRS += $(IOT_INCLUDE_DIRS)
INCLUDE_ALL_DIRS += $(TLS_INCLUDE_DIR)
INCLUDE_ALL_DIRS += $(APP_INCLUDE_DIRS)
INCLUDE_ALL_DIRS += $(HOMEKIT_INCLUDE_DIRS)

SRC_FILES += $(APP_SRC_FILES)
SRC_FILES += $(IOT_SRC_FILES)

# Logging level control
LOG_FLAGS += -DENABLE_IOT_DEBUG
LOG_FLAGS += -DENABLE_IOT_INFO
LOG_FLAGS += -DENABLE_IOT_WARN
LOG_FLAGS += -DENABLE_IOT_ERROR

COMPILER_FLAGS += $(LOG_FLAGS)
#If the processor is big endian uncomment the compiler flag
#COMPILER_FLAGS += -DREVERSED

MBED_TLS_MAKE_CMD = $(MAKE) -C $(MBEDTLS_DIR)

#LIB_FILES += $(shell find ./HomeKitApp/ -name '*.a')

PRE_MAKE_CMD = $(MBED_TLS_MAKE_CMD)
#MAKE_CMD = $(CC) $(SRC_FILES) $(LIB_FILES) $(COMPILER_FLAGS)  -pthread -ldns_sd -lm  -L/lib -lcrypto -o $(APP_NAME) $(LD_FLAG) $(EXTERNAL_LIBS) $(INCLUDE_ALL_DIRS)
MAKE_CMD = $(CC) -I/include $(INCLUDE_ALL_DIRS) $(SRC_FILES) $(COMPILER_FLAGS)  -pthread -ldns_sd -lm  -L/lib -lcrypto -L /usr/lib -lLinux -lOpenSSL -lhap -o $(APP_NAME) $(LD_FLAG) $(EXTERNAL_LIBS) 

all:
	$(PRE_MAKE_CMD)
	$(DEBUG)$(MAKE_CMD)
	$(POST_MAKE_CMD)

clean:
	rm -f $(APP_DIR)/$(APP_NAME)
	$(MBED_TLS_MAKE_CMD) clean

