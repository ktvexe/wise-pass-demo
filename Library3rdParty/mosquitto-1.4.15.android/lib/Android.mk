LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	mosquitto.c \
	logging_mosq.c \
	memory_mosq.c \
	messages_mosq.c\
	net_mosq.c\
	read_handle.c\
	read_handle_client.c\
	read_handle_shared.c\
	send_mosq.c\
	send_client_mosq.c\
	socks_mosq.c\
	srv_mosq.c\
	thread_mosq.c\
	time_mosq.c\
	tls_mosq.c\
	util_mosq.c\
	will_mosq.c

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libcutils \
	libssl \
	libcrypto

LOCAL_CFLAGS := \
	-DWITH_THREADING -DDUMMY_PTHREAD_CANCEL
	
LOCAL_C_INCLUDES := \
		-I $(LOCAL_PATH)/..

LOCAL_MODULE:= libmosquitto
LOCAL_MODULE_TAGS := debug

include $(BUILD_SHARED_LIBRARY)
