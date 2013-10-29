#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([memshare], [0.3.2], [100yep@gmail.com])
AM_INIT_AUTOMAKE([foreign])
AM_PROG_LIBTOOL
AC_PROG_RANLIB
AC_CONFIG_SRCDIR([src/memsend.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset])

AC_ARG_ENABLE([memcmds],[
AS_HELP_STRING([--disable-memcmds], [Don't build the memshare cmds])])
AM_CONDITIONAL([MEMCMD], [test "x$enable_memcmds" != "xno"])

AC_ARG_ENABLE([tlog],
AS_HELP_STRING([--disable-tlog], [Don't build the tlog library]))
AM_CONDITIONAL([TLOG], [test "x$enable_tlog" != "xno"])

AC_OUTPUT(Makefile)
