#! /bin/sh
#
#	@(#)suggest.sh	4.4	(Berkeley)	82/11/06
#
trap 'rm $$; exit' 1 2 3 15
D=/usr/lib/explain.d
while echo "phrase?";read x
do
cat >$$ <<dn
/$x.*	/s/\(.*\)	\(.*\)/use "\2" for "\1"/p
dn
case $x in
[a-z]*)
sed -n -f $$ $D; rm $$;;
*) rm $$;;
esac
done
