#
# Makefile for window library

TARGETS= libwin.a libwin_dbx.a
#TARGETS= libwin_dbx.a

all: ${TARGETS}

clean:
	rm -rf objs dbxobjs

clobber: clean
	rm -f ${TARGETS}

objs dbxobjs:
	mkdir $@

libwin.a: objs anything
	cd objs; make -f ../Make.gen A0=../libwin.a A1="-O" A2=strip A3=""

libwin_dbx.a: dbxobjs anything
	cd dbxobjs; make -f ../Make.gen A0=../libwin_dbx.a A1="-g -DDEBUG" \
		A2=@true A3="malloc.o"

anything:
