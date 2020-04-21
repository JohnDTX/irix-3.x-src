#
# This file, when sourced under csh, sets various environment variables so
# that makes can be done within the sgi (IRIS) source tree, without the
# use of Mk.sh.
#
# There are a couple of environment variables which must have already
# been set, possibly in your .cshrc file:
#
#	ROOT	- tree where "make install" will put things
#	SRCROOT - source tree in which makes are to be done
#   The following are only necessary if distribution images are built:
#	TROOT	- location of temporary root tree created by mkimage
#	DIST	- directory where cpio dist images go
#
setenv DISTFS bullet:/d/tmp		# what's this??? (dcs)
setenv TOOLROOT /
setenv GLROOT /usr
setenv machine juniper
setenv GLHDWR gl2
setenv MAKEFLAGS bk
setenv BUILDER $USER
setenv DTERM /term
setenv HOST J
setenv PROD 3000
#   The following are only necessary if distribution images are built:
setenv rbase $TROOT
setenv sbase $SRCROOT
setenv dest $DIST
setenv idb $sbase/idb
