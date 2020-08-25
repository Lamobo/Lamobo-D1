VER:=cgic205

export MAKE=make
CROSS_COMPILE=arm-none-linux-gnueabi-
CC=$(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)ld
AR=$(CROSS_COMPILE)ar
NM=$(CROSS_COMPILE)nm
RANLIB=$(CROSS_COMPILE)ranlib

all: $(VER)/Makefile
	$(MAKE) CC=$(CC) AR=$(AR) RANLIB=$(RANLIB) -C $(VER)

install:
	$(CP) $(VER)/setting $(ROOTFS_CGI_DIR)

clean:
	-$(MAKE) -C $(VER) clean

.PHONY: all install clean 

