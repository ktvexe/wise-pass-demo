# libMQTTConnector
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
        MQTTConnectorEx.c \
		MQTTConnector.c

LOCAL_SHARED_LIBRARIES := \
        liblog \
        libcutils \
		libMosquittoCarrier

LOCAL_C_INCLUDES := \
        -I $(LOCAL_PATH)/../../Lib_EI/MosquittoCarrier \
        -I $(LOCAL_PATH)/../../Platform/Linux/ \
		-I $(LOCAL_PATH)/../../Include \
		-I external/mosquitto/lib

LOCAL_MODULE:= libMQTTConnector
LOCAL_MODULE_TAGS := debug

include $(BUILD_SHARED_LIBRARY)


