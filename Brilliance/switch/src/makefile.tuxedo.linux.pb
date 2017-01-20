include $(HOME)/switch/Makefile.linux

all:	clean libusrfun.so clean

.dbc.o:
#	./getsrc switch $*.dbc
	swPreproc "<DB_DB2><DB_POSTGRESQL><DB_INFORMIX><DB_SYBASE><DB_ALTIBASE><DB_DB2_64>" $*.dbc $*.pc
	$(ESQL) $(PROCPLSFLAGS) $(ORA_INCLUDE) iname=$*.pc
	$(CC) $(CFLAGS) -DPB -DTUXEDO -DDB_ORACLE -DOS_HPUX -DDATABASE -o $*.o $(INCLUDE) $*.c
#	./rmsrc $*.dbc
#	mv $*.bnd $(BND)
	rm $*.c 
	rm $*.pc
#	rm $*.lis


.c.o:
#	./getsrc switch $*.c
	$(CC) $(CFLAGS) -DPB -DTUXEDO -DDB_ORACLE -DOS_HPUX -DDATABASE -o $*.o $(INCLUDE) $*.c 
#	./rmsrc $*.c

libusrfun.so: swUsrfun.o swUsrextfun.o  PlSec.o swFun.o swFailline.o sw_shmapi_tranlog.o \
	swCodetranfun.o swShmapi.o swxml.o swLog.o swUtil.o
	@echo Building "libusrfuntuxpb.a..."
	$(AR) rv $(LIB)/libusrfuntuxpb.a \
                swUsrfun.o \
		swUsrextfun.o \
		swFun.o \
		swCodetranfun.o \
		swShmapi.o \
		swxml.o  \
		sw_shmapi_tranlog.o swFailline.o \
		PlSec.o swLog.o swUtil.o


clean:
	rm -f tp*
	rm -f *.lis
	rm -f *.o
	#mv *.o $(OBJ); rm `ls *.dbc | sed 's/dbc/pgc/g'`; rm `ls *.dbc | sed 's/dbc/c/g'` 
