# Copyright (C) 2010,2011 Stefan Vigerske
# All Rights Reserved.
# This file is distributed under the Common Public License.

## $Id: configure.ac 928 2010-09-14 12:18:54Z stefan $

#############################################################################
#                       Names and other basic things                        #
#############################################################################

AC_PREREQ(2.59)

AC_INIT([ThirdPartySoplex],[1.0.0],[http://projects.coin-or.org/BuildTools/newticket])

AC_COPYRIGHT([
Copyright 2010,2011 Stefan Vigerske and others.
All Rights Reserved.
This file is part of the open source package COIN-OR which is distributed
under the Eclipse Public License.])

# List one file in the package so that the configure script can test
# whether the package is actually there
AC_CONFIG_SRCDIR(soplex/src/soplex.h)

# Where should everything be installed by default?  Here, we want it
# to be installed directly in 'bin', 'lib', 'include' subdirectories
# of the directory where configure is run.  The default would be
# /usr/local.
AC_PREFIX_DEFAULT([`pwd`])

AC_COIN_PROJECTDIR_INIT(SoPlex,1:0:0)

#############################################################################
#                         Standard build tool stuff                         #
#############################################################################

# Get the system type
AC_CANONICAL_BUILD

# Check if user wants to produce debugging code
AC_COIN_DEBUG_COMPILE(SoPlex)

# Get the name of the C compiler and appropriate compiler options
AC_COIN_PROG_CC

# Get the name of the C++ compiler and appropriate compiler options
AC_COIN_PROG_CXX

# Check if pkg-config is available (to setup addlibs file)
AC_COIN_HAS_PKGCONFIG

# Initialize automake
AC_COIN_INIT_AUTO_TOOLS

###########################################################################
#                         ZLIB and compiler flags                         #
###########################################################################

AC_COIN_CHECK_GNU_ZLIB(SOPLEX)
if test $coin_has_zlib = "yes"; then
  CXXFLAGS="$CXXFLAGS -DWITH_ZLIB"
fi

CXXFLAGS="$CXXFLAGS -DNO_ADDITIONAL_CHECKS"

###########################################################################
#                               finish up                                 #
###########################################################################

AC_CONFIG_FILES([Makefile coinsoplex.pc coinsoplex-uninstalled.pc])

# dummy header file to avoid many extra compiler flags
AC_CONFIG_HEADER([config_soplex.h])

AC_SUBST(SOPLEX_PCLIBS)
 
AC_COIN_FINALIZE
