#
#Makefile
#


dir_y += akipcserver

all:
	@for i in $(dir_y); \
	do \
		$(MAKE) -C $$i; \
	done

install:
	@for i in $(dir_y); \
	do \
		$(MAKE) -C $$i install; \
	done

reinstall:

clean:
	@for i in $(dir_y); \
	do \
		$(MAKE) -C $$i clean; \
	done


.PHONY: all install reinstall clean

