#! /bin/csh
set off = (`strings -o $1 | grep '0123456789abcdef'`)
if ( $#off != 2 ) then
    echo error - "$off"
    exit 1
endif
@ doff = $off[1] + 10
(echo '?m 0 0x40000000 0';\
	echo '$d';\
	echo $doff'?w' "'AB'";\
	echo $doff'+2?w' "'CD'";\
	echo $doff'+4?w' "'EF'")\
	| adb -w $1 -
