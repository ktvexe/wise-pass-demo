# libMosquittoCarrier
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
        MosquittoCarrierEx.c \
		MosquittoCarrier.c \
		topic.c

LOCAL_SHARED_LIBRARIES := \
        liblog \
        libcutils \
	libWISEUtil \
	libmosquitto

LOCAL_C_INCLUDES := \
        -I $(LOCAL_PATH)/../../Platform \
        -I $(LOCAL_PATH)/../../Platform/Linux/ \
	-I $(LOCAL_PATH)/../../Include \
	-I $(LOCAL_PATH)/../../Library3rdParty/mosquitto-1.4.15.android/lib

LOCAL_CFLAGS := \
        -DDUMMY_PTHREAD_CANCEL \
	-DSYNC_CONNECT

LOCAL_MODULE:= libMosquittoCarrier
LOCAL_MODULE_TAGS := debug

include $(BUILD_SHARED_LIBRARY)


