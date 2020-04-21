#! /bin/sh
# NAME
#	Mk - make wrapper for building SVR3 software
# SYNOPSIS
#	Mk [variable=value] [builtin] [directory[:target]] ...
# DESCRIPTION
#	See the manual page, Mk.1.
#
me=`basename $0 .sh`
cwd=`pwd`
cpwd=`dirname $cwd`

# for each argument, do this command
ITERATOR=:

# issue a message containing arguments and starting time
case $# in
  0)	set default
esac
echo "$0 of $* on \c"; date '+%a %h %d %H:%M'

# set the build's environment by pushing back variable assigment arguments
set ROOT=${ROOT:-} "$@"
set SRCROOT=${SRCROOT:-$cwd} "$@"
set GLROOT=${GLROOT:-/usr} "$@"
set TOOLROOT=${TOOLROOT:-$cpwd/tools} "$@"
set machine=${machine:-juniper} "$@"
set GLHDWR=${GLHDWR:-gl2} "$@"
set MAKEFLAGS=${MAKEFLAGS:-bk} "$@"
set BUILDER=${USER:-andre} "$@"
set DTERM=${DTERM:-/term} "$@"
set HOST=${HOST:-J} "$@"
set PROD=${PROD:-3000} "$@"
set PRODUCT=${PRODUCT:-3000} "$@"
set BOOT=${BOOT:-xxxx} "$@"		# default is no drive to avoid trashing
					# anything
case "$HOSTENV" in
  mipsbsd)
	set TOOLROOT=${TOOLROOT:-$cpwd/tools} "$@"
	;;
  *)
	set TOOLROOT=$TOOLROOT "$@"
esac

case "$machine" in
  iris)
	set processor=10 "$@"
	;;
  *)
	set processor=20 "$@"
esac

set SHELL=$TOOLROOT/bin/sh "$@"	# *very* important commondefs definition
# Note that $TOOLROOT in the above line will NOT be overridden by any
# "TOOLROOT=whatever" occurrances on the command line - observance by dcs

set TROOT=${TROOT:-/troot} "$@"	# temporary root tree for making distribution
set DIST=${DIST:-/dist} "$@"	# directory where cpio dist images go
set DISTFS=bullet:/d/tmp "$@"	# what's this ?  -  dcs
set XHOST=foist "$@"
set XUSER=guest "$@"		# mount and umount must suid-root on XHOST
set REALIDB=$cwd/idb "$@"	# same directory as Mk is run from

# timestamp files for incremental header installation - the .Stamp suffix
# convention is understood by case(inchead|incsyshead) below.
hstamp=$cwd/head.Stamp
syshstamp=$cwd/syshead.Stamp

# the following are coupled, unfortunately
ksubdir=uts/mips
ksyshdirs="sys bsd/net bsd/netinet bsd/netns"
syshdirs="$ksubdir/sys $ksubdir/bsd/net $ksubdir/bsd/netinet $ksubdir/bsd/netns"

while test $# -gt 0; do
	time=`date '+%H:%M'`

	# peel off the optional target from $arg
	arg=`expr "$1" : '\(.*\):.*'`
	if test -z "$arg"; then
		arg="$1"
	fi
	targ=`expr "$1" : '.*:\(.*\)'`
	sarg="$1"	# save full argument
	shift

	# do user-defined function, passing the argument and its timestamp
	eval $ITERATOR $sarg $time

	# environment and shell variables -- these are set from the command
	# line so that arguments can override envariables and so that dependent
	# definitions (e.g. include and minclude for ROOT) can be derived in
	# one place -- the inner case statement below
	case "$arg" in
	  [A-Za-z_]*=*)
		var=`expr "$sarg" : '\([^=]*\)=.*'`
		val=`expr "$sarg" : '[^=]*=\(.*\)'`
		eval $var='"$val"'
		case $var in
		  IDB)
			if test `expr "$IDB" : '/.*'` -eq 0; then
				IDB=$cwd/$IDB
			fi
			;;
		  ROOT)
			include=$ROOT/usr/include
			minclude=$include/make
			if test ! -d /$ROOT; then
				rm -f $ROOT
				set rootboot "$@"
			fi
			idb=$ROOT/etc/idb	# default built idb
			;;
		  TOOLROOT)
			install=$TOOLROOT/etc/Install
			make=$TOOLROOT/bin/make
		esac
		case $var in
		  ITERATOR|XHOST|XUSER)
			;;
		  *)
			export $var
		esac
		;;

	# builtins for common targets
	  default)
		    set install \
		    $*
		;;

	  install)
		# XXX hacked for ESDI systems to clean cmd
		 set rootboot dirs \
			FILES:$arg include:$arg lib/libc:$arg usr.lib:$arg \
			usr.bin/fortran$processor:$arg  sun/usr.lib:$arg \
			bsd/usr.lib:$arg lib:$arg bsd:$arg sun:$arg \
			xns:$arg gl2:$arg mextools:$arg usr.bin:$arg \
			usr.etc:$arg sys:$arg stand:$arg gpib:$arg \
			pub:$arg etc:$arg bin:$arg dev:$arg mex:$arg \
			games:$arg gifts.gl2:$arg demos.gl2:$arg \
			test/misc:$arg \
		    $*

		;;

	  depend|incdepend|clean|clobber|purge|fluff|tags)
		subs=
		for f in *; do
			case $f in
			  include|man|test|copywrites|diag|dist|dev)
				continue
			esac
			if test -d $f; then
				subs="$f:$arg $subs"
			fi
		done
		set -- $subs "$@"
		;;

	  idb)
		# IDB if set names a raw idb to which install appends records
		# idb if set names a cooked idb for donl's tools
		# XXX apply occam's razor
		if test -f $idb; then
			mv -f $idb $idb.prev
		fi
		rawidb=/usr/tmp/rawidb.$$
		if (IDB=$rawidb $0 install &&
		    sh swtools/idbcanon.sh $rawidb > $idb &&
		    rm -f $rawidb) then
			echo "+++ $arg made in $idb"
		else
			echo "--- $arg make failed in $idb"
		fi
		;;

	  images)
		rbase=$TROOT; export rbase
		sbase=$SRCROOT; export sbase
		idb=$SRCROOT/idb # ; export idb
		dest=$DIST; export dest
		specdir=$SRCROOT/dist
		echo "=== $sarg $specdir $time"
		if test ! -d $DIST; then
			rm -rf $DIST; mkdir $DIST
		fi
		if (cd $specdir; $make)
		then
			echo "+++ images made in /dist"
		else
			echo "--- images make failed in /dist"
		fi
		;;

	  booti)
		cd dist/tools
		./newboot -m $machine -d $BOOT -r $ROOT -e $DIST \
			-i $SRCROOT/idb -s $SRCROOT
		;;

	  log)
		logdir=$cwd/../logs
		if test ! -d $logdir; then
			rm -f $logdir; mkdir $logdir
		fi
		while :; do
			logfile=$logdir/`date +%m%d%H%M`
			if test ! -f $logfile; then
				break
			fi
			sleep 1
		done
		curlog=$logdir/curlog
		rm -f $curlog; ln -s $logfile $curlog
		2>&1 $0 $* > $logfile
		chmod 666 $logfile
		chmod 666 $curlog
		echo "Log file is $logfile"
		break
		;;

	# tree maintenance builtins
	  rcsync)
		# XXX parameterize brendan and cifix (aka rcsync)
		rsh jake -l brendan "find $SRCROOT -name RCS -print | \
		    xargs ~brendan/bin/cifix -v"
		;;

	  tclean)
		tlink -cv \
		    -x '^Mk$' \
		    -x '^Mk\..*$' \
		    -x '^.*made$' \
		    -x '^.*Stamp$' \
		    $SRCROOT $cwd
		;;

	  tlink)
		tlink -v -x '^.*\.[ao]$' $SRCROOT $cwd
		# remove the fluff tlink may have created
		rm -rf $SRCROOT/usr.bin/awk/token.c
		rm -rf $SRCROOT/usr.bin/lint/llib.l$GLHDWR.ln
		rm -rf $SRCROOT/usr.bin/lint/llib.l$GLHDWR
		;;

	  tsetup)
		tlink -v -x '^.*\.[ao]$' $SRCROOT $cwd \
		    cmd/install head $ksubdir $*
		set rootboot inchead incsyshead
		;;

	# snapshot operations
	  snap)
		(cd $SRCROOT
		    find . ! -type d ! -name '*,v' -print |
		    cpio -paudvm $cwd)
		;;

	  trunk)
		trunk=$cpwd/trunk
		echo "=== trunk $trunk $time"
		rm -rf $trunk
		mkdir $trunk $trunk/src
		cp $0 $trunk/src
		(cd $SRCROOT
		    find head $ksubdir ! -type d ! -name '*,v' -print |
		    cpio -paudvm $trunk/src)
		;;

	# single and double level bootstrap builtins
	  boot)
		set rootboot dirs \
		    FILES:install include:install lib/libc:install \
		    usr.lib:install usr.bin/fortran$processor:install \
		    sun/usr.lib:install bsd/usr.lib:install lib:install \
		    xns/lib:install xns/multicast:install \
		    gl2:install dev:install \
		    mextools/mextools/imglib:install \
		    mextools/mextools/portlib:install \
		    demos.gl2:install \
		    $*
		;;

	  boot2)
		set boot \
		    lib:clobber cmplrs:clobber \
		    lib:install cmplrs:install \
		    $*
		;;

	# internal builtins for ad-hoc bootstrapping
	  rootboot)
		# XXX We may need commondefs for makefiles which precede head,
		# XXX sys*list in $ROOT/etc, and install in $TOOLROOT/etc.
		dp=
		for d in `echo $minclude | tr '/' ' '`; do
			dp=$dp/$d
			if test ! -d $dp; then
				rm -f $dp; mkdir $dp
			fi
		done
		if test ! -d ${ROOT}/etc ; then
		    mkdir ${ROOT}/etc
		fi
		cp ${SRCROOT}/FILES/sysdirlist ${ROOT}/etc
		cp ${SRCROOT}/include/make/${PRODUCT}defs $minclude/${PRODUCT}defs
		ln -s $minclude/${PRODUCT}defs $minclude/defs
		cp ${SRCROOT}/include/make/commondefs $minclude/commondefs
		cp ${SRCROOT}/include/make/commonrules $minclude/commonrules
		if test ! -f /usr/lib/libp.a || \
			test ! -f /usr/lib/pascal || \
			test ! -f /bin/as ; then
			echo "Sorry ... The following files must exist in order to do a build "
			echo "		/usr/lib/libp.a "
			echo "		/usr/lib/pascal "
			echo "		/bin/as "
			exit 255
		fi

		;;

	  toolboot)
		# XXX Not all commondefs tools have been installed under
		# XXX $TOOLROOT at this point -- echo, ed, make, sh, get, lex,
		# XXX lorder, m4, and yacc are missing.  Rather than wasting
		# XXX time and space installing them, we just link them in
		# XXX from $ROOT.  This assumes they don't change much.
		if test -z "$TOOLROOT"; then
			echo "toolboot: TOOLROOT null or unset"
			continue
		fi
		for prog in echo ed make sh; do
			rm -f $TOOLROOT/bin/$prog
			ln -s /bin/$prog $TOOLROOT/bin/$prog
		done
		for prog in get lex lorder m4 yacc; do
			rm -f $TOOLROOT/usr/bin/$prog
			ln -s /usr/bin/$prog $TOOLROOT/usr/bin/$prog
		done
		;;

	# builtins for directory and header installation
	# XXX cmd/install/idbcanon.sh knows how to handle the __SYSDIRLIST__
	# XXX idb attribute
	  dirs)
		sysdirlist=$ROOT/etc/sysdirlist
		baddir=no
		if test ! -r $sysdirlist; then
			echo "%%% cannot read $sysdirlist."
			continue
		fi
		if test -z "$IDB"; then
			cat $sysdirlist |
			while read dir; do
				if test ! -d $ROOT$dir; then
					echo $dir
				fi
			done
		else
			cat $sysdirlist
		fi | xargs |
		while read dirs; do
			$install -dir -idb __SYSDIRLIST__ "$dirs"
			if test $? -ne 0; then
				baddir=yes
			fi
		done
		if test $baddir = no; then
			echo "+++ dirs made"
		else
			echo "--- dirs make failed"
		fi
		;;

	  head)
		idbtag="-idb std.sw.cc"
		insdir=/usr/include
		findargs="-follow -type f
		    ! -name *,v ! -name *.c ! -name sys.*
		    ! -name *.o ! -name *.a ! -name .emacs*
		    ! -name *.mk ! -name Make* ! -name Stamp"
		# XXX above should not -follow with only -type f, but rather
		# XXX should read: ( -type f -o -type l )
		if test -z "$IDB" -a -f "$hstamp"; then
			findargs="-newer $hstamp $findargs"
		fi

		glcompat="get.h device.h gl.h fget.h fdevice.h fgl.h"

		if (cd head; echo "=== head head $time"
		    find . $findargs -print |
			sed -e 's@^\./@@' -e '/gl\/template/d' | xargs |
			while read headers; do
			    $install -F $insdir -m 664 $idbtag "$headers"
			done
		    $install -src sys.mips -F $insdir -m 664 $idbtag sys.s
		    cd gl/template; $make install
		    for f in $glcompat; do
			$install -lns $insdir/gl/$f -F $insdir $idbtag $f
		    done) then
			echo "+++ head made"
		else
			echo "--- head make failed"
		fi
		;;

	  syshead)
		idbtag="-idb std.sw.cc"
		findargs="-follow -type f -name *.h"
		# XXX replace -follow with -type l or -type f
		if test -z "$IDB" -a -f "$syshstamp"; then
			findargs="-newer $syshstamp $findargs"
		fi

		if (cd $ksubdir; echo "=== syshead $ksubdir $time"
		    find $ksyshdirs $findargs -print | xargs |
		    while read headers; do
			$install -F /usr/include -m 664 $idbtag "$headers"
		    done) then
			echo "+++ syshead made in $ksubdir"
		else
			echo "--- syshead make failed in $ksubdir"
		fi
		;;

	  inchead|incsyshead)
		touch $arg
		htype=`expr $arg : 'inc\(.*\)'`
		if $0 $htype; then
			# XXX maybe save previous timestamp?
			mv -f $arg $cwd/$htype.Stamp
		else
			rm -f $arg
		fi
		;;

	# the graphics peripherals code, which must be cross-compiled into
	# our tree via remote shell on a 68020 which NFS-mounts us
	  periph)
		echo "=== $sarg $arg $time"
		case "$targ" in
		  install)
			xtarg= local=yes
			;;
		  *)
			xtarg=$targ local=no
		esac

		hostname=`hostname`
		buildmnt="IRIS4Dbuild"
		rsh $XHOST -l $XUSER <<- __ENDREMOTE__
			if mount | grep -s $buildmnt; then
				umount $buildmnt
			fi
			rm -rf $buildmnt; mkdir $buildmnt
			if mount $hostname:$cpwd $buildmnt; then
				cd $buildmnt/src/$arg
				make -f $arg.mk $xtarg
			fi
		__ENDREMOTE__

		if test $? -eq 0 -a $local = yes &&
		    (cd $arg; $make -f $arg.mk $targ) then
			echo "+++ $arg${targ:+:}$targ made in $arg"
		else
			echo "--- $arg${targ:+:}$targ make failed in $arg"
		fi
		;;

	# an argument of the form directory[:target]
	  *)
		# handle *.skip and non-directory cruft
		if test `expr $arg : '.*\.skip'` -gt 0; then
			continue
		fi
		if test ! -d $arg; then
			echo "%%% $arg is not a directory."
			continue
		fi

		# determine makefile name, recurring for cases such as cmd
		# where intervening directories contain no makefile
		mkfile=
		for suf in "" .d .ENGR; do
			base=`basename $arg $suf`
			if test -r $arg/$base.mk; then
				mkfile="-f $base.mk"
				break
			fi
		done
		if test -z "$mkfile" -a \
		    ! -r $arg/Makefile -a ! -r $arg/makefile; then
			cd $arg
			subs=
			for f in *; do
				if test -d $f; then
					subs="$subs $arg/$f${targ:+:}$targ"
				else
					echo "%%% $arg/$f: cannot make $targ"
				fi
			done
			set -- $subs "$@"
			cd $cwd
			continue
		fi

		# issue the make command
		tbase=$base${targ:+:}$targ
		if (cd $arg; echo "=== $tbase $arg $time"
		    $make $mkfile $targ) then
			echo "+++ $tbase made in $arg"
		else
			echo "--- $tbase make failed in $arg"
		fi
	esac
done
