CC = ccv3
DEST =
FRC =
CFLAGS=	$(DEFS) -I$(INCLUDE) -O -v
LFLAGS=	-s -R $(RELOC) -X -F $(DOFF)
B=	$(DEST)/usr/lib/acct
BB=	$(DEST)/bin
SHB =   $(DEST)/../dist/usr/lib/acct
LIB = lib/a.a
OBJS = acctcms.o acctcom.o acctcon1.o acctcon2.o acctdisk.o acctdusg.o \
	acctmerg.o acctdusg.o acctmerg.o accton.o acctprc1.o acctprc2.o \
	acctwtmp.o fwtmp.o wtmpfix.o

all:	library $(OBJS) $(B)/acctcms $(BB)/acctcom $(B)/acctcon1\
	$(B)/acctcon2 $(B)/acctdisk $(B)/acctdusg $(B)/acctmerg $(B)/accton\
	$(B)/acctprc1 $(B)/acctprc2 $(B)/acctwtmp\
	$(B)/fwtmp $(B)/wtmpfix\
	$(SHB)/chargefee $(SHB)/ckpacct $(SHB)/dodisk $(SHB)/lastlogin\
	$(SHB)/monacct $(SHB)/nulladm $(SHB)/prctmp $(SHB)/prdaily\
	$(SHB)/prtacct $(SHB)/remove $(SHB)/runacct\
	$(SHB)/sdisk $(SHB)/shutacct $(SHB)/startup $(SHB)/turnacct

library:
	cd lib; make "INCLUDES=$(INCLUDE)"

$(B)/acctcms:	lib/a.a acctcms.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG)\
		acctcms.o $(LIB) -o $(B)/acctcms
		fixbin68 $(B)/acctcms

$(BB)/acctcom:	lib/a.a acctcom.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG)\
		acctcom.o $(LIB) -o $(BB)/acctcom
		fixbin68 $(BB)/acctcom

$(B)/acctcon1:	lib/a.a acctcon1.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG)\
		acctcon1.o  $(LIB) -o $(B)/acctcon1
		fixbin68 $(B)/acctcon1

$(B)/acctcon2:	lib/a.a acctcon2.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG)\
		acctcon2.o $(LIB) -o $(B)/acctcon2
		fixbin68 $(B)/acctcon2
 
$(B)/acctdisk:	lib/a.a acctdisk.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG)\
		acctdisk.o $(LIB) -o $(B)/acctdisk
		fixbin68 $(B)/acctdisk

$(B)/acctdusg:	lib/a.a acctdusg.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG)\
		acctdusg.o $(LIB) -o $(B)/acctdusg
		fixbin68 $(B)/acctdusg

$(B)/acctmerg:	lib/a.a acctmerg.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG)\
		acctmerg.o $(LIB) -o $(B)/acctmerg
		fixbin68 $(B)/acctmerg

$(B)/accton:	lib/a.a accton.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS)\
		accton.o $(LIB) -o $(B)/accton
		fixbin68 $(B)/accton
#		chown root $(B)/accton
#		chmod 4755 $(B)/accton

$(B)/acctprc1:	lib/a.a acctprc1.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG)\
		acctprc1.o $(LIB) -o $(B)/acctprc1
		fixbin68 $(B)/acctprc1
 
$(B)/acctprc2:	lib/a.a acctprc2.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS) $(FFLAG)\
		acctprc2.o $(LIB) -o $(B)/acctprc2
		fixbin68 $(B)/acctprc2

$(B)/acctwtmp:	lib/a.a acctwtmp.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS)\
		acctwtmp.o $(LIB) -o $(B)/acctwtmp
		fixbin68 $(B)/acctwtmp

$(B)/fwtmp:		lib/a.a fwtmp.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS)\
		fwtmp.o $(LIB) -o $(B)/fwtmp
		fixbin68 $(B)/fwtmp

$(B)/wtmpfix:	lib/a.a wtmpfix.c $(FRC)
		$(CC) $(CFLAGS) $(LFLAGS)\
		wtmpfix.o $(LIB) -o $(B)/wtmpfix
		fixbin68 $(B)/wtmpfix

$(SHB)/chargefee:	chargefee.sh $(FRC)
		cp chargefee.sh $(SHB)/chargefee

$(SHB)/ckpacct:	ckpacct.sh $(FRC)
		cp ckpacct.sh $(SHB)/ckpacct

$(SHB)/dodisk:		dodisk.sh $(FRC)
		cp dodisk.sh $(SHB)/dodisk

$(SHB)/monacct:	monacct.sh $(FRC)
		cp monacct.sh $(SHB)/monacct


$(SHB)/lastlogin:	lastlogin.sh $(FRC)
		cp lastlogin.sh $(SHB)/lastlogin
 
$(SHB)/nulladm:	nulladm.sh $(FRC)
		cp nulladm.sh $(SHB)/nulladm
 
$(SHB)/prctmp:		prctmp.sh $(FRC)
		cp prctmp.sh $(SHB)/prctmp

$(SHB)/prdaily:	prdaily.sh $(FRC)
		cp prdaily.sh $(SHB)/prdaily
 
$(SHB)/prtacct:	prtacct.sh $(FRC)
		cp prtacct.sh $(SHB)/prtacct
 
$(SHB)/remove:		remove.sh $(FRC)
		cp remove.sh $(SHB)/remove
 
$(SHB)/runacct:	runacct.sh $(FRC)
		cp runacct.sh $(SHB)/runacct
 
$(SHB)/sdisk:		sdisk.sh $(FRC)
		cp sdisk.sh $(SHB)/sdisk

$(SHB)/shutacct:	shutacct.sh $(FRC)
		cp shutacct.sh $(SHB)/shutacct

$(SHB)/startup:	startup.sh $(FRC)
		cp startup.sh $(SHB)/startup

$(SHB)/turnacct:	turnacct.sh $(FRC)
		cp turnacct.sh $(SHB)/turnacct
 

clean:
	-rm -f *.o
	cd lib; make clean
 
clobber:	clean
		-rm -f acctcms acctcom acctcon1 acctcon2 acctdisk\
		acctdusg acctmerg accton acctprc1 acctprc2 acctwtmp\
		fwtmp wtmpfix
		-rm -f chargefee ckpacct dodisk lastlogin nulladm\
		monacct prctmp prdaily prtacct remove runacct\
		sdisk shutacct startup turnacct
		cd lib; make clobber

FRC:
