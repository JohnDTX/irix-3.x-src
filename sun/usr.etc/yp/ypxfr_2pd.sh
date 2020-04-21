#! /bin/sh
#
# @(#)ypxfr_2perday.sh 1.1 86/02/05 Copyr 1985 Sun Microsystems, Inc.  
# @(#)ypxfr_2perday.sh	2.1 86/04/16 NFSSRC
#
# ypxfr_2perday.sh - Do twice-daily yp map check/updates
#

# set -xv
/usr/etc/yp/ypxfr hosts.byname
/usr/etc/yp/ypxfr hosts.byaddr
/usr/etc/yp/ypxfr ethers.byaddr
/usr/etc/yp/ypxfr ethers.byname
/usr/etc/yp/ypxfr netgroup
/usr/etc/yp/ypxfr netgroup.byuser
/usr/etc/yp/ypxfr netgroup.byhost
/usr/etc/yp/ypxfr mail.aliases
