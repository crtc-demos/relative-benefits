# This is a makefile.

	ifeq ($(CROSS),true)
	SYSROOT :=	/home/jules/stuff-in-hiding/r-pi/piroot
	CFLAGS :=	-O2 -g -Wall --sysroot=$(SYSROOT)
	CC :=		arm-bcm2708hardfp-linux-gnueabi-gcc
	else
	SYSROOT :=
	CFLAGS :=	-O2 -g -Wall
	CC :=		gcc
	endif

	LIBS :=		-L$(SYSROOT)/usr/X11R6/lib -L$(SYSROOT)/opt/vc/lib \
			-L$(SYSROOT)/lib -L$(SYSROOT)/usr/lib \
			-L$(SYSROOT)/usr/lib/arm-linux-gnueabihf -lm -lGLESv2 \
			-lEGL -lpng -Wl,-dynamic-linker,$(SYSROOT)/lib/arm-linux-gnueabihf/ld-linux.so.3

	INCLUDE :=	-I$(SYSROOT)/opt/vc/include \
			-I$(SYSROOT)/opt/vc/include/interface/vcos/pthreads \
			-I$(SYSROOT)/usr/include/arm-linux-gnueabihf

	OBJ = 		transform.o shader.o main.o 

	LDFLAGS = 	-g

	TARGET =	pidemo
.PHONY:	clean

all:	$(TARGET)

clean:
	rm -f $(TARGET) *.o *.fo *.vo

cleaner:
	rm -f $(TARGET) *.o *.fo *.vo *.d

$(TARGET):	$(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) $(LIBS) -o $@

%.o:	%.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

%.d:	%.c
	$(CC) $(CFLAGS) $(INCLUDE) -MM $< > $@

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cleaner)
include	$(OBJ:.o=.d)
endif
endif
