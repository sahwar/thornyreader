LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libmobi

LOCAL_STATIC_LIBRARIES  := thornyreader

#LOCAL_CFLAGS             += -lmobi

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/../thornyreader \
    $(LOCAL_PATH)/src \
    $(LOCAL_PATH)/tools

LOCAL_SRC_FILES := \
    src/epubcreator.c \
	src/buffer.c \
    src/compression.c \
    src/debug.c \
    src/encryption.c \
    src/index.c \
    src/memory.c \
    src/meta.c \
    src/miniz.c \
    src/opf.c \
    src/parse_rawml.c \
    src/read.c \
    src/sha1.c \
    src/structure.c \
    src/util.c \
    src/write.c \
    src/xmlwriter.c


    LOCAL_SRC_FILES += \
    tools/common.c \
    tools/mobimeta.c \
    tools/win32/getopt.c\


LOCAL_ARM_MODE := $(APP_ARM_MODE)

include $(BUILD_STATIC_LIBRARY)