#
#
include ${ROOT}/usr/include/make/commondefs
#
IDBT = -idb "std.sw.unix mr"
LIBS=	libgl.a libgl_p.a libkgl.a libkgl_p.a

all: objs pobjs kglobjs kglpobjs
	cd objs; make -f ../Make.gl PREF= CPROF=
	cd pobjs; make -f ../Make.gl PREF=_p CPROF=-p
	cd kglobjs; make -f ../Make.kgl PREF= CPROF=
	cd kglpobjs; make -f ../Make.kgl PREF=_p CPROF=-p

objs pobjs kglobjs kglpobjs:
	mkdir $@

clobber:	clean
	rm -f ${LIBS}

clean:
	rm -rf objs pobjs kglobjs kglpobjs

install:	all
	${INSTALL} -src libkgl.a -F /usr/lib ${IDBT} libkgl3.a
	${INSTALL} -src libgl_p.a -F /usr/lib ${IDBT} libgl2_p.a
	${INSTALL} -src libkgl_p.a -F /usr/lib ${IDBT} libkgl3_p.a
	${INSTALL} -F /usr/lib ${IDBTAG} libgl.a
	${INSTALL} -ln /usr/lib/libgl.a -F /usr/lib libgl2.a
	ranlib ${ROOT}/usr/lib/libgl.a
	ranlib ${ROOT}/usr/lib/libkgl3.a
	ranlib ${ROOT}/usr/lib/libgl2_p.a
	ranlib ${ROOT}/usr/lib/libkgl3_p.a
