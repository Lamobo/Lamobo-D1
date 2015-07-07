LOCAL_SRC_CFILES := \
		gnu_getopt.c \
		gnu_getopt_long.c \
		log.c \
		Thread.c \
		AkAlsaHardware.c \
		AkAudioEncoder.c \
		AkAudioRecorder.c \
		AkAudioDemo.c \
		main.c

LOCAL_MODULE := record_audio

PREFIX := arm-none-linux-gnueabi-

LOCAL_SHARED_LIBRARIES := asound akaudiocodec akaudiofilter
LOCAL_C_INCLUDES := ../../librootfs/akmedialib/include/akmedialib \
					../../librootfs/alsalib/include
LOCAL_LDFLAGS := -L../../librootfs/akmedialib/usr/lib \
				 -L../../librootfs/alsalib/usr/lib

include exec_rules.mk
