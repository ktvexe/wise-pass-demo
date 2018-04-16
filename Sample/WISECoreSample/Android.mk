# WISECoreSample
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
        WISECoreSample.c

LOCAL_SHARED_LIBRARIES := \
        liblog \
        libcutils \
		libWISEUtil \
		libWISECore \
		libMQTTConnector \
		libMosquittoCarrier

LOCAL_C_INCLUDES := \
		-I $(LOCAL_PATH)/../../Include \
		-I $(LOCAL_PATH)/../../Platform/ \
		-I $(LOCAL_PATH)/../../Platform/Linux/ \
        -I $(LOCAL_PATH)/../../Lib_EI/WISECore

LOCAL_CFLAGS := \
        -DDUMMY_PTHREAD_CANCEL

LOCAL_MODULE:= WISECoreSample
LOCAL_MODULE_TAGS := debug

include $(BUILD_EXECUTABLE)


