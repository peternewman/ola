## ac_nicolaudie.m4 -- macros for detecting the Nicolaudie LsHardDevKit
## library.
## Copyright (C) 2018 Peter Newman
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

# NICOLAUDIE_DEVICE()
# Check that the Nicolaudie LsHardDevKit library is installed and usable. This
# will define the following:
#
# In config.h:
#   HAVE_LSHARDDEVKIT_H
# In Makefile.am
#   libLsHardDevKit_LIBS
#   HAVE_NICOLAUDIE_DEV_KIT
#
# -----------------------------------------------------------------------------
AC_DEFUN([NICOLAUDIE_DEVICE],
[
  AC_REQUIRE_CPP()
  AC_CHECK_HEADERS([LsHardDevKit.h])

  libLsHardDevKit_LIBS="-lLsHardDevKit"
  AC_SUBST(libLsHardDevKit_LIBS)

  old_libs=$LIBS
  LIBS="${LIBS} ${libLsHardDevKit_LIBS}"
  AC_LINK_IFELSE(
    [AC_LANG_PROGRAM([#include <LsHardDevKit.h>],
                     [[DHDK_init(DHDK_DASUSB|DHDK_DASNET, "ac_nicolaudie.m4");
                       DHDK_deinit();]])],
    [have_nicolaudie=yes],
    [have_nicolaudie=no])
  LIBS=$old_libs
  AS_IF([test "x$have_nicolaudie" == xno],
        [AC_MSG_WARN([Nicolaudie LsHardDevKit library is not usable.])])

  AM_CONDITIONAL(HAVE_NICOLAUDIE_DEV_KIT, test "${have_nicolaudie}" = "yes")
])
