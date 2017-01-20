INCLUDE=-I$(BMQ_PATH)/inc

CFLAGS=-Wall -g -c -D _LINUXES_

SYSLIBS=-lz -lbz2 -lgmp -lpthread
#BMQLIBS=-L$(BMQ_PATH)/lib -lbmqapi -lclamav 
BMQLIBS=-L$(BMQ_PATH)/lib -lbmqapi
#BMQLIBS=-L$(BMQ_PATH)/lib -lbmqapi 
#BMQCLLIBSC=-L$(BMQ_PATH)/lib -lbmqclapi -lclamav
BMQCLLIBSC=-L$(BMQ_PATH)/lib -lbmqclapi
#BMQCLLIBSC=-L$(BMQ_PATH)/lib -lbmqclapi 

EXE=$(BMQ_PATH)/bin

SRC=$(BMQ_PATH)/src
TMP=$(BMQ_PATH)/tmp
LIB=$(BMQ_PATH)/lib

CC=cc
AR=ar

all:	ser  cls 

ser:	libbmqapi.a bmqstartup bmqshutdown bmqmng bmqclean  bmqGrp_snd bmqGrp_rcv bmqGrp_rcvs bmqGrp_rcvw bmqSetload bmqGrp_rcv_comm bmqGrp_rcv_front

cls:	libbmqclapi.a bmqcls bmqfilecls bmqfilemng bmqfilesaf bmqfilelog

clean:	
	mv *.o $(TMP)

.c.o:
	#./getsrc bmq $*.c
	$(CC) $(CFLAGS) -o $*.o $(INCLUDE) $*.c
	#./rmsrc $*.c


libbmqapi.a: bmqapi.o bmqLicense.o bmqcommpub.o bmqusrfun.o
	$(AR) rv $(LIB)/libbmqapi.a \
		bmqapi.o bmqLicense.o bmqcommpub.o bmqusrfun.o
	mv bmqapi.o bmqLicense.o $(TMP)

libbmqclapi.a:	bmqclapi.o bmqcommpub.o bmqusrfun.o 
	$(AR) rv $(LIB)/libbmqclapi.a \
		bmqclapi.o bmqcommpub.o bmqusrfun.o 
	mv bmqclapi.o bmqcommpub.o bmqusrfun.o  $(TMP)

bmqcls:	bmqcls.o
	$(CC) -g -o $(EXE)/bmqcls bmqcls.o $(BMQLIBS) $(SYSLIBS)
	mv bmqcls.o $(TMP)

bmqfilecls:	bmqfilecls.o
	$(CC) -g -o $(EXE)/bmqfilecls bmqfilecls.o  $(BMQLIBS) $(SYSLIBS)
	mv bmqfilecls.o $(TMP)

bmqfilemng:	bmqfilemng.o
	$(CC) -g -o $(EXE)/bmqfilemng bmqfilemng.o  $(BMQLIBS) $(SYSLIBS)
	mv bmqfilemng.o $(TMP)

bmqfilesaf:	bmqfilesaf.o
	$(CC) -g -o $(EXE)/bmqfilesaf bmqfilesaf.o  $(BMQLIBS) $(SYSLIBS)
	mv bmqfilesaf.o $(TMP)

bmqfilelog:	bmqfilelog.o
	$(CC) -g -o $(EXE)/bmqfilelog bmqfilelog.o  $(BMQLIBS) $(SYSLIBS)
	mv bmqfilelog.o $(TMP)

bmqstartup:	bmqstartup.o
	$(CC) -o $(EXE)/bmqstartup bmqstartup.o $(BMQLIBS) $(SYSLIBS)
	mv bmqstartup.o $(TMP)

bmqshutdown:	bmqshutdown.o
	$(CC) -o $(EXE)/bmqshutdown bmqshutdown.o $(BMQLIBS) $(SYSLIBS)
	mv bmqshutdown.o $(TMP)

bmqmng:	bmqmng.o
	$(CC) -g -o $(EXE)/bmqmng bmqmng.o $(BMQLIBS) $(SYSLIBS)
	mv bmqmng.o $(TMP)

#bmqmgr: bmqmgr.o bmqwinapi.o
#	$(CC) -g -o $(EXE)/bmqmgr bmqmgr.o bmqwinapi.o $(BMQLIBS) $(SYSLIBS) \
#	-lcurses -ltermcap
#	mv bmqmgr.o bmqwinapi.o $(TMP)

bmqGrp_rcv:	bmqGrp_rcv.o
	$(CC) -g -o $(EXE)/bmqGrp_rcv bmqGrp_rcv.o $(BMQLIBS) $(SYSLIBS)
	mv bmqGrp_rcv.o $(TMP)

bmqGrp_rcv_comm:	bmqGrp_rcv_comm.o
	$(CC) -g -o $(EXE)/bmqGrp_rcv_comm bmqGrp_rcv_comm.o $(BMQLIBS) $(SYSLIBS)
	mv bmqGrp_rcv_comm.o $(TMP)

bmqGrp_rcv_front:	bmqGrp_rcv_front.o
	$(CC) -g -o $(EXE)/bmqGrp_rcv_front bmqGrp_rcv_front.o $(BMQLIBS) $(SYSLIBS)
	mv bmqGrp_rcv_front.o $(TMP)

bmqGrp_rcvs:	bmqGrp_rcvs.o
	$(CC) -g -o $(EXE)/bmqGrp_rcvs bmqGrp_rcvs.o $(BMQLIBS) $(SYSLIBS)
	mv bmqGrp_rcvs.o $(TMP)

bmqGrp_rcvw:	bmqGrp_rcvw.o
	$(CC) -g -o $(EXE)/bmqGrp_rcvw bmqGrp_rcvw.o $(BMQLIBS) $(SYSLIBS)
	mv bmqGrp_rcvw.o $(TMP)

bmqGrp_snd:	bmqGrp_snd.o
	$(CC) -g -o $(EXE)/bmqGrp_snd bmqGrp_snd.o $(BMQLIBS) $(SYSLIBS)
	mv bmqGrp_snd.o $(TMP)

bmqclean:bmqclean.o
	$(CC) -g -o $(EXE)/bmqclean bmqclean.o $(BMQLIBS) $(SYSLIBS)
	mv bmqclean.o $(TMP)

bmqSetload:bmqSetload.o
	$(CC) -g -o $(EXE)/bmqSetload bmqSetload.o $(BMQLIBS) $(SYSLIBS)
	mv bmqSetload.o $(TMP)

getlicense:getlicense.o
	$(CC) -g -o $(EXE)/getlicense getlicense.o $(BMQLIBS) $(SYSLIBS)
	mv getlicense.o $(TMP)

ftp:
	> bmqapi.c         
	> bmqBuild         
	> bmqChgDebug.c    
	> bmqclapi.c       
	> bmqclean.c       
	> bmqcls.c         
	> bmqcommpub.c     
	> bmqfilecls.c     
	> bmqfilelog.c     
	> bmqfilemng.c     
	> bmqfilesaf.c     
	> bmqGrp_rcv.c     
	> bmqGrp_rcvs.c    
	> bmqGrp_rcvw.c    
	> bmqGrp_snd.c     
	> bmqLicense.c     
	> bmqmgr.c         
	> bmqmng.c         
	> bmqSetload.c     
	> bmqshutdown.c    
	> bmqstartup.c     
	> bmqsutdown.c     
	> bmqusrfun.c      
	> bmqwinapi.c      
	> getlicense.c     
	> swXmldoc.c       
	make
  
  
  
  
  
  
  
  
