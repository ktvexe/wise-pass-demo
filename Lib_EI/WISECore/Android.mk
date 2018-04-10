# libWISECore
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
        liteparse.c \
		WISECoreEx.c \
		WISECore.c

LOCAL_SHARED_LIBRARIES := \
        liblog \
        libcutils \
		libWISEUtil \
		libMQTTConnector

LOCAL_C_INCLUDES := \
        -I $(LOCAL_PATH)/../../Lib_EI/MQTTConnector \
        -I $(LOCAL_PATH)/../../Platform/Linux/ \
		-I $(LOCAL_PATH)/../../Include

LOCAL_MODULE:= libWISECore
LOCAL_MODULE_TAGS := debug

include $(BUILD_SHARED_LIBRARY)


