# Make (non-V-kernel) graphics terminal, wsiris

DIRS = wsterm wsinteg

install clobber clean all default:
	@for d in $(DIRS) ; do echo "==== gl2/tpe2$$d"; \
		cd $$d; make $@; cd .. ; done
