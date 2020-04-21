
M	= u
S	= 3
all clean clobber:

install:
	cp *.3g $(DEST)/usr/man/$M_man/man3
	cd $(DEST)/usr/man/$M_man/man$S;chmod 644 *.3g
	cd $(DEST)/usr/man/$M_man/man$S;chgrp bin *.3g
	cd $(DEST)/usr/man/$M_man/man$S;chown bin *.3g

FRC:
