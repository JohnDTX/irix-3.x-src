#! /bin/sh
# set -v
# set -x
#	INSTALL COMMAND
#	supports both system 5 and bsd4.2 fetures
# $Source: /d2/3.7/src/etc/RCS/install.sh,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 15:38:02 $
FLIST=/etc/syslist
DEFAULT="/bin /usr/bin /etc /lib /usr/lib" FOUND="" MOVOLD=""
ECHO=echo PATH=:/bin:/usr/bin:/etc FLAG=off MODE=755 OWNER=bin GROUP=bin CMD=cp
USAGE="eval echo 'usage: install [options] file [dir1 ...]'; exit 2"

# debugging flag
if [ "$1" = "-d" ]
then
	debug=debug
	echo "Debugging turned on"
	shift
fi

# handle bsd4.2 context switch
sys=BSD42
if [ -f /etc/inittab ]
then
	sys=SYS5
	if [ "$debug" != "" ]
	then
		echo "I see /etc/inittab so we are defaulting to System 5"
	fi
	realsys=SYS5
fi
if [ "$INSTALLSYS" != "" ]
then
	sys=$INSTALLSYS
	if [ "$debug" != "" ]
	then
		echo 'I see $INSTALLSYS being' $INSTALLSYS
	fi
fi
if [ "$1" = "-U" ]
then
	sys=$2
	if [ "$debug" != "" ]
	then
		echo "I see -U $2"
	fi
	shift; shift
fi
if [ $sys = BSD42 ]
then
	if [ "$debug" != "" ]
	then
		echo "We are bsd 4.2"
	fi
	if [ "$realsys"  = SYS5 ]
	then
		owner=`id | sed 's/[^(]*(\([^)]*\).*/\1/'`
	else
		owner=`whoami`
	fi
	if [ "$owner" = root ]
	then
		OWNER="root"
	else
		OWNER=""
	fi
	GROUP=staff
	CMD=/bin/mv
	force=force
	ECHO=:
	USAGE="eval echo 'usage: install [options] file destination'; exit 2"
fi

for i in $*
do
	if [ "$debug" != "" ]
	then
		echo "Parsing $i"
	fi
# see if we should skip past the parameter of last flag since FOR's
# arg list isn't affected by shifts
	if [ $FLAG = on ]
	then
		case $i in
		    -*) echo "install: The -c, -f, -n options each require \c"
			echo "a directory following!"
			exit 2;;
		     *) FLAG=off
			if [ "$debug" != "" ]
			then
				echo "Skipping past parameter"
			fi
			continue;;
		esac
	fi
	case $i in
	    -c)
		if [ $sys = BSD42 ]
		then
			if [ "$debug" != "" ]
			then
				echo "Handling BSD's -c"
			fi
			CMD=/bin/cp
			shift
		else
			if [ "$debug" != "" ]
			then
				echo "Handling SYS5's -c"
			fi
			if [ x$ARG = x-f -o x$arg = x-i -o x$arg = x-o -o x$arg = x-n ]
			then
				echo "install: -c dir: illegal option with \c"
				echo "${arg-"-f"} option!"
				exit 2
			elif [ "$2" = "" ]
			then
				echo "install: -c option must have argument"
				exit 2
			else
				direct=$2
				FLAG=on
				ARG=-c
				shift; shift
			fi
		fi;;
	    -f)
		if [ x$ARG = x-c -o x$arg = x-i -o x$arg = x-n ]
		then
			echo "install: -f dir: illegal option with \c"
			echo "${arg-"-c"} option!"
			exit 2
		elif [ "$2" = "" ]
		then
			echo "install: -f option must have argument"
			exit 2
		else
			direct=$2
			FLAG=on
			ARG=-f
			shift; shift
		fi;;
	    -i) if [ x$ARG  = x-c -o x$ARG = x-f ]
		then
			echo "install: -i: illegal option with $ARG option!"
			exit 2
		elif [ "$2" = "" ]
		then
			echo "install: -i option requires argument"
			exit 2
		else
			DEFAULT=""
			arg=-i
			shift
		fi;;
	    -o)
		if [ $sys = BSD42 ]
		then
			if [ "$debug" != "" ]
			then
				echo "Handling BSD's -o"
			fi
			OWNER="$2"
			force=force
			FLAG=on
			shift; shift
		else
			if [ "$debug" != "" ]
			then
				echo "Handling SYS5's -o"
			fi
			if  [ x$ARG = x-c ]
			then
				echo "install: -o: illegal option with $ARG \c"
				echo "option!"
				exit 2
			elif [ "$2" = "" ]
			then
				$USAGE
			else
				MOVOLD=yes
				arg=-o
				shift
			fi
		fi;;
	    -n) if [ x$ARG = x-c -o x$ARG = x-f ]
		then
			echo "install: -n dir: illegal option with $ARG option!"
			exit 2
		elif [ "$2" = "" ]
		then
			echo "install: -n option requires argument"
			exit 2
		else
			LASTRES=$2
			FLAG=on
			FOUND=n
			arg=-n
			shift; shift
		fi;;
	    -s)
		if [ $sys = BSD42 ]
		then
			strip=strip
			shift
		else
			if [ "$2" = "" ]
			then
				$USAGE
			else
				ECHO=:
				arg=-s
				shift
			fi
		fi;;
	     -m)
		MODE=$2
		force=force
		FLAG=on
		shift; shift;;
	     -u)
		OWNER="$2"
		force=force
		FLAG=on
		shift; shift;;
	     -g)
		GROUP=$2
		force=force
		FLAG=on
		shift; shift;;
	     -U)
		echo "install: -U must be first arg"
		exit 2;;
	     -F)
		force=force
		shift;;
	     -S)
		strip=strip
		shift;;
	     *)
		if [ "$debug" != "" ]
		then
			echo "PARSING $i: no more flags"
		fi
		break;;
	esac
done

FILEP=$i FILE=`echo $i | sed -e "s+.*/++"`

if [ "$debug" != "" ]
then
	echo "Done Parsing"
	echo "Copying from $FILEP to $FILE in directory $direct"
fi

# handle bsd4.2 context switch
if [ $sys = BSD42 ]
then
	if [ "$debug" != "" ]
	then
		echo "Doing context switch to BSD"
	fi
	if [ "$2" = "" ]
	then
		echo "install: no destination specified"
		exit 1
	fi
	if [ "$3" != "" ]
	then
		echo "install: too many files specified -> $*"
		exit 1
	fi
	if [ "$1" = "$2" -o "$2" = "." ]
	then
		echo "install: can't move $1 onto itself"
		exit1
	fi
	ARG=-f
	if [ ! -d $2 ]
	then
		if [ "$debug" != "" ]
		then
			echo "BSD: dest contains file name: strip it off"
		fi
		direct=`echo $2 | sed 's+/[^/]*$++'`
		FILE=`echo $2 | sed -e "s+.*/++"`
		if [ "$debug" != "" ]
		then
			echo "After SED:copying $FILEP to $FILE in dir $direct"
		fi
		if [ "$direct" = "" ]
		then
			if [ "$debug" != "" ]
			then
				echo "Installing in /"
			fi
			direct="/"
		fi
		if [ "$direct" = "$2" ]
		then
			if [ "$debug" != "" ]
			then
				echo "Installing in ."
			fi
			direct="."
		fi
	else
		direct=$2
	fi
	if [ "$owner" != root ]
	then
		GROUP=`ls -ldg $direct | sed 's/.............\([^ ]*\).*/\1/'`
	fi
	if [ "$debug" != "" ]
	then
		echo "BSD: installing $1 in dir $direct"
	fi
fi

if [ "$debug" != "" ]
then
	echo "Done doing possible BSD context switch"
fi

if [ x$ARG = x-c -o x$ARG = x-f ]
then
	case $2 in
		-*) $USAGE ;;
		"") :	;;
	esac
	if [ $sys = BSD42 -a -f $direct/$FILE ]
	then
		rm -f $direct/$FILE
	fi
	if [ -f $direct/$FILE -o -f $direct/$FILE/$FILE ]
	then
		case $ARG in
			-c) echo "install: $FILE already exists in $direct"
			    exit 2;;
			-f) if [ -k $direct/$FILE ]
			    then
				chmod -t $direct/$FILE
				if [ $? != 0 ]
				then
					echo "install: chmod failed"
					exit 2
				fi
				$direct/$FILE < /dev/null > /dev/null
				tbit=on
			    fi
			    if [ "$debug" != "" ]
			    then
				echo "Handling -f style install"
			    fi
			    if [ "$MOVOLD" = yes ]
			    then
				mv $direct/$FILE $direct/OLD$FILE
				cp $direct/OLD$FILE $direct/$FILE
				if [ $? = 0 ]
				then
				   $ECHO "$FILE moved to $direct/OLD$FILE"
				else
				   echo "install: $CMD $direct/OLD$FILE \c"
				   echo "$direct/$FILE failed"
				   exit 2
				fi
			    fi
			    if $CMD $FILEP $direct/$FILE
			    then
				$ECHO "$FILEP installed as $direct/$FILE"
				if [ "$force" != "" ]
				then
					if [ "$strip" != "" ]
					then
						strip $direct/$FILE
					fi
					chmod $MODE $direct/$FILE
					if [ $? != 0 ]
					then
						echo "install: chmod failed"
						exit 2
					fi
					chgrp $GROUP $direct/$FILE
					if [ $? != 0 ]
					then
						echo "install: chgrp failed"
						exit 2
					fi
					if [ $sys != BSD42 -o "$owner" = root ]
					then
						chown $OWNER $direct/$FILE
						if [ $? != 0 ]
						then
						  echo "install: chown failed"
						  exit 2
						fi
					fi
			    	fi
			    else
				echo "install: $CMD failed"
				exit 2
			    fi
			    if [ "$tbit" = on ]
			    then
				chmod +t $direct/$FILE
				if [ $? != 0 ]
				then
					echo "install: chmod failed"
					exit 2
				fi
			    fi
			    exit;;
		esac
	else
		$CMD $FILEP $direct/$FILE
		if [ $? = 0 ]
		then
			$ECHO "$FILEP installed as $direct/$FILE"
			if [ "$strip" != "" ]
			then
				strip $direct/$FILE
			fi
			chmod $MODE $direct/$FILE
			if [ $? != 0 ]
			then
				echo "install: chmod failed"
				exit 2
			fi
			chgrp $GROUP $direct/$FILE
			if [ $? != 0 ]
			then
				echo "install: chgrp failed"
				exit 2
			fi
			if [ $sys != BSD42 -o "$owner" = root ]
			then
				chown $OWNER $direct/$FILE
				if [ $? != 0 ]
				then
					echo "install: chown failed"
					exit 2
				fi
			fi
		else
			echo "install: $CMD failed"
			exit 2
		fi
	fi
	exit
fi

shift

if [ "$debug" != "" ]
then
	echo "No directory specified, doing search"
fi

PUTHERE=""
for i in $*
do
	case $i in
		-*) $USAGE ;;
	esac
	PUTHOLD=`find $i -name $FILE -type f -print`
	PUTHERE=`expr "\`echo $PUTHOLD\`" : '\([^ ]*\)'`
	if [ "$PUTHERE" != "" ]
	then break
	fi
done
if [ -r $FLIST -a "$PUTHERE" = "" ]
then
	PUTHERE=`grep "/${FILE}$" $FLIST | sed  -n -e '1p'`
fi
if [ "$PUTHERE" = "" ]
then
	for i in $DEFAULT
	do
		PUTHOLD=`find $i -name $FILE -type f -print`
		PUTHERE=`expr "\`echo $PUTHOLD\`" : '\([^ ]*\)'`
		if [ "$PUTHERE" != "" ]
		then break
		fi
	done
fi
if [ "$PUTHERE" != "" ]
then
		    if [ -k $PUTHERE ]
		    then
			chmod -t $PUTHERE
			if [ $? != 0 ]
			then
				echo "install: chmod failed"
				exit 2
			fi
			$PUTHERE < /dev/null > /dev/null
			tbit=on
		    fi
		    if [ "$MOVOLD" = yes ]
		    then
			old=`echo $PUTHERE | sed -e "s+/[^/]*$++"`
			mv $PUTHERE $old/OLD$FILE
			cp $old/OLD$FILE $PUTHERE
			if [ $? = 0 ]
			then
			    $ECHO "old $FILE $CMD""ed to $old/OLD$FILE"
			    chgrp $GROUP $PUTHERE
			    if [ $? != 0 ]
			    then
				echo "install: chgrp failed"
				exit 2
			    fi
			else
			    echo "install: $CMD $direct/OLD$FILE \c"
			    echo "$direct/$FILE failed"
			    exit 2
			fi
		    fi
		    FOUND=y
		    if $CMD $FILEP $PUTHERE
		    then
			if [ "$force" != "" ]
			then
				if [ "$strip" != "" ]
				then
					strip $FILE
				fi
				chmod $MODE $FILE
				if [ $? != 0 ]
				then
					echo "install: chmod failed"
					exit 2
				fi
				chgrp $GROUP $FILE
				if [ $? != 0 ]
				then
					echo "install: chgrp failed"
					exit 2
				fi
				if [ $sys != BSD42 -o "$owner" = root ]
				then
					chown $OWNER $direct/$FILE
					if [ $? != 0 ]
					then
						echo "install: chown failed"
						exit 2
					fi
				fi
			fi
			if [ "$tbit" = on ]
			then
			    chmod +t $PUTHERE
			    if [ $? != 0 ]
			    then
				echo "install: chmod failed"
				exit 2
			fi
			fi
			$ECHO "$FILEP installed as $PUTHERE"
			break
		    else
			echo "install: $CMD failed"
			exit 2
		    fi
fi

case $FOUND in
	"") echo "install: $FILE was not found anywhere!"
	    exit 2;;
	 y) :	;;
	 n) $CMD $FILEP $LASTRES/$FILE
	    if [ $? = 0 ]
	    then
		$ECHO "$FILEP installed as $LASTRES/$FILE by default!"
		cd $LASTRES
		if [ "$strip" != "" ]
		then
			strip $FILE
		fi
		chmod $MODE $FILE
		if [ $? != 0 ]
		then
			echo "install: chmod failed"
			exit 2
		fi
		chgrp $GROUP $FILE
		if [ $? != 0 ]
		then
			echo "install: chgrp failed"
			exit 2
		fi
		if [ $sys != BSD42 -o "$owner" = root ]
		then
			chown $OWNER $FILE
			if [ $? != 0 ]
			then
				echo "install: chown failed"
				exit 2
			fi
		fi
	    else
		echo "install: $CMD failed"
		exit 2
	    fi;;
esac
