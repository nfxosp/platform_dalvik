LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(TARGET_CPU_SMP),true)
    target_smp_flag := -DANDROID_SMP=1
else
    target_smp_flag := -DANDROID_SMP=0
endif

WITH_JIT := true
include $(LOCAL_PATH)/ReconfigureDvm.mk

LOCAL_SRC_FILES := \
	compiler/codegen/arm/Exynos.cpp

LOCAL_C_INCLUDES += \
	dalvik \
	dalvik/vm

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libExynosDvm

LOCAL_CFLAGS += $(target_smp_flag)
LOCAL_CFLAGS += -Wno-unused-variable -Wno-unused-function

include $(BUILD_STATIC_LIBRARY)
