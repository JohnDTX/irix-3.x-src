
dirs = gl2 fgl2  tpe2 rgl2 frgl2 pgl2
idirs = gl2/include


all default install clean:
	@GLHDWR=gl2; export GLHDWR;\
	    for d in $(dirs);  do \
		echo "====/gl2/$$d"; cd $$d ; $(MAKE) $@; cd ..; done

Include:
	@GLHDWR=gl2; export GLHDWR;\
	     for d in $(idirs);  do \
		echo "====/gl2/$$d"; cd $$d ; $(MAKE) Include; cd ..; done

