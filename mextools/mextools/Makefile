all:
	cd include; make install
	cd imglib; make
	if test '$(machine)' = iris ;\
		then CPUBOARD=PM2; export CPUBOARD ; cd portlib; make ;\
		else CPUBOARD=IP2; export CPUBOARD ; cd portlib; make ; fi
	cd imgtools; make
	if test '$(machine)' = iris ;\
		then CPUBOARD=PM2; export CPUBOARD ; cd tools; make ;\
		else CPUBOARD=IP2; export CPUBOARD ; cd tools; make ; fi

clean:
	cd imglib; make clean
	cd portlib; make clean
	cd imgtools; make clean
	cd tools; make clean

install: 
	cd include; make install
	cd fonts; make install
	cd images; make install
	cd imglib; make install
	cd mexrcs; make install
	cd portlib; make install
	cd imgtools; make install
	cd tools; make install
