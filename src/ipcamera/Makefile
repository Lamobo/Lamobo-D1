#
#Makefile
#
WORKDIR:=$(shell pwd)
LIBFS:=$(WORKDIR)/../librootfs
ROOTFS_BIN_DIR ?= $(WORKDIR)/rootfs/rootfs/usr/bin
ROOTFS_CGI_DIR ?= $(WORKDIR)/rootfs/rootfs/usr/www/cgi-bin
MAKE:=make
CP:=cp -f

export MAKE CP LIBFS ROOTFS_BIN_DIR ROOTFS_CGI_DIR

dir-y += source
dir-y += rootfs


all:
	@for i in $(dir-y); \
	do \
		$(MAKE) -C $$i; \
	done

install:
	@for i in $(dir-y);	\
	do	\
		$(MAKE) -C $$i install;	\
	done

reinstall:
	@for i in $(dir-y);	\
	do	\
		$(MAKE) -C $$i reinstall; \
	done

clean:
	@for i in $(dir-y); \
	do \
		$(MAKE) -C $$i clean; \
	done


.PHONY: all install reinstall clean

