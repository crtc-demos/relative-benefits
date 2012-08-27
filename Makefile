# This is a makefile.

	CC :=		gcc
	CFLAGS :=	-O2 -g -Wall
	LIBS :=		-L/usr/X11R6/lib -L/opt/vc/lib -lm \
			-lGLESv2 -lEGL -lpng
	INCLUDE :=	-I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads
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
