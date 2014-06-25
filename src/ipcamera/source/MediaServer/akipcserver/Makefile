LIVE555_DIR = ../aklivelib
LIBROOT_DIR = ../../../../librootfs
USE_CPP := 1

LOCAL_SRC_CFILES := \
		log.c \
		encode.c \
		muxer.c \
		asyncwrite.c \
		CycBuffer.c \
		Thread.c \
		Tool.c \
		camera.c \
		AkAlsaHardware.c \
		audio.c \
		audio_enc.c	\
		akIPCNetControl.c \
		inisetting.c \
		AkRecordManager.c \
		led.c	\
		SDcard.c \
		AkMotionDetect.c \
		akaudioDecode.c \
		photograph.c 	\
		PTZControl.c	\

LOCAL_SRC_CXXFILES :=main.cpp video_process.cpp audio_process.cpp DemuxForLive555.cpp platform_cb.cpp

LOCAL_MODULE := akipcserver

PREFIX := arm-none-linux-gnueabi-

#链接时指定的libs所在目录
LOCAL_SHARED_LIBRARIES := iniparser akuio akmedialib akaudiocodec akaudiofilter akstreamenclib asound akmotiondetectlib
#编译时指定的要使用的外部头文件目录
LOCAL_C_INCLUDES := $(LIBROOT_DIR)/akmedialib/include/akmedialib \
					$(LIBROOT_DIR)/akmedialib/include \
					$(LIBROOT_DIR)/akuiolib/include \
					$(LIBROOT_DIR)/alsalib/include \
					$(LIVE555_DIR)/include/akliveMedia/include \
					$(LIVE555_DIR)/include/liveMedia/include \
					$(LIVE555_DIR)/include/groupsock/include \
					$(LIVE555_DIR)/include/UsageEnvironment/include \
					$(LIVE555_DIR)/include/BasicUsageEnvironment/include
#链接时指定要链接的lib.
LOCAL_LDFLAGS := -L$(LIBROOT_DIR)/akmedialib/usr/lib \
				-L$(LIBROOT_DIR)/akuiolib/usr/lib \
				-L$(LIBROOT_DIR)/alsalib/usr/lib \
				-L./ \
				$(LIVE555_DIR)/libs/libakliveMedia.a \
				$(LIVE555_DIR)/libs/libliveMedia.a \
				$(LIVE555_DIR)/libs/libgroupsock.a \
				$(LIVE555_DIR)/libs/libBasicUsageEnvironment.a \
				$(LIVE555_DIR)/libs/libUsageEnvironment.a 
				
include exec_rules.mk
