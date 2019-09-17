LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := VedioDecoder

LOCAL_STATIC_LIBRARIES :=	\
						android_native_app_glue	\
						cpufeatures	\

LOCAL_CFLAGS := -D__cpusplus -g -O3 -mfloat-abi=softfp -mfpu=neon -march=armv7-a -mtune=cortex-a8

LOCAL_LDLIBS    := -llog -landroid -ljnigraphics

LOCAL_SRC_FILES :=	\
				avjni.c	\
				VedioDecoder.cpp	\
				CAndroidDecoder.cpp	\
				ffjni.c	\
				mediacodec_wrapper.c	\
				XH264Decoder.cpp	\
				XH264Reader.cpp		\
				NeonDecoder.cpp	\

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
$(call import-module,android/cpufeatures)
