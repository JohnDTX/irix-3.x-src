#
# Install binary files from the system root into a chroot area
#
BOOTPROGS = adb ed sed sh basename cat chgrp chmod chown cpio date \
	echo expr find grep false cmp tail true line \
	ls mkdir mv pwd rm rmdir size strings strip test touch

B	= $(ROOT)/bin

all:	install

install:
	rm -f $(ROOT)/bin/mv
	cd /bin; cp $(BOOTPROGS) $(ROOT)/bin
	cd $(ROOT)/bin ; /bin/rm -f cp ln ;\
		/bin/mv mv ln ; /bin/ln ln cp ; /bin/ln ln mv ;\
		/bin/chown root mv ; /bin/chgrp sys mv ; /bin/chmod 4755 mv
