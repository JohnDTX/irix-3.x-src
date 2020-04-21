AR	= ar

#
#
include ${ROOT}/usr/include/make/commondefs
LIB	= liby.a

LIBFILES= libmai.o libzer.o

all:	$(LIB)

install:all
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib \
	-idb "std.sw.unix" "${LIB}"
	ranlib ${ROOT}/usr/lib/"${LIB}"

$(LIB):	$(LIBFILES)
	rm -f $(LIB)
	$(AR) qv $(LIB) $(LIBFILES)
	ranlib $@

clean:
	rm -f *.o
	
clobber:clean
	rm -f $(LIB)
