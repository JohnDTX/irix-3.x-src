#
# Makefile for constructing the IP2 standalone library and the proms 
#

all: lib cmd 

install:
	case $(machine) in \
		juniper)	 \
			cd lib; make install ;;\
		iris) \
			cd pm2mon ; make install ;;\
	esac
	cd cmd; make install
#	cd mon; make install

clean clobber:
	cd lib; make clean
	cd cmd; make clean
	cd mon; make clean
	cd pm2mon; make clobber

lib: anything
	cd lib; make

cmd: anything
	cd cmd; make

mon: anything
	cd mon; make

pm2mon: anything
	cd pm2mon; make

anything:
