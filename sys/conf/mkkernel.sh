#! /bin/sh
#
# Reconfigure a kernel
#
usage="usage: mkkernel [-s system-name] [-c class] [-t type]"

# Set defaults
SYS=TEST
TYPE=`uname -t`
CLASS=tcp

while test $# != 0;
do
	case $1 in
	  -s)
		shift; SYS="$1"
		;;
	  -c)
		shift; CLASS="$1"
		;;
	  -t)
		shift; TYPE="$1"
		;;
	   *)
		echo 'mkkernel: unknown flag "'$1'"'
		echo $usage
		exit 255
		;;
	esac
	shift
done

# Make sure arguments were proper
if test -z "$SYS"; then
	echo "mkkernel: incorrectly specified system"
	echo $usage
	exit 255
fi
if test -z "$TYPE"; then
	echo "mkkernel: incorrectly specified type"
	echo $usage
	exit 255
fi
if test -z "$CLASS"; then
	echo "mkkernel: incorrectly specified class"
	echo $usage
	exit 255
fi

# Translate product type into real type
case $TYPE in
  3*|2400T|2500T)
	TYPE="3000"
	;;
  2*)
	TYPE="2000"
	;;
  1*)
	TYPE="1000"
	;;
  *)
	echo 'mkkernel: unknown product type "'$TYPE'"'
	echo "legal types are: 3000 2000 3010 3020 3030 2400 2400T 2500T"
	exit 255
esac

# Make sure class type is known
case $CLASS in
  nfs|tcp|xns)
	;;
  *)
	echo 'mkkernel: unknown class "'$CLASS'"'
	echo "legal classes are: nfs tcp xns"
	exit 255
	;;
esac

# Make directory, if its not there
if test ! -d ../$SYS; then
	mkdir ../$SYS
fi

# Create the configuration file, if its not there
if test ! -r $SYS; then
	cp $TYPE.$CLASS $SYS
	echo 'Now edit the configuration file "'$SYS'"'
	exit 0
fi

# Configure the kernel
./config -b $SYS

# Make the kernel
cd ../$SYS
make binary TYPE=$TYPE CLASS=$CLASS SYS=$SYS
