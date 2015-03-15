LOCAL_PATH:=$(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    inotify_watch.cpp \
    inotify_utils.cpp \
    event_queue.cpp \

LOCAL_C_INCLUDES := \
    external/skia/include/core \
    external/skia/include/images \
    external/openssl/include

LOCAL_SHARED_LIBRARIES := \
    libgui \
    libutils \
    libskia \
    libcutils \
    libcrypto

LOCAL_MODULE:=inotify_watch
LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT_SBIN)
LOCAL_MODULE_TAGS:=optional

include $(BUILD_EXECUTABLE)

