#include $(ORACLE_HOME)/precomp/lib/env_precomp.mk

PROCPLSFLAGS= parse=NONE lines=y
ORA_CLIENT_LIB=shared
#LIB_HOME=$(ORACLE_HOME)/lib -lclntsh -lecpg
LIB_HOME=$(ORACLE_HOME)/lib -lclntsh -lxml2
ORA_INCLUDE=include=$(SWITCH_DIR)/inc include=$(ORACLE_HOME)/precomp/public include=$(ORACLE_HOME)/rdbms/public include=$(ORACLE_HOME)/rdbms/demo include=$(ORACLE_HOME)/plsql/public include=$(ORACLE_HOME)/network/public

INCLUDE=-I$(SWITCH_DIR)/inc -I$(ORACLE_HOME)/precomp/public -I$(ORACLE_HOME)/precomp/public -I$(ORACLE_HOME)/rdbms/public -I$(ORACLE_HOME)/rdbms/demo -I$(ORACLE_HOME)/plsql/public -I/usr/include/libxml2

CFLAGS=-Wall -pedantic  -c $(DEBUGFLAGS) -DONLY_SHM_SUPPORTED
BITFLAG=
#CFLAGS=-g -c -Aa -Ae $(BITFLAG)     #在hp机上用

SYSLIBS=-lpthread -lcrypto
DMQLIBS=-L$(BMQ_PATH)/lib -lbmqapi
DMQCLLIBS=-L$(BMQ_PATH)/lib -lbmqclapi
APPLIBS=-L$(SWITCH_DIR)/lib -lusrfun -lmqapi 
APILIBS=-L$(SWITCH_DIR)/lib -lswapi 
#PTLIBS= -L$(HOME)/pt/lib -lPtSec -lPtPub -L$(HOME)/pb/lib -lPbSec -lPbPub 
PTLIBS= -L$(HOME)/pb/lib -lPbSec -lPbPub 

SRC=$(SWITCH_DIR)/src
BND=$(SWITCH_DIR)/bnd
OBJ=$(SWITCH_DIR)/obj
LIB=$(SWITCH_DIR)/lib
EXE=$(SWITCH_DIR)/bin
ETC=$(SWITCH_DIR)/etc
TMP=$(SWITCH_DIR)/tmp

AR=ar

ESQL=proc
CC=cc 

.SUFFIXES:	 .dbc .pc .c .o

all:	libmqapi.a libusrfun.so libswapi.a libgfsxml.a swInit swShmGroup swShutdown \
	swCtrl swTidTimCache \
	swRouter  swFormat  swMonrev\
	swSyscom \
	swSyscon \
	swSysmon  swSystty swTask swHost\
	swComm swComtcpsss swComtcpssc swComtcpsls swComtcpslc \
	swComtcpass swComtcpasc swComtcpals swComtcpalc \
	swFront  swFunctest swGetpack \
	swTest swTestsign swTestterm \
	swTestpack swLoadfile swAudit\
	clean

.dbc.o:
#	./getsrc switch $*.dbc
	swPreproc "<DB_DB2><DB_POSTGRESQL><DB_INFORMIX><DB_SYBASE><DB_ALTIBASE><DB_DB2_64>" $*.dbc $*.pc
	$(ESQL) $(PROCPLSFLAGS) $(ORA_INCLUDE) iname=$*.pc
	$(CC) $(CFLAGS) -DDB_ORACLE -DOS_LINUX -DDATABASE -o $*.o $(INCLUDE) $*.c
#	./rmsrc $*.dbc
#	mv $*.bnd $(BND)
	rm $*.c 
	rm $*.pc
	rm $*.lis


.c.o:
#	./getsrc switch $*.c
	$(CC) $(CFLAGS) -DDB_ORACLE -DOS_LINUX -DDATABASE -o $*.o $(INCLUDE) $*.c 
#	./rmsrc $*.c

libmqapi.a: swMqapi.o
	@echo Building "libmqapi.a..."
	$(AR) rv $(LIB)/libmqapi.a \
		swMqapi.o

libusrfun.so: swUsrfun.o swUsrextfun.o swFun.o swShmapi.o  sw_shmapi_tranlog.o\
	swCodetranfun.o  swxml.o 
	@echo Building "libusrfun.a..."
	$(AR) rv $(LIB)/libusrfun.a \
                swUsrfun.o \
                swUsrextfun.o \
		swFun.o \
		swShmapi.o \
		swCodetranfun.o \
		swxml.o
	
libswapi.a: swapi.o swGroupapi.o swFmtapi.o \
	swFmtfun.o swFm8583.o swFm8583E.o
	@echo Building "libswapi.a..."
	$(AR) rv $(LIB)/libswapi.a \
		swapi.o \
		swFmtapi.o \
		swFmtfun.o \
		swFm8583.o \
		swGroupapi.o \
		swFm8583E.o

libgfsxml.a:swxml.o
	@echo Building "libgfsxml.a..."
		$(AR) rv $(LIB)/libgfsxml.a swxml.o


swInit:   swInit.o swShmfun.o
	@echo Building "swInit..."
	$(CC) $(BITFLAG) -o $(EXE)/swInit \
		swInit.o \
		swShmfun.o  \
		-L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQLIBS) $(SYSLIBS)

swShutdown: swShutdown.o swShmfun.o
	@echo Building "swShutdown..."
	$(CC) $(BITFLAG) -o $(EXE)/swShutdown \
		swShutdown.o \
		swShmfun.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS)  \
		$(DMQLIBS) $(SYSLIBS)

swFormat: swFormat.o swFmtapi.o \
	swFmtfun.o swFm8583.o swFm8583E.o
	@echo Building "swFormat..."
	$(CC) $(BITFLAG) -o $(EXE)/swFormat \
		swFormat.o \
		swFmtapi.o \
		swFmtfun.o \
		swFm8583.o \
		swFm8583E.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQLIBS) $(SYSLIBS)

swFormatcl: swFormat.o swFmtapi.o \
	swFmtfun.o swFm8583.o swFm8583E.o
	@echo Building "swFormatcl..."
	$(CC) $(BITFLAG) -o $(EXE)/swFormatcl \
		swFormat.o \
		swFmtapi.o \
		swFmtfun.o \
		swFm8583.o \
		swFm8583E.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQLIBS) $(SYSLIBS)

swCtrl: swCtrl.o swFmtapi.o \
	swFmtfun.o swFm8583.o swFm8583E.o 
	@echo Building "swCtrl..."
	$(CC) $(BITFLAG) -o $(EXE)/swCtrl \
		swCtrl.o \
		swFmtapi.o \
		swFmtfun.o \
		swFm8583.o \
		swFm8583E.o \
		 -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQLIBS) $(SYSLIBS)

swCtrlcl: swCtrl.o swFmtapi.o \
	swFmtfun.o swFm8583.o swFm8583E.o
	@echo Building "swCtrlcl..."
	$(CC) $(BITFLAG) -o $(EXE)/swCtrlcl \
		swCtrl.o \
		swFmtapi.o \
		swFmtfun.o \
		swFm8583.o \
		swFm8583E.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQCLLIBS)

swRouter: swRouter.o 
	@echo Building "swRouter..."
	$(CC) $(BITFLAG) -o $(EXE)/swRouter \
		swRouter.o \
                $(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQLIBS) $(SYSLIBS)

swRoutercl: swRouter.o
	@echo Building "swRoutercl..."
	$(CC) $(BITFLAG) -o $(EXE)/swRoutercl \
		swRouter.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQCLLIBS)

swMonlog: swMonlog.o
	@echo Building "swMonlog..."
	$(CC) $(BITFLAG) -o $(EXE)/swMonlog\
		swMonlog.o\
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQLIBS) $(SYSLIBS)

swMonsaf: swMonsaf.o
	@echo Building "swMonsaf..."
	$(CC) $(BITFLAG) -o $(EXE)/swMonsaf\
		swMonsaf.o\
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQLIBS) $(SYSLIBS)

swMonrev: swMonrev.o
	@echo Building "swMonrev..."	
	$(CC) $(BITFLAG) -o $(EXE)/swMonrev\
		swMonrev.o\
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQLIBS) $(SYSLIBS)

swMonlogcl: swMonlog.o
	@echo Building "swMonlogcl..."
	$(CC) $(BITFLAG) -o $(EXE)/swMonlogcl\
		swMonlog.o\
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQCLLIBS)

swMonsafcl: swMonsaf.o
	@echo Building "swMonsafcl..."	
	$(CC) $(BITFLAG) -o $(EXE)/swMonsafcl\
		swMonsaf.o\
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQCLLIBS)

swMonrevcl: swMonrev.o
	@echo Building "swMonrevcl..."	
	$(CC) $(BITFLAG) -o $(EXE)/swMonrevcl\
		swMonrev.o\
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQCLLIBS)

swBatch: swBatch.o
	@echo Building "swBatch..."	
	$(CC) $(BITFLAG) -o $(EXE)/swBatch\
		swBatch.o\
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQLIBS) $(SYSLIBS)

swDb_Db2: swDb_Db2.o
	@echo Building "swDb_Db2..."	
	$(CC) $(BITFLAG) -o $(EXE)/swDb_Db2\
		swDb_Db2.o\
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQLIBS) $(SYSLIBS)
		
swDb_Db2cl: swDb_Db2.o
	@echo Building "swDb_Db2cl..."
	$(CC) $(BITFLAG) -o $(EXE)/swDb_Db2cl\
		swDb_Db2.o\
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQCLLIBS)

swFile: swFile.o swftp.o
	@echo Building "swFile..."	
	$(CC) $(BITFLAG) -o $(EXE)/swFile \
		swFile.o \
		swftp.o \
		$(SYSLIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) -L$(LIB_HOME) $(DMQLIBS) $(SYSLIBS)
		
swFilecl: swFile.o swftp.o
	@echo Building "swFilecl..."
	$(CC) $(BITFLAG) -o $(EXE)/swFilecl \
		swFile.o \
		swftp.o \
		$(SYSLIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) -L$(LIB_HOME) $(DMQCLLIBS)

swSyscom1:swSyscom1.o\
	swSyscomfun1.o
	@echo Building "swSyscom1..."
	$(CC) $(BITFLAG) -o $(EXE)/swSyscom1\
		swSyscom1.o\
		swSyscomfun1.o\
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQLIBS) $(SYSLIBS)

swSyscom: swSyscom.o\
	swSyscomfun.o
	@echo Building "swSyscom..."	
	$(CC) $(BITFLAG) -o $(EXE)/swSyscom\
		swSyscom.o \
		swSyscomfun.o\
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQLIBS) $(SYSLIBS)
		
swSyscomcl: swSyscom.o\
	swSyscomfun.o
	@echo Building "swSyscomcl..."	
	$(CC) $(BITFLAG) -o $(EXE)/swSyscomcl\
		swSyscom.o \
		swSyscomfun.o\
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQCLLIBS)

swSyscon: swSyscon.o\
	swSysconfun.o
	@echo Building "swSyscon..."
	$(CC) $(BITFLAG) -o $(EXE)/swSyscon\
		swSyscon.o\
		swSysconfun.o\
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQLIBS) $(SYSLIBS)

swSysconcl: swSyscon.o\
	swSysconfun.o
	@echo Building "swSysconcl..."
	$(CC) $(BITFLAG) -o $(EXE)/swSysconcl\
		swSyscon.o\
		swSysconfun.o\
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQCLLIBS)
		


swSysmon: swSysmon.o
	@echo Building "swSysconcl..."
	$(CC) $(BITFLAG) -o $(EXE)/swSysmon\
		swSysmon.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQLIBS) -lcurses $(SYSLIBS)

swSysmoncl: swSysmon.o
	@echo Building "swSysconcl..."
	$(CC) $(BITFLAG) -o $(EXE)/swSysmoncl\
		swSysmon.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQCLLIBS) -lcurses

swSystty: swSystty.o
	@echo Building "swSystty..."
	$(CC) $(BITFLAG) -o $(EXE)/swSystty \
		swSystty.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQLIBS) -lcurses $(SYSLIBS)

swTask: swTask.o
	@echo Building "swTask..."
	$(CC) $(BITFLAG) -o $(EXE)/swTask\
		swTask.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(BMQLIBS) $(DMQLIBS) $(SYSLIBS)

swPrep: swPrep.o
	@echo Building "swPrep..."
	$(CC) $(BITFLAG) -o $(EXE)/swPrep\
		swPrep.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APILIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQLIBS) $(SYSLIBS)

swPrep_TUX: swPrep.ec
	@echo Building "swPrep_TUX..."
	$(CC) -D TUXEDO_APP $(INFFLAGS) $(INCLUDE) \
		-I$(INFORMIXDIR)/incl -I$(TUXDIR)/include\
		swPrep.ec
	$(CC) -D TUXEDO_APP $(CFLAGS) $(INCLUDE) \
		-I$(INFORMIXDIR)/incl -I$(TUXDIR)/include\
		swPrep.c
	buildclient -o $(EXE)/swPrep_TUX\
		-f swPrep.o \
		-f "$(SYSLIBS) $(APILIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) -L$(LIB_HOME) $(DMQLIBS) \
		$(ESQLCLIBS)" 

swEchotest: swEchotest.o
	@echo Building "swEchotest..."
	$(CC) $(BITFLAG) -o $(EXE)/swEchotest\
		swEchotest.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APILIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQLIBS) $(SYSLIBS)

swEchotestcl: swEchotest.o
	@echo Building "swEchotestcl..."
	$(CC) $(BITFLAG) -o $(EXE)/swEchotestcl\
		swEchotest.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APILIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQCLLIBS)

swGetpack: swGetpack.o
	@echo Building "swGetpack..."
	$(CC) $(BITFLAG) -o $(EXE)/swGetpack\
		swGetpack.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APILIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQLIBS) $(SYSLIBS)

#swTestmult: swTestmult.o
#	@echo Building "swTestmult..."
#	$(CC) -o $(EXE)/swTestmult\
#		swTestmult.o \
#		$(SYSLIBS) -L$(LIB_HOME) $(APILIBS) \
#		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQLIBS) $(SYSLIBS)

swTestsign: swTestsign.o
	@echo Building "swTestsign..."
	$(CC) $(BITFLAG) -o $(EXE)/swTestsign\
		swTestsign.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APILIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQLIBS) $(SYSLIBS)

swTestfmt: swTestfmt.o
	@echo Building "swTestfmt..."
	$(CC) $(BITFLAG) -o $(EXE)/swTestfmt\
		swTestfmt.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APILIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQLIBS) $(SYSLIBS)

swChgvdebug: swChgvdebug.o
	@echo Building "swChgvdebug..."
	$(CC) $(BITFLAG) -o $(EXE)/swChgvdebug\
		swChgvdebug.o

swFunctest: swFunctest.o
	@echo Building "swFunctest..."
	$(CC) $(BITFLAG) -o $(EXE)/swFunctest\
		swFunctest.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APILIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQLIBS) $(SYSLIBS)  -lcrypto

swLoadfile:   swLoadfile.o
	@echo Building "swLoadfile..."
	$(CC) $(BITFLAG) -o $(EXE)/swLoadfile \
		swLoadfile.o \
		-L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQLIBS) $(SYSLIBS)


swSyncdata: swSyncdata.o
	@echo Building "swSyncdata..."
	$(CC) $(BITFLAG) -o $(EXE)/swSyncdata \
	swSyncdata.o \
	-L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
	$(DMQLIBS) $(SYSLIBS) 

swAudit:   swAudit.o
	@echo Building "swAudit..."
	$(CC) $(BITFLAG) -o $(EXE)/swAudit \
		swAudit.o \
		-L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQLIBS) $(SYSLIBS)

swShmview: swShmview.o\
	swShmtest.o
	@echo Building "swShmview..."
	$(CC) $(BITFLAG) -o $(EXE)/swShmview\
		swShmview.o\
		swShmtest.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APILIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQLIBS) $(SYSLIBS)

swTest: swTest.o
	$(CC) $(BITFLAG) -o $(EXE)/swTest\
		swTest.o\
		$(SYSLIBS) -L$(LIB_HOME) $(APILIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQLIBS) $(SYSLIBS)

swHost: swHost.o
	$(CC) $(BITFLAG) -o $(EXE)/swHost\
		swHost.o\
		$(SYSLIBS) -L$(LIB_HOME) $(APILIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQLIBS) $(SYSLIBS)

swTestpack: swTestpack.o
	$(CC) $(BITFLAG) -o $(EXE)/swTestpack\
		swTestpack.o\
		$(SYSLIBS) -L$(LIB_HOME) $(APILIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQLIBS) $(SYSLIBS)

swTestpack2: swTestpack2.o
	$(CC) $(BITFLAG) -o $(EXE)/swTestpack2\
		swTestpack2.o\
		$(SYSLIBS) -L$(LIB_HOME) $(APILIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQLIBS) $(SYSLIBS)

swTestn: swTestn.o
	$(CC) $(BITFLAG) -o $(EXE)/swTestn\
		swTestn.o\
		$(SYSLIBS) -L$(LIB_HOME) $(APILIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQLIBS) $(SYSLIBS)

swTestterm: swTestterm.o
	$(CC) $(BITFLAG) -o $(EXE)/swTestterm\
		swTestterm.o\
		$(SYSLIBS) -L$(LIB_HOME) $(APILIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQLIBS) $(SYSLIBS)
swTidTimCache:swTidTimCache.o swShmfun.o
	$(CC) $(BITFLAG) -o $(EXE)/swTidTimCache\
		swTidTimCache.o\
		swShmfun.o  \
		$(SYSLIBS) -L$(LIB_HOME) $(APILIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQLIBS) $(SYSLIBS)
swMakefml: swMakefml.o
	$(CC) $(BITFLAG) -o $(EXE)/swMakefml\
		swMakefml.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQLIBS) $(SYSLIBS)
swMkfmlhead: swMkfmlhead.o
	$(CC) $(BITFLAG) -o $(EXE)/swMkfmlhead\
		swMkfmlhead.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQLIBS) $(SYSLIBS)

swComm: swComm.o 
	@echo Building "swComm..."
	$(CC) $(BITFLAG) -o $(EXE)/swComm\
		swComm.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APILIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQLIBS) $(SYSLIBS)

swComtcpsss: swComtcpsss.o swCompub.o \
	swComtcppub.o 
	@echo Building "swComtcpsss..." 
	$(CC) $(BITFLAG) -o $(EXE)/swComtcpsss\
		swComtcpsss.o \
		swCompub.o \
		swComtcppub.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APILIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQLIBS) $(SYSLIBS)

swComtcpssc: swComtcpssc.o swCompub.o\
	swComtcppub.o
	@echo Building "swComtcpssc..."
	$(CC) $(BITFLAG) -o $(EXE)/swComtcpssc\
		swComtcpssc.o \
		swCompub.o \
		swComtcppub.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APILIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQLIBS) $(SYSLIBS)

swComtcpsls: swComtcpsls.o swCompub.o\
	swComtcppub.o
	@echo Building "swComtcpsls..."
	$(CC) $(BITFLAG) -o $(EXE)/swComtcpsls\
		swComtcpsls.o \
		swCompub.o \
		swComtcppub.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APILIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQLIBS) $(SYSLIBS)

swComtcpslc: swComtcpslc.o swCompub.o\
	swComtcppub.o
	@echo Building "swComtcpslc..."
	$(CC) $(BITFLAG) -o $(EXE)/swComtcpslc\
		swComtcpslc.o \
		swCompub.o \
		swComtcppub.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APILIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQLIBS) $(SYSLIBS)

swComtcpass: swComtcpass.o swCompub.o\
	swComtcppub.o
	@echo Building "swComtcpass..."
	$(CC) $(BITFLAG) -o $(EXE)/swComtcpass\
		swComtcpass.o \
		swCompub.o \
		swComtcppub.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APILIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQLIBS) $(SYSLIBS)

swComtcpasc: swComtcpasc.o swCompub.o\
	swComtcppub.o
	@echo Building "swComtcpasc..."
	$(CC) $(BITFLAG) -o $(EXE)/swComtcpasc\
		swComtcpasc.o \
		swCompub.o \
		swComtcppub.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APILIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQLIBS) $(SYSLIBS)

swComtcpals: swComtcpals.o swCompub.o\
	swComtcppub.o
	@echo Building "swComtcpals..."
	$(CC) $(BITFLAG) -o $(EXE)/swComtcpals\
		swComtcpals.o \
		swCompub.o \
		swComtcppub.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APILIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQLIBS) $(SYSLIBS)

swComtcpalc: swComtcpalc.o swCompub.o\
	swComtcppub.o
	@echo Building "swComtcpalc..."
	$(CC) $(BITFLAG) -o $(EXE)/swComtcpalc\
		swComtcpalc.o \
		swCompub.o \
		swComtcppub.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APILIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQLIBS) $(SYSLIBS)

swFront: swFront.o
	@echo Building "swFront..."
	$(CC) $(BITFLAG) -o $(EXE)/swFront swFront.o\
		  -L$(LIB_HOME) $(APILIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQLIBS) $(SYSLIBS)

swPreproc: swPreproc.o
	@echo Building "swPreproc..."
	$(CC) $(BITFLAG) -o $(EXE)/swPreproc swPreproc.o

swGetlicense: swGetlicense.o swLicense.o  
	@echo Building "swGetlicense..."
	$(CC) $(BITFLAG) -o $(EXE)/swGetlicense \
		swGetlicense.o \
		swLicense.o \
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQCLLIBS)	

	

swCheckcfg: swCheckcfg.o swSyscomfun.o swShmfun.o
	@echo Building "swCheckcfg..."
	$(CC) $(BITFLAG) -o $(EXE)/swCheckcfg swCheckcfg.o swSyscomfun.o swShmfun.o\
		$(SYSLIBS) -L$(LIB_HOME) $(APILIBS) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQLIBS) $(SYSLIBS)
		
swShmGroup: swShmGroup.o
	@echo Building "swShmGroup..."
	$(CC) $(BITFLAG) -o $(EXE)/swShmGroup swShmGroup.o\
		$(SYSLIBS) -L$(LIB_HOME) \
		$(APPLIBS) $(PTLIBS) $(APILIBS) $(DMQLIBS) $(SYSLIBS)
		
swSyswin: swSyswin.o\
	swSyswinfun.o\
	swCurses.o
	@echo Building "swSyswin..."
	$(CC) $(BITFLAG) -o $(EXE)/swSyswin\
		swSyswin.o\
		swSyswinfun.o\
		swCurses.o\
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQLIBS) -lcurses $(SYSLIBS)

swSyswincl: swSyswin.o\
	swSyswinfun.o\
	swCurses.o
	@echo Building "swSyswincl..."
	$(CC) $(BITFLAG) -o $(EXE)/swSyswincl\
		swSyswin.o\
		swSyswinfun.o\
		swCurses.o\
		$(SYSLIBS) -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
		$(DMQLIBS) -lcurses $(SYSLIBS)
				
yacc2c:
	yacc -d -b Route -o Route.tab.c yacc/route.y
	lex  -olex.route.c yacc/route.l
	yacc -d -b Format -o Format.tab.c yacc/format.y
	lex -olex.format.c yacc/format.l
	yacc -d -b XMF -o XMF.tab.c yacc/formatXMF.y
	lex -olex.xmf.c yacc/formatXMF.l

swRouteYacc: Route.tab.o lex.route.o
	$(CC) $(BITFLAG) -o $(EXE)/swRouteYacc Route.tab.o lex.route.o

swFormatYacc: Format.tab.o lex.format.o
	$(CC) $(BITFLAG) -o $(EXE)/swFormatYacc Format.tab.o lex.format.o

swXMFYacc: XMF.tab.o lex.xmf.o
	$(CC) $(BITFLAG) -o $(EXE)/swXMFYacc XMF.tab.o lex.xmf.o

test_putlog:   test_putlog.o 
	@echo Building "test_putlog..."
	$(CC) $(BITFLAG) -o $(EXE)/test_putlog \
                test_putlog.o \
                -L$(LIB_HOME) $(APPLIBS) $(PTLIBS) $(APILIBS) \
                $(DMQLIBS) $(SYSLIBS)

clean:
	rm -f tp*
	#mv *.o $(OBJ); rm `ls *.dbc | sed 's/dbc/pc/g'`; rm `ls *.dbc | sed 's/dbc/c/g'`
	rm -f *.lis
	rm -f *.o

#clean:
	#mv *.o $(OBJ); mv `ls *.dbc | sed 's/dbc/pc/g'` $(TMP); mv `ls *.dbc | sed 's/dbc/c/g'` $(TMP)

ftp:
	> swapi.dbc
	> swBatch.dbc
	> swChgvdebug.c
	> swCodetranfun.c
	> swComm.c
	> swCompub.c
	> swComtcpalc.c
	> swComtcpals.c
	> swComtcpasc.c
	> swComtcpass.c
	> swComtcppub.c
	> swComtcpslc.c
	> swComtcpsls.c
	> swComtcpssc.c
	> swComtcpsss.c
	> swCtrl.dbc
	> swCurses.c
	> swDb_Db2.dbc
	> swDb_Inf.dbc
	> swDb_Ora.dbc
	> swEchotest.c
	> swFile.c
	> swFm8583.c
	> swFm8583E.c
	> swFmtapi.c
	> swFmtfun.c
	> swFormat.c
	> swFront.c
	> swftp.c
	> swFunctest.c
	> swFun.dbc
	> swGetlicense.c
	> swGetpack.c
	> swInit.dbc
	> swLicense.c
	> swMakefml.c
	> swMkfmlhead.c
	> swMonlog.dbc
	> swMonrev.dbc
	> swMonsaf.dbc
	> swMqapi.c
	> swPrep.dbc
	> swPreproc.c
	> swRouter.c
	> swShmapi.dbc
	> swShmfun.c
	> swShmtest.c
	> swShmview.c
	> swShutdown.c
	> swSyscom.c
	> swSyscomfun.c
	> swSyscon.c
	> swSysconfun.dbc
	> swSysmon.dbc
	> swSystty.dbc
	> swSyswin.c
	> swSyswinfun.dbc
	> swTask.c
	> swTest.c
	> swTestmult.c
	> swTestsign.c
	> swTestterm.c
	> swUsrextfun.c
	> swUsrfun.c
	> hxz_test.c
	> swLoadfile.dbc
	> swAudit.dbc
	make

