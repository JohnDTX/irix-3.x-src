#! /bin/sh
#
# @(#)ypxfr_1perday.sh 1.1 86/02/05 Copyr 1985 Sun Microsystems, Inc.  
# @(#)ypxfr_1perday.sh	2.1 86/04/16 NFSSRC
#
# ypxfr_1perday.sh - Do daily yp map check/updates
#

# set -xv
/usr/etc/yp/ypxfr group.byname
/usr/etc/yp/ypxfr group.bygid 
/usr/etc/yp/ypxfr protocols.byname
/usr/etc/yp/ypxfr protocols.bynumber
/usr/etc/yp/ypxfr networks.byname
/usr/etc/yp/ypxfr networks.byaddr
/usr/etc/yp/ypxfr services.byname
/usr/etc/yp/ypxfr ypservers
