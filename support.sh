#!/bin/sh
echo "Build dump for Buici"
echo '$Id: support.sh,v 1.3 1998/10/26 19:03:44 elf Exp $'
echo "Please send this data to elf@debian.org if you are having problems."
echo "You can use a command link this to do it:"
echo '  sh support.sh | mail -s "Support Log" elf@debian.org'
echo '============================================'
echo "Report from " ` whoami` on `hostname` of `domainname` on `date`
uname -a
gcc --version
nm --version
./configure
echo '============================================'
cat config.log
echo '============================================'
make
echo '============================================'
cat config.h
echo '============================================'
cat Makefile
echo '============================================'
