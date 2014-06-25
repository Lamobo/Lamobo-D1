CFLAGS=-g -Wall
CC=gcc
AR=ar
RANLIB=ranlib
LIBS=-L./ -lcgic -liniparser
OBJS=libcgic.a cgictest.cgi capture login setting

all: $(OBJS)

install: libcgic.a
	cp libcgic.a /usr/local/lib
	cp cgic.h /usr/local/include
	@echo libcgic.a is in /usr/local/lib. cgic.h is in /usr/local/include.

libcgic.a: cgic.o cgic.h
	rm -f libcgic.a
	$(AR) rc libcgic.a cgic.o
	$(RANLIB) libcgic.a

#mingw32 and cygwin users: replace .cgi with .exe

setting: cgi_anyka.o setting.o libcgic.a cgi_anyka.h
	$(CC) cgi_anyka.o setting.o -o setting ${LIBS}
login: cgi_anyka.o login.o libcgic.a cgi_anyka.h
	$(CC) cgi_anyka.o login.o -o login ${LIBS}

cgictest.cgi: cgictest.o libcgic.a
	$(CC) cgictest.o -o cgictest.cgi ${LIBS}

capture: capture.o libcgic.a
	$(CC) capture.o -o capture ${LIBS}

clean:
	rm -f *.o  $(OBJS)

