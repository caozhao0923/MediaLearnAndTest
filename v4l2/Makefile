#Makefile
CC      = gcc
CFLAGS  = -g -Wall -O2

TARGET  = test_v4l2.bin
SRCS    = test_v4l2.c
C_OBJS  = test_v4l2.o

all:$(TARGET)

$(TARGET):$(C_OBJS)
	$(CC) $(CFLAGS) -o $@ $^
%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $<
.PHONY:clean
clean:
	rm -rf *.o $(TARGET) $(CXX_OBJS) $(C_OBJS)
	rm ./img/*
