#! /bin/sh
#
# @(#)ypxfr_1perhour.sh 1.1 86/02/05 Copyr 1985 Sun Microsystems, Inc.  
# @(#)ypxfr_1perhour.sh	2.1 86/04/16 NFSSRC
#
# ypxfr_1perhour.sh - Do hourly yp map check/updates
#

# set -xv
/usr/etc/yp/ypxfr passwd.byname
/usr/etc/yp/ypxfr passwd.byuid 
