#! /bin/sh
#
# @(#)ypinit.sh 1.1 86/02/05 Copyr 1984 Sun Microsystems, Inc.  
# @(#)ypinit.sh	2.1 86/04/16 NFSSRC
#
# ypinit.sh - set up a populated yp directory structure on a master server
# or a slave server.
#

# set -xv
maps="ethers.byaddr ethers.byname group.bygid group.byname \
hosts.byaddr hosts.byname mail.aliases netgroup netgroup.byuser \
netgroup.byhost networks.byaddr networks.byname passwd.byname \
passwd.byuid protocols.byname protocols.bynumber services.byname ypservers"
yproot_dir=/usr/etc/yp
hf=/tmp/ypinit.$$
XFR=${YPXFR-/usr/etc/yp/ypxfr}

masterp=F
slavep=F
host=""
def_dom=""
master=""
got_host_list=F
exit_on_error=F
errors_in_setup=F

PATH=$PATH:$yproot_dir
export PATH 

case $# in
1)	case $1 in
	-m)	masterp=T;;
	*)	echo 'usage:'
		echo '	ypinit -m'
		echo '	ypinit -s master_server'
		echo ""
		echo "\
where -m is used to build a master yp server data base, and -s is used for"
		echo "\
a slave data base.  master_server must be an existing reachable yp server."
		exit 1;;
	esac;;

2)	case $1 in
	-s)	slavep=T; master=$2;;
	*)	echo 'usage:'
		echo '	ypinit -m'
		echo '	ypinit -s master_server'
		echo ""
		echo "\
where -m is used to build a master yp server data base, and -s is used for"
		echo "\
a slave data base.  master_server must be an existing reachable yp server."
		exit 1;;
	esac;;

*)	echo 'usage:'
	echo '	ypinit -m'
	echo '	ypinit -s master_server' 
	echo ""
	echo "\
where -m is used to build a master yp server data base, and -s is used for"
	echo "\
a slave data base.  master_server must be an existing reachable yp server."
	exit 1;;
esac


host=`hostname`

if [ $? -ne 0 ]
then 
	echo "Can't get local host's name.  Please check your path."
	exit 1
fi

if [ -z "$host" ]
then
	echo "The local host's name hasn't been set.  Please set it."
	exit 1
fi

def_dom=`domainname`

if [ $? -ne 0 ]
then 
	echo "Can't get local host's domain name.  Please check your path."
	exit 1
fi

if [ -z "$def_dom" ]
then
	echo "The local host's domain name hasn't been set.  Please set it."
	exit 1
fi

domainname $def_dom

if [ $? -ne 0 ]
then 
	echo "\
You have to be the superuser to run this.  Please log in as root."
	exit 1
fi

if [ ! -d $yproot_dir -o -f $yproot_dir ]
then
    echo "\
The directory $yproot_dir doesn't exist.  Restore it from the distribution."
	exit 1
fi

if [ $slavep = T ]
then
	if [ $host = $master ]
	then
		echo "\
The host specified should be a running master yp server, not this machine."
		exit 1
	fi
fi

if [ "$setup" != "yes" ]; then
	echo "Installing the yp data base will require that you answer a few questions."
	echo "Questions will all be asked at the beginning of the procedure."
	echo ""
	echo "Do you want this procedure to quit on non-fatal errors? [y/n: n]  \c"
	read doexit
else
	doexit=yes
fi

case $doexit in
y*)	exit_on_error=T;;
Y*)	exit_on_error=T;;
*)	echo "\
OK, please remember to go back and redo manually whatever fails.  If you"
	echo "\
don't, some part of the system (perhaps the yp itself) won't work.";;
esac

echo ""

for dir in $yproot_dir/$def_dom
do

	if [ -d $dir ]; then
		if [ "$setup" != "yes" ]; then
			echo "Can we destroy the existing $dir and its contents? [y/n: n]  \c"
			read kill_old_dir
		else 
			kill_old_dir=yes
		fi

		case $kill_old_dir in
		y*)	rm -r -f $dir

			if [ $?  -ne 0 ]
			then
			echo "Can't clean up old directory $dir.  Fatal error."
				exit 1
			fi;;

		Y*)	rm -r -f $dir

			if [ $?  -ne 0 ]
			then
			echo "Can't clean up old directory $dir.  Fatal error."
				exit 1
			fi;;

		*)    echo "OK, please clean it up by hand and start again.  Bye"
			exit 0;;
		esac
	fi

	mkdir $dir

	if [ $?  -ne 0 ]
	then
		echo "Can't make new directory $dir.  Fatal error."
		exit 1
	fi

done

if [ $slavep = T ]
then

	echo "\
There will be no further questions. The remainder of the procedure should take"
	echo "a few minutes, to copy the data bases from $master."

	for dom in  $def_dom
	do
		for map in $maps
		do
			echo "Transferring $map..."
			$XFR -h $master -c -d $dom $map

			if [ $?  -ne 0 ]
			then
				errors_in_setup=T

				if [ $exit_on_error = T ]
				then
					exit 1
				fi
			fi
		done
	done

	echo ""
	echo "${host}'s yellowpages data base has been set up\c"

	if [ $errors_in_setup = T ]
	then
		echo " with errors.  Please remember"
		echo "to figure out what went wrong, and fix it."
	else
		echo " without any errors."
	fi

	exit 0
else

	rm -f $yproot_dir/*.time

	while [ $got_host_list = F ]; do
		echo $host >$hf
		if [ "$setup" != "yes" ]; then
			echo ""
			echo "\
	At this point, we have to construct a list of the hosts which will run yp"
			echo "\
	servers.  $host is in the list of yp server hosts.  Please continue to add"
			echo "\
	the names for the other hosts, one per line.  When you are done with the"
			echo "list, type a <ctl D>."
			echo "	next host to add:  $host"
			echo "	next host to add:  \c"

			while read h
			do
				echo "	next host to add:  \c"
				echo $h >>$hf
			done

			echo ""
			echo "The current list of yp servers looks like this:"
			echo ""

			cat $hf
			echo ""
			echo "Is this correct?  [y/n: y]  \c"
			read hlist_ok

			case $hlist_ok in
			n*)	got_host_list=F
				echo "Let's try the whole thing again...";;
			N*)	got_host_list=F
				echo "Let's try the whole thing again...";;
			*)	got_host_list=T;;
			esac
		else 
			got_host_list=T
		fi
	done

	echo "\
There will be no further questions. The remainder of the procedure should take"
	echo "5 to 10 minutes."

	echo "Building $yproot_dir/$def_dom/ypservers..."
	$yproot_dir/makedbm $hf $yproot_dir/$def_dom/`ypmapname ypservers`

	if [ $?  -ne 0 ]
	then
		echo "\
Couldn't build yp data base $yproot_dir/ypservers."
		errors_in_setup=T

		if [ $exit_on_error = T ]
		then
			exit 1
		fi
	fi

	rm $hf

	in_pwd=`pwd`
	cd $yproot_dir
	echo "Running \c"
	echo "$yproot_dir\c"
	echo "/Makefile..."
	make NOPUSH=1 

	if [ $?  -ne 0 ]
	then
		echo "\
Error running Makefile."
		errors_in_setup=T
		
		if [ $exit_on_error = T ]
		then
			exit 1
		fi
	fi

	cd $in_pwd
	echo ""
	echo "$host has been set up as a yp master server\c"

	if [ $errors_in_setup = T ]
	then
		echo " with errors.  Please remember"
		echo "to figure out what went wrong, and fix it."
	else
		echo " without any errors."
	fi

	echo ""
	echo "\
If there are running slave yp servers, run yppush now for any data bases"
	echo "\
which have been changed.  If there are no running slaves, run ypinit on"
	echo "\
those hosts which are to be slave servers."

fi
