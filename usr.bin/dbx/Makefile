#
#
include ${ROOT}/usr/include/make/commondefs

# make file for debugger "dbx"
#
# The file "defs.h" is included by all.
#

.SUFFIXES:
.SUFFIXES: .h .c .s .o

AOUT	= dbx
LCINCS = -I${ROOT}/usr/include 
IRISFLAGS = '-Ddouble=long float' '-Datof=_latof' -DGB_FIXES $(INCLUDES)
LCDEFS	= -D${machine} ${IRISFLAGS} '-DMAINTAINER="dbx-maintainer"' ${DEBUG_FLAG} -DSVS -DDBMEX
AS	= as20 -i

LD = cc
LIBRARIES = -lPW

OBJ = \
    y.tab.o \
    asm.o \
    events.o \
    c.o \
    check.o \
    coredump.o \
    debug.o \
    eval.o \
    fortran.o \
    keywords.o \
    languages.o \
    library.o \
    lists.o \
    machine.o \
    main.o \
    mappings.o \
    modula-2.o \
    names.o \
	newobj.o \
    object.o \
    operators.o \
    pascal.o \
    printsym.o \
    process.o \
    runtime.o \
    scanner.o \
    source.o \
    stabstring.o \
    symbols.o \
    tree.o \
    ops.o \
	dbxshm.o

HDR = \
    asm.h \
    events.h \
    c.h \
    check.h \
    coredump.h \
    debug.h \
    eval.h \
    fortran.h \
    keywords.h \
    languages.h \
    lists.h \
    machine.h \
    main.h \
    mappings.h \
    modula-2.h \
    names.h \
    object.h \
    operators.h \
    pascal.h \
    printsym.h \
    process.h \
    runtime.h \
    scanner.h \
    source.h \
    stabstring.h \
    symbols.h \
    tree.h \
    ops.h

SRC = \
    defs.h \
    commands.y \
    asm.c \
    events.c \
    c.c \
    cerror.s \
    check.c \
    coredump.c \
    debug.c \
    eval.c \
    fortran.c \
    keywords.c \
    languages.c \
    library.c \
    lists.c \
    machine.c \
    main.c \
    mappings.c \
    modula-2.c \
    names.c \
	newobj.c \
    object.c \
    operators.c \
    pascal.c \
    printsym.c \
    process.c \
    runtime.c \
    scanner.c \
    source.c \
    stabstring.c \
    symbols.c \
    tree.c \
    ops.c \
	dbxshm.c

.c.o:
	@echo "compiling $*.c"
	${CC} ${CFLAGS} -c $*.c

.s.o:
	@echo "assembling $*.s"
	@rm -f tmp
	@/lib/cpp $*.s | grep -v "^#" > tmp
	@${AS} -o $*.o tmp
	@rm -f tmp

.c.h:
	./makedefs -f $*.c $*.h

${AOUT}: makedefs mkdate ${HDR} ${OBJ}
	@rm -f date.c
	@./mkdate > date.c
	@echo "linking"
	@${CC} ${CFLAGS} -c date.c
	@${LD} ${LDFLAGS} ${DEBUG_FLAG} date.o ${OBJ} ${LIBRARIES} -o ${AOUT}

profile: ${HDR} ${OBJ}
	@rm -f date.c
	@./mkdate > date.c
	@echo "linking with -p"
	@${CC} ${LDFLAGS} ${DEBUG_FLAG} -p date.c ${OBJ} ${LIBRARIES} -o ${AOUT}

y.tab.c: commands.y
	@echo "expect 2 shift/reduce conflicts"
	yacc -d commands.y

makedefs: makedefs.c library.c
	echo NOTE: expect lots of 'enumeration type clash' warnings from ccom
	${CC} -I${ROOT}/usr/include $(DEBUG_FLAG)  \
		makedefs.c library.c -o makedefs 
	rm -f library.o 

mkdate: mkdate.c
	${CC} $(DEBUG_FLAG) ${CFLAGS} mkdate.c -o mkdate

cerror.o: cerror.s
	/lib/cpp -P  cerror.s cerror.i $(IRISFLAGS)
	$(AS) -o cerror.o cerror.i

print:
	@echo "don't print it, it's too long"

#
# Don't worry about the removal of header files, they're created from
# the source files.
#

clean clobber:
	rm -f ${HDR} ${OBJ} y.tab.c y.tab.h ${AOUT} mkdate mkdate.o \
	    makedefs makedefs.o date.c date.o core mon.out prof.out make.out\
	    cerror.i cerror.o

cleandefs:
	rm -f ${HDR} y.tab.h

testinstall: ${AOUT} test install

test:
	@cd tests; make

install: $(AOUT)
	strip dbx
	${INSTALL} -u bin -g bin -m 755 -F /usr/bin \
	-idb "root upd" dbx

#
# Create a tar file called "tape" containing relevant files.
#

TAPE = tape

tape:
	@tar cf ${TAPE} \
	    Makefile History version READ_ME ${SRC} \
	    makedefs.c mkdate.c tests pchanges ptests

#
# Header dependencies are purposely incomplete since header files
# are "written" every time the accompanying source file changes even if
# the resulting contents of the header don't change.  The alternative is
# to force a "makedefs" to be invoked for every header file each time dbx
# is made.
#
# Also, there should be a dependency of scanner.o and keywords.o on y.tab.h
# but misfortunately silly make does a "makedefs y.tab.c y.tab.h" which
# destroys y.tab.h.
#

symbols.o tree.o check.o eval.o events.o: operators.h
