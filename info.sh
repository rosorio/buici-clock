#!/bin/sh
echo "Informational dump for Buici"
echo '$Id: info.sh,v 1.2 1997/10/23 02:30:35 elf Exp $'
echo "Please send this data to elf@netcom.com if you are having problems."
echo "Report from " ` whoami` on `hostname` of `domainname` on `date`
uname -a
gcc --version
nm --version
if [ -f o/buici ] ; then strings o/buici | grep libc ; else echo "no binary in o/buici" ; fi
ls -l /lib/libc.so*
xdpyinfo -display ${DISPLAY} -queryExtensions
xprop -display ${DISPLAY} -root
xrdb -query
