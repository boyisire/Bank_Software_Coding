INCLUDE=-I$(BMQ_PATH)/inc
CFLAGS=-Wall -g -c -D _LINUXES_
SYSLIBS=-lpthread
BMQLIBS=-L$(BMQ_PATH)/lib -lbmqapi #-lclamav
EXE=.
SRC=$(BMQ_PATH)/src
TMP=$(BMQ_PATH)/tmp
LIB=$(BMQ_PATH)/lib

TARGET=sendmsg getmsg multsend
CC=gcc
AR=ar

all: $(TARGET)

clean:	
	-mv *.o $(TMP)
	-mv $(TARGET) $(TMP)

.c.o:
	$(CC) $(CFLAGS) -o $*.o $(INCLUDE) $*.c


sendmsg:sendmsg.o
	$(CC) -g -o $(EXE)/sendmsg  sendmsg.o $(BMQLIBS) $(SYSLIBS)
	mv sendmsg.o $(TMP)

getmsg:getmsg.o
	$(CC) -g -o $(EXE)/getmsg  getmsg.o $(BMQLIBS) $(SYSLIBS)
	mv getmsg.o $(TMP)

multsend:multsend.o
	$(CC) -g -o $(EXE)/multsend multsend.o $(BMQLIBS) $(SYSLIBS)
	mv multsend.o $(TMP)
