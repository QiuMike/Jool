#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([Jool], [3.5.7], [jool@nic.mx])
AC_CONFIG_SRCDIR([src/jool.c])
AM_INIT_AUTOMAKE([subdir-objects])

# Checks for programs.
AC_PROG_CC

# It's been added because automake requires it, since automake 1.14 it is no longer required.
AM_PROG_CC_C_O

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_CHECK_FUNCS([inet_ntoa memset strcasecmp strtol pow])
AC_SEARCH_LIBS([pow], [m])
AC_SEARCH_LIBS([pthread_create], [pthread])
AC_SEARCH_LIBS([argp_parse], [argp])

# Checks for dependencies.
PKG_CHECK_MODULES(LIBNLGENL3, libnl-genl-3.0 >= 3.1)

# Apparently, there was some long debate in the interwebs some time ago about
# whether maintainer mode should be enabled or not. I don't know the final
# verdict because all I see is opinions. So I'm doing what I think is right
# in spite of whatever the "official" current practice is.
#
# Here's the thing: I don't actually give a flying f*ck about maintainer mode.
# When it comes to autotools, I always rebuild everything from scratch because
# I've found their timestamps and cache heuristics to be appallingly unreliable
# (and their error messages abysmal).
#
# Whe logic that drove me to include this clause is as follows:
#
# Until Debian packaging, we used to not need any maintainer mode clauses.
# And everything was whateverly fine. According to the documentation, not
# including the clause enables maintainer mode, and prevents it from being
# turned off (ie. the configure script is ignorant of the
# `--disable-maintainer-mode` flag).
#
# From Debian packaging onwards, `dh_auto_configure` (used by
# `dpkg-buildpackage`) wants to issue the `--disable-maintainer-mode` flag to
# `./configure`.
# So `./configure` spews a "unrecognized options: --disable-maintainer-mode"
# warning.
#
# `AM_MAINTAINER_MODE([enable])` leaves maintainer mode enabled by default,
# but also includes the `--disable-maintainer-mode` flag.
AM_MAINTAINER_MODE([enable]) 

# Spit out the makefiles.
AC_OUTPUT(Makefile stateless/Makefile stateful/Makefile joold/Makefile)
