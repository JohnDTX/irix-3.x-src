#! /bin/sh -e
#
# Generate all the various kernels
#
#

if test $# = 0; then
	echo "usage: genall.sh product-typte"
	exit 1
fi
type=$1
if test -z "$type"; then
	type=3000
fi


CONFILES="`echo proto* files* makefile* devices*`"
for i in tcp nfs xns; do
	sys="${type}.${i}"
	make "CONFILES=$CONFILES" "SUBTYPE=${sys}" \
		../${sys}/makefile
	cd ../${sys}
	make incdepend
	make default vmunix.a "TYPE=${type}" "CLASS=${i}" "SYS=${sys}"
	rm -f ../kernels/${sys}
	ln vmunix ../kernels/${sys}
	cd ../conf
done
