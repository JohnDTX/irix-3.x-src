
all default:
	@if test "$$machine"x = "juniper"x; then \
		cd ip2; make; \
	fi
	@if test "$$machine"x = "iris"x; then \
		cd pm2; make; \
	fi
	@if test "$$machine"x = "PM2GL1"x; then \
		cd pm2; make; \
	fi
	cd revnetgroup; make

boot install:
	@if test "$$HOST"x = "J"x; then \
		cd ip2; make $@; \
	fi
	@if test "$$HOST"x = "I"x; then \
		cd pm2; make $@; \
	fi
	cd revnetgroup; make $@

clean clobber:
	cd ip2; make $@
	cd pm2; make $@
	cd revnetgroup; make $@
