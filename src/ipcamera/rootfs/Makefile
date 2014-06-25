#
#Makefile
#

include rootfs.conf
TempDir := temp_dir
RootfsDir := ./rootfs
BUILD_ROOTFS := ./create_fs_image.sh
BUILD_JFFS2 := ./create_jffs2fs.sh
BUILD_SQAUSHFS := ./create_squashfs.sh

PAGE_SIZE ?=

all: image 

image: libfs selffs 

reinstall:
ifeq ($(PAGE_SIZE), 2048)
	$(BUILD_ROOTFS) 2048
else
ifeq ($(PAGE_SIZE), 4096)
	$(BUILD_ROOTFS) 4096
else
ifeq ($(PAGE_SIZE), 8192)
	$(BUILD_ROOTFS) 8192
else
	$(BUILD_ROOTFS)
endif	# 8192
endif	# 4096
endif	# 2048

	@echo 'building root.jffs2 file system ...'
	@rm -f root.jffs2
	@$(BUILD_JFFS2)

	@echo 'building root.sqsh4 file system ...'
	@rm -f root.sqsh4
	@$(BUILD_SQAUSHFS)

libfs:$(LIBFS)
	@rm -fr $(TempDir)
	@mkdir $(TempDir)
	@$(CP) -a $(LIBFS)/* $(TempDir)/
	@find $(TempDir) -type d -name "\.svn" | xargs rm -fr {}\;
	@$(CP) -a $(TempDir)/* ./
	@rm -fr $(TempDir)
selffs:extract.sh rootfs.tar.gz
	@rm -fr $(TempDir)
	@mkdir $(TempDir)
	@./extract.sh
	@for i in $(dir_y); \
	do \
		$(CP) -a $$i/* $(TempDir)/; \
	done
	@rm -f $(TempDir)/*.txt
	@rm -f $(TempDir)/README
	@rm -fr $(TempDir)/include
	@find $(TempDir) -type d -name "\.svn" | xargs rm -fr {}\;
	@$(CP) -a $(TempDir)/* $(RootfsDir)
	@rm -fr $(TempDir)

install:

clean:
	@rm -fr rootfs rootfs.initramfs root.jffs2 root.sqsh4
	@ls $(LIBFS)/ | xargs -I {} rm -fr {} \;

.PHONY:all install image reinstall libfs selfs clean

