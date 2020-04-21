include $(ROOT)/usr/include/make/commondefs

DIRS = libidb cmd

default install clean clobber rmtargets:
	for f in $(DIRS) ; do ( cd $$f ; $(MAKE) -f $$f.mk $@ ) done
