# mosquitto_pub
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	pub_client.c \
	client_shared.c 

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libcutils \
	libmosquitto

LOCAL_CFLAGS := -DVERSION="\"1.4.15\""
LOCAL_C_INCLUDES := \
	-I $(LOCAL_PATH)/.. \
	-I $(LOCAL_PATH)/../lib

LOCAL_MODULE:= mosquitto_pub
LOCAL_MODULE_TAGS := debug

include $(BUILD_EXECUTABLE)

# mosquitto_sub
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	sub_client.c \
	client_shared.c

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libcutils \
	libmosquitto

LOCAL_CFLAGS := -DVERSION="\"1.4.15\""

LOCAL_C_INCLUDES := \
	-I $(LOCAL_PATH)/.. \
	-I $(LOCAL_PATH)/../lib

LOCAL_MODULE:= mosquitto_sub
LOCAL_MODULE_TAGS := debug

include $(BUILD_EXECUTABLE)
