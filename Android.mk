# libWISEUtil
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
        basequeue.c \
        Linux/util_path.c \
        Linux/util_string.c \
        Linux/util_libloader.c \
        Linux/NamedPipeClient.c \
        Linux/NamedPipeServer.c \
        Linux/network.c \
        Linux/util_os.c \
        Linux/util_process.c \
        Linux/service.c \
        Linux/util_power.c

LOCAL_SHARED_LIBRARIES := \
        liblog \
        libcutils

LOCAL_C_INCLUDES := \
        -I $(LOCAL_PATH)/. \
        -I $(LOCAL_PATH)/Linux

LOCAL_MODULE:= libWISEUtil
LOCAL_MODULE_TAGS := debug

include $(BUILD_SHARED_LIBRARY)


