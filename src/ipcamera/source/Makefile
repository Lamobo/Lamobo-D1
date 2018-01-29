#
#Makefile
#

dir_y += cgic
dir_y += keypad
dir_y += MediaServer
dir_y += httpup
dir_y += thread_poll

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

