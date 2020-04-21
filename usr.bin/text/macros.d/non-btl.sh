#! /bin/sh
#  text-update  --  shell command to remove BTL specific macros from MM
#
#  @(#)non-btl.sh	1.1


cd /usr/src/cmd/text/macros.d

if touch -c /usr/lib/macros/mmn mmn.src   2> /dev/null
     then echo "Modifying the macro packages\n"
     else echo "You must be logged in as root to modify the macros"
	  exit 1
fi

for i in n t
do
ed - mm${i}.src <<!
/E.NOCOMPACT/i
.rm PM CS TM )X }2\" NON-BTL INSTALLATION
.
w
q
!
done


echo "Re-installing the macro packages\n"

make -f macros.mk insmmn insmmt


echo "MM modified for non-BTL use."
