dnl configure.ac
dnl Written by Gary V. Vaughan, 2012

dnl Copyright (c) 2012-2014 Gary V. Vaughan

dnl This file is part of luaposix.
dnl See README for license.

AC_INIT([luaposix], [32], [http://github.com/luaposix/luaposix/issues])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AS_BOX([Configuring AC_PACKAGE_TARNAME AC_PACKAGE_VERSION])
echo

AM_INIT_AUTOMAKE([-Wall subdir-objects])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl Check for programs
AC_PROG_CC
gl_EARLY
AC_PROG_MKDIR_P
AC_PROG_EGREP
AC_PROG_SED

AC_USE_SYSTEM_EXTENSIONS
AC_ARG_ENABLE([gcc-warnings],
  [AS_HELP_STRING([--enable-gcc-warnings],
                  [turn on lots of GCC warnings (for developers)])],
  [case $enableval in
     yes|no) ;;
     *)      AC_MSG_ERROR([bad value $enableval for gcc-warnings option]) ;;
   esac
   gl_gcc_warnings=$enableval],
  [gl_gcc_warnings=no]
)

if test "$gl_gcc_warnings" = yes; then
  gl_WARN_ADD([-Werror], [WERROR_CFLAGS])

  # Set up the list of undesired warnings.
  nw=
  nw="$nw -Wsystem-headers"   # Don't let system headers trigger warnings
  nw="$nw -Wundef"            # All compiler preprocessors support #if UNDEF
  nw="$nw -Wtraditional"      # All compilers nowadays support ANSI C
  nw="$nw -Wstrict-overflow"  # Use a lower level (see below).
  nw="$nw -Wconversion"       # These warnings usually don't point to mistakes.
  nw="$nw -Wsign-conversion"  # Likewise.
  nw="$nw -Waggregate-return" # We do this on purpose.
  nw="$nw -Wtraditional-conversion" # Don't care.
  nw="$nw -Wpadded"           # Don't care.
  nw="$nw -Wc++-compat"       # Don't care.
  nw="$nw -Woverlength-strings" # Don't care.
  nw="$nw -Wmissing-format-attribute" # Don't care.
  nw="$nw -Wunreachable-code" # Seems buggy.
  nw="$nw -Wunused-macros"    # those macros might come in handy later

  gl_MANYWARN_ALL_GCC([warnings])

  # Enable all GCC warnings not in this list.
  gl_MANYWARN_COMPLEMENT([warnings], [$warnings], [$nw])
  for w in $warnings; do
    gl_WARN_ADD([$w])
  done

  # Add an extra warning
  gl_WARN_ADD([-Wstrict-overflow=1])
  # Add some more safety measures
  gl_WARN_ADD([-D_FORTIFY_SOURCE=2])
  gl_WARN_ADD([-fmudflap])
fi

dnl required by automake 1.12.x, not available in 1.10.x
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

LT_PREREQ([2.2.6])
LT_INIT([disable-static])

module=yes eval shrext=$shrext_cmds

AC_SUBST([objdir])
AC_SUBST([shrext])
AC_SUBST(LIBTOOL_DEPS)

gl_INIT

dnl Check for header files
AC_HEADER_STDC
AC_CHECK_HEADERS([crypt.h strings.h sys/statvfs.h net/if.h linux/netlink.h])

dnl Search libs
AC_SEARCH_LIBS([crypt], [crypt],
  [AC_DEFINE([HAVE_CRYPT], [], [Define this if your system has a crypt() function])])

dnl Check for functions
AC_CHECK_FUNCS([strlcpy statvfs posix_fadvise])

dnl Compiler flags for POSIX
dnl Get this from autotools/gnulib
if test Darwin != `uname`; then
  # FIXME: Make -lrt conditional on _XOPEN_REALTIME
  POSIX_EXTRA_CFLAGS=-D_XOPEN_SOURCE=700
  POSIX_EXTRA_LDFLAGS=-lrt
fi
AC_SUBST(POSIX_EXTRA_CFLAGS)
AC_SUBST(POSIX_EXTRA_LDFLAGS)

dnl Curses
AX_WITH_CURSES
if test "$ax_cv_curses" = "yes"; then
  WANTEDLIBS=ext/curses/curses_c.la
  WANTEDLUA=lib/curses.lua
fi
AC_ARG_VAR(CURSES_LIB, [linker flags for curses library])
AC_SUBST(WANTEDLIBS)
AC_SUBST(WANTEDLUA)

save_LIBS=$LIBS
LIBS="$CURSES_LIB $LIBS"
AC_CHECK_FUNCS([resizeterm])
LIBS=$save_LIBS

dnl Lua 5.1 or 5.2
AX_PROG_LUA(5.1, 5.3)
AX_LUA_HEADERS
AC_PATH_PROG([SPECL], [specl], [:])

dnl Perl
AC_PATH_PROG([PERL], [perl])
if test -z "$PERL"; then
  AC_MSG_FAILURE([cannot find perl])
fi

dnl ldoc
AC_PATH_PROG([LDOC], [ldoc], [false])
AM_CONDITIONAL([HAVE_LDOC], [test false != "$LDOC"])

dnl Generate output files
SS_CONFIG_TRAVIS([ldoc specl])
AC_CONFIG_HEADER(config.h)
AC_CONFIG_FILES([Makefile build-aux/config.ld])
AC_OUTPUT
