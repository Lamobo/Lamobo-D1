LOCAL_SRC_CFILES := \
    gnu_getopt.c \
    gnu_getopt_long.c \
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
    audio_process.c \
    audio_enc.c \
    main.c \
    SDcard.c

LOCAL_MODULE := record_video

PREFIX := arm-none-linux-gnueabi-

LOCAL_SHARED_LIBRARIES := iniparser akuio akmedialib akaudiocodec akaudiofilter akstreamenclib asound

LOCAL_C_INCLUDES := ../../librootfs/akmedialib/include/akmedialib \
                    ../../librootfs/akmedialib/include \
                    ../../librootfs/akuiolib/include \
                    ../../librootfs/alsalib/include

LOCAL_LDFLAGS := -L../../librootfs/akmedialib/usr/lib \
                 -L../../librootfs/akuiolib/usr/lib \
                 -L../../librootfs/alsalib/usr/lib \
                 -L./
				

include exec_rules.mk
