#
#
include ${ROOT}/usr/include/make/commondefs

# IEEE must be defined for IEEE floating point format.
# DOUBLES32BITS must be defined if doubles and floats are the same size
#    (and long floats are 64 bits)
# HOST_DEC_FLOAT must be defined if IEEE is defined and the host compiler
# uses dec float format.
# ONVAX must be defined if the target machine is a VAX.
# HOST_FULL_IEEE must be defined is IEEE is defined and the host compiler
#   uses full IEEE floating point format.
# NATIVE must be defined if stackprobe instructions are to be generated,
#   and external names are to be preceeded by an underscore.
# SGI_REGS must be defined if arguments to compiler-generated floating point
#   software are to be placed in registers.  This is necessary for SKY support
#   and assumes IEEE floating point.
# STACKPROBE must be defined if stackprobes are to be generated
# SGI_FIELDS must be defined if the new sgi bitfield optimizations are
#	to be performed.	
#
#

PROGRAM  = ccom
DEFINES  = -DIEEE -DHOST_FULL_IEEE -DDOUBLES32BITS -DNATIVE \
			-DSGI_REGS -DSGI_FIELDS -DSTABDOT -DFLEXNAMES \
			-DOLD_SGI_DEFAULT $(ENV_CFLAGS)
LCOPTS   = -O
LCDEFS   = $(DEFINES) 
LDFLAGS  = $(ENV_CFLAGS)
LIBS     = 
SYSLIBS  = 
LINTOPTS = -uv

SRCS   	= cgram.y xdefs.c scan.c pftn.c trees.c optim.c code.c local.c \
	   reader.c local2.c order.c match.c allo.c common.c table.c stab.c
 
OBJS    = cgram.o xdefs.o scan.o pftn.o trees.o optim.o code.o local.o \
	   reader.o local2.o order.o match.o allo.o common.o table.o stab.o

HDRS   	= manifest.h macdefs.h mfile1.h mfile2.h mac2defs.h \
		regprefs.h fpregs.h fpaopcodes.h skyopcodes.h

all: $(PROGRAM) 

# install program in bin

install : all
	strip ccom
	${INSTALL} -u bin -g bin -m 755 -F /lib \
	-idb "std.sw.unix mr" ccom
	rm -f $(ROOT)/usr/include/fpregs.h
	${INSTALL} -u root -g sys -m 444 -F /usr/include \
	-idb "std.sw.unix mr" fpregs.h

# remove files that can be re-created

clean:
	rm -f $(OBJS) *.out $(PROGRAM) cgram.c *.o

clobber: clean
	rm -f ccom

# make program from scratch and install in bin

new: clean all install

# create a new RCS version of the source and header files
# check the Makefile out again so it can be used to check other files out

release:
	release $(SRCS) $(HDRS) Makefile

snapshot:
	snapshot $(SRCS) $(HDRS) Makefile

junk:
	rcs -u1.10 $(SRCS) $(HDRS) Makefile

co:
	co $(SRCS) $(HDRS)

optim.o: manifest.h macdefs.h mfile1.h optim.c
trees.o: manifest.h macdefs.h mfile1.h trees.c 
pftn.o: manifest.h macdefs.h mfile1.h pftn.c
code.o: manifest.h macdefs.h mfile1.h
local.o: manifest.h macdefs.h mfile1.h 
scan.o: manifest.h macdefs.h mfile1.h scan.c
xdefs.o: manifest.h mfile1.h macdefs.h xdefs.c
cgram.o: manifest.h mfile1.h macdefs.h cgram.c
common.o: manifest.h mfile1.h macdefs.h common.c
table.o: manifest.h mfile2.h mac2defs.h macdefs.h table.c
reader.o: manifest.h mfile2.h mac2defs.h macdefs.h reader.c regprefs.h
local2.o: manifest.h mfile2.h mac2defs.h macdefs.h regprefs.h fpregs.h \
			fpaopcodes.h skyopcodes.h
order.o: manifest.h mfile2.h mac2defs.h macdefs.h regprefs.h fpregs.h
match.o: manifest.h mfile2.h mac2defs.h macdefs.h match.c 
allo.o: manifest.h mfile2.h mac2defs.h macdefs.h allo.c regprefs.h


.c.o:
	$(CCF) -c $*.c

.c.i:
	$(CCF) -P $*.c

cgram.c: cgram.y
	yacc cgram.y
	mv y.tab.c cgram.c

$(PROGRAM): $(OBJS) $(HDRS)
	@echo loading $(PROGRAM)
	$(CCF) $(LDFLAGS) $(OBJS) $(LIBS) $(SYSLIBS) -o $(PROGRAM)



# run lint over entire program and put result into lint.out

lint:
	lint $(LINTOPTS) $(DEFINES) $(SRCS) > lint.out

