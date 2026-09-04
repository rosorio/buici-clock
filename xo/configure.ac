dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.71])dnl              dnl Minimum Autoconf version required.
AC_INIT
AC_CONFIG_SRCDIR([ldisplay.cxx])  dnl Name of unique file that must exist

AC_CONFIG_HEADERS([config.h])

AC_LANG([C++])                    dnl Default to c++ cause that what we done.

dnl Checks for programs.
AC_PROG_YACC
AC_PROG_LEX([noyywrap])
AC_PROG_CXX
AC_PROG_LN_S

if test "${GXX}" = "yes" ; then 
 CFLAGS_W=" -Wall -Wno-parentheses -Wno-unused" 
 CFLAGS="${CFLAGS}"
fi

dnl Checks for header files.
AC_PATH_XTRA
AC_CHECK_HEADERS(memory.h fcntl.h limits.h unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
dnl -- Removed because it tends to interfere with headers
dnl AC_C_CONST
dnl -- Removed because it tends to interfere with headers
dnl AC_C_INLINE
AC_TYPE_SIZE_T

dnl Checks for library functions.
dnl AC_FUNC_MMAP
dnl AC_FUNC_VPRINTF				dnl -- We use vsprintf


AC_SUBST(CFLAGS_W)
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

dnl -- Prevent accidental edits.  
chmod -w Makefile
chmod -w config.h
