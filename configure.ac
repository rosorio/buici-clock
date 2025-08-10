dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.71])dnl              dnl Minimum Autoconf version required.
AC_INIT
AC_CONFIG_SRCDIR([buici.xbm])     dnl Name of unique file that must exist

AC_CONFIG_HEADERS([config.h])

AC_LANG([C++])                    dnl Default to c++ cause that what we done.

dnl Checks for programs.
dnl AC_PROG_CC
AC_PROG_CXX
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_INSTALL

if test "${GXX}" = "yes" ; then 
 CFLAGS_W=" -Wall -Wno-parentheses -Wno-unused" 
 CFLAGS="${CFLAGS}"
fi

dnl Find our XO X class library
AC_MSG_CHECKING("for XO class library")
XO_PATH=`echo xo-*`
if test "${XO_PATH}" = "" -o ! -d ${XO_PATH} ; then eval XO_PATH=xo ; fi
if test "${XO_PATH}" = "" -o ! -d ${XO_PATH} ; then eval XO_PATH=xo ; fi
if test "${XO_PATH}" = "" -o ! -d ${XO_PATH} ; then eval XO_PATH=../xo ; fi
if test ! -d ${XO_PATH} ; then AC_MSG_ERROR(Unable to find xo library) ; fi
AC_MSG_RESULT(${XO_PATH})

AC_CHECK_LIB(m, cos)

dnl Checks for header files.
dnl AC_HEADER_DIRENT
AC_PATH_XTRA

AC_CHECK_HEADERS([unistd.h])

dnl Checks for typedefs, structures, and compiler characteristics.
dnl -- Removed because it tends to interfere with headers
dnl AC_C_CONST
dnl -- Removed because it tends to interfere with headers
dnl AC_C_INLINE
dnl AC_TYPE_OFF_T
AC_CHECK_HEADERS_ONCE([sys/time.h])

dnl AC_CHECK_SIZEOF(long, 4)			dnl -- Make sure to warn user
dnl AC_CHECK_SIZEOF(int, 4)			dnl --   about these defaults.

dnl Checks for library functions.
dnl AC_FUNC_MMAP
dnl AC_FUNC_VPRINTF				dnl -- We use vsprintf
dnl AC_CHECK_FUNCS(mktime stime strftime)	dnl -- Include in the source

PKG_CHECK_MODULES([CAIRO],[cairo])

AC_SUBST(XO_PATH)
AC_SUBST(CFLAGS_W)
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

dnl -- Prevent accidental edits.  
chmod -w Makefile
chmod -w config.h

dnl -- Create empty dependency file so that Make can include it
if test ! -f depend.m ; then touch depend.m ; fi

dnl -- Configure XO, too, if it is local
if test ! "${XO_PATH}" = "../xo" ; then
 if test ! -e ${XO_PATH}/config.cache ; then cp config.cache ${XO_PATH} ; fi
 (cd ${XO_PATH} ; ./configure --build=$build_alias --host=$host_alias )
fi

dnl -- Check for missing RESOURCE_MANAGER string
if test "${DISPLAY}" = "" ; then
 echo "-------------------------------------------------------------------"
 echo "I cannot make some configuration determinations because you are"
 echo "not currently running on an X Server.   Buici will compile as is,"  
 echo "but certain display-specific errors can only be detected when"
 echo "configure is run in a window with access to the display.  I"
 echo "encourage you to run configure again when the X display is active"
 echo "in order to complete the configuration tests."
 echo "-------------------------------------------------------------------"
elif test \
  "`xprop -display ${DISPLAY} -root | grep -l 'RESOURCE_MANAGER(STRING)'`"\
   = "" ; then
 echo "-------------------------------------------------------------------"
 echo "Your X Server is not providing an expected property string used to"
 echo "store resources.  Would you please consider sending the output of"
 echo "the info.sh script to me, the author, elf@netcom.com.  It will help"
 echo "me understand why Buici doesn\'t work on some X Servers."
 echo "-------------------------------------------------------------------"
else
 echo "-------------------------------------------------------------------"
 echo "Your X Server appears to operate as expected."
 echo "-------------------------------------------------------------------"
fi
echo
echo "Run 'make' to build the application."
echo
