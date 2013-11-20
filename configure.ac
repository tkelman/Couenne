# Copyright (C) 2011 Stefan Vigerske and others
# All Rights Reserved.
# This file is distributed under the Eclipse Public License.

## $Id: configure.ac 937 2010-10-02 19:05:12Z stefan $

#############################################################################
#                       Names and other basic things                        #
#############################################################################

AC_PREREQ(2.59)

AC_INIT([ThirdPartyScip],[1.0.2],[http://projects.coin-or.org/BuildTools/newticket])

AC_COPYRIGHT([
Copyright 2011 Stefan Vigerske and others.
All Rights Reserved.
This file is part of the open source package COIN-OR which is distributed
under the Eclipse Public License.])

# List one file in the package so that the configure script can test
# whether the package is actually there
AC_CONFIG_SRCDIR(scip/src/scip/scip.h)

# Where should everything be installed by default?  Here, we want it
# to be installed directly in 'bin', 'lib', 'include' subdirectories
# of the directory where configure is run.  The default would be
# /usr/local.
AC_PREFIX_DEFAULT([`pwd`])

AC_COIN_PROJECTDIR_INIT(SCIP,1:2:0)

#############################################################################
#                         Standard build tool stuff                         #
#############################################################################

# Get the system type
AC_CANONICAL_BUILD

# Check if user wants to produce debugging code
AC_COIN_DEBUG_COMPILE(SCIP)

# Get the name of the C compiler and appropriate compiler options
AC_COIN_PROG_CC

# Get the name of the C++ compiler and appropriate compiler options
AC_COIN_PROG_CXX

# Initialize automake
AM_INIT_AUTOMAKE([subdir-objects])
AM_PROG_CC_C_O
AC_COIN_INIT_AUTO_TOOLS

#############################################################################
#                         Check SCIP version number                         #
#############################################################################

AC_MSG_CHECKING([SCIP version])
scipver=`awk '/define SCIP_VERSION/ {print $3}' ${srcdir}/scip/src/scip/def.h`
scipsubver=`awk '/define SCIP_SUBVERSION/ {print $3}' ${srcdir}/scip/src/scip/def.h`
AC_MSG_RESULT([${scipver}.${scipsubver}])
if test $scipver -lt 200 -o $scipver -ge 210 ; then
  AC_MSG_ERROR(["Incorrect SCIP version. This build system is made for SCIP 2.0.x"])
fi

#############################################################################
#                       Check for external libraries                        #
#############################################################################

# check for some LP solvers
AC_COIN_CHECK_PACKAGE(SoPlex, coinsoplex)
AC_COIN_CHECK_PACKAGE(Clp, clp)
AC_COIN_CHECK_USER_LIBRARY([Cplex], [CPX],[cplex.h],   [CPXgetstat])
AC_COIN_CHECK_USER_LIBRARY([Mosek], [MSK],[mosek.h],   [MSK_makeenv])
AC_COIN_CHECK_USER_LIBRARY([Xpress],[XPR],[xprs.h],    [XPRSinit])
AC_COIN_CHECK_USER_LIBRARY([Gurobi],[GRB],[gurobi_c.h],[GRBloadenv])
AC_COIN_CHECK_USER_LIBRARY([QSopt], [QSO],[qsopt.h],   [QScreate_prob])

AC_ARG_WITH([scip-lpsolver],
  AS_HELP_STRING([--with-scip-lpsolver],
    [specify LP solver to use in SCIP (soplex, clp, cplex, mosek, xpress, gurobi, qsopt, none)]),
  [if test "$withval" = no ; then
     scip_lpsolver=none
   else
     scip_lpsolver="$withval"
   fi],
  [if test "$coin_has_soplex" = yes ; then
     scip_lpsolver=soplex
   elif test "$coin_has_clp" = yes ; then
     scip_lpsolver=clp
   elif test "$coin_has_cpx" = true ; then
     scip_lpsolver=cplex
   elif test "$coin_has_msk" = true ; then
     scip_lpsolver=mosek
   elif test "$coin_has_xpr" = true ; then
     scip_lpsolver=xpress
   elif test "$coin_has_grb" = true ; then
     scip_lpsolver=gurobi
   elif test "$coin_has_qso" = true ; then
     scip_lpsolver=qsopt
   else
     scip_lpsolver=none
   fi
  ])
  
case "$scip_lpsolver" in
  soplex)
    if test "$coin_has_soplex" = no ; then
      AC_MSG_ERROR(["LP solver SoPlex not available.])
    fi
    SCIPLIB_CFLAGS="$SCIPLIB_CFLAGS $SOPLEX_CFLAGS"
    SCIPLIB_LIBS="$SCIPLIB_LIBS $SOPLEX_LIBS"
    SCIPLIB_LIBS_INSTALLED="$SCIPLIB_LIBS $SOPLEX_LIBS_INSTALLED"
    SCIPLIB_PCLIBS="$SCIPLIB_PCLIBS $SOPLEX_PCLIBS"
    SCIPLIB_PCREQUIRES="$SCIPLIB_PCREQUIRES $SOPLEX_PCREQUIRES"
  ;;
  clp)
    if test "$coin_has_clp" = no ; then
      AC_MSG_ERROR(["LP solver CLP not available.])
    fi
    SCIPLIB_CFLAGS="$SCIPLIB_CFLAGS $CLP_CFLAGS"
    SCIPLIB_LIBS="$SCIPLIB_LIBS $CLP_LIBS"
    SCIPLIB_LIBS_INSTALLED="$SCIPLIB_LIBS $CLP_LIBS_INSTALLED"
    SCIPLIB_PCLIBS="$SCIPLIB_PCLIBS $CLP_PCLIBS"
    SCIPLIB_PCREQUIRES="$SCIPLIB_PCREQUIRES $CLP_PCREQUIRES"
  ;;
  cplex)
    if test "$coin_has_cpx" = false ; then
      AC_MSG_ERROR(["LP solver CPLEX not available.])
    fi
    SCIPLIB_CFLAGS="$SCIPLIB_CFLAGS -I$CPXINCDIR"
    SCIPLIB_LIBS="$SCIPLIB_LIBS $CPXLIB"
    SCIPLIB_LIBS_INSTALLED="$SCIPLIB_LIBS $CPXLIB"
    SCIPLIB_PCLIBS="$SCIPLIB_PCLIBS $CPXLIB"
  ;;
  mosek)
    if test "$coin_has_msk" = false ; then
      AC_MSG_ERROR(["LP solver Mosek not available.])
    fi
    SCIPLIB_CFLAGS="$SCIPLIB_CFLAGS -I$MSKINCDIR"
    SCIPLIB_LIBS="$SCIPLIB_LIBS $MSKLIB"
    SCIPLIB_LIBS_INSTALLED="$SCIPLIB_LIBS $MSKLIB"
    SCIPLIB_PCLIBS="$SCIPLIB_PCLIBS $MSKLIB"
  ;;
  xpress)
    if test "$coin_has_xpr" = false ; then
      AC_MSG_ERROR(["LP solver Xpress not available.])
    fi
    SCIPLIB_CFLAGS="$SCIPLIB_CFLAGS -I$XPRINCDIR"
    SCIPLIB_LIBS="$SCIPLIB_LIBS $XPRLIB"
    SCIPLIB_LIBS_INSTALLED="$SCIPLIB_LIBS $XPRLIB"
    SCIPLIB_PCLIBS="$SCIPLIB_PCLIBS $XPRLIB"
  ;;
  gurobi)
    if test "$coin_has_grb" = false ; then
      AC_MSG_ERROR(["LP solver Gurobi not available.])
    fi
    SCIPLIB_CFLAGS="$SCIPLIB_CFLAGS -I$GRBINCDIR"
    SCIPLIB_LIBS="$SCIPLIB_LIBS $GRBLIB"
    SCIPLIB_LIBS_INSTALLED="$SCIPLIB_LIBS $GRBLIB"
    SCIPLIB_PCLIBS="$SCIPLIB_PCLIBS $GRBLIB"
  ;;
  qsopt)
    if test "$coin_has_qso" = false ; then
      AC_MSG_ERROR(["LP solver QSopt not available.])
    fi
    SCIPLIB_CFLAGS="$SCIPLIB_CFLAGS -I$QSOINCDIR"
    SCIPLIB_LIBS="$SCIPLIB_LIBS $QSOLIB"
    SCIPLIB_LIBS_INSTALLED="$SCIPLIB_LIBS $QSOLIB"
    SCIPLIB_PCLIBS="$SCIPLIB_PCLIBS $QSOLIB"
  ;;
  none)
  ;;
  *)
    AC_MSG_ERROR([LP solver $scip_lpsolver unknown.])
  ;;
esac
AC_MSG_NOTICE([build with LP solver $scip_lpsolver])
AM_CONDITIONAL(SCIPLPI_SPX, test $scip_lpsolver = soplex)
AM_CONDITIONAL(SCIPLPI_CLP, test $scip_lpsolver = clp)
AM_CONDITIONAL(SCIPLPI_CPX, test $scip_lpsolver = cplex)
AM_CONDITIONAL(SCIPLPI_MSK, test $scip_lpsolver = mosek)
AM_CONDITIONAL(SCIPLPI_XPR, test $scip_lpsolver = xpress)
AM_CONDITIONAL(SCIPLPI_GRB, test $scip_lpsolver = gurobi)
AM_CONDITIONAL(SCIPLPI_QSO, test $scip_lpsolver = qsopt)
AM_CONDITIONAL(SCIPLPI_NONE, test $scip_lpsolver = none)

# check for NLP solvers
if test $scipver -ge 202 ; then
  AC_COIN_CHECK_PACKAGE(Ipopt, ipopt, SCIPLib)
else
  AM_CONDITIONAL(COIN_HAS_IPOPT, test 0 = 1)
fi

# check for CppAD
AC_COIN_CHECK_PACKAGE(CppAD, cppad, SCIPLib)

#############################################################################
#                       Check for system header and libraries               #
#############################################################################

AC_COIN_CHECK_GNU_ZLIB(SCIPlib)
if test $coin_has_zlib = "yes"; then
  EXTRAFLAGS="$EXTRAFLAGS -DWITH_ZLIB"  
fi

AC_COIN_CHECK_GMP(SCIPLib)
if test $coin_has_gmp = yes; then
  EXTRAFLAGS="$EXTRAFLAGS -DWITH_GMP"  
fi

AC_COIN_CHECK_GNU_READLINE(SCIPLib)
if test $coin_has_readline = yes; then
  EXTRAFLAGS="$EXTRAFLAGS -DWITH_READLINE"  
fi

AC_CHECK_MEMBER(struct sigaction.sa_handler,,
  [EXTRAFLAGS="$EXTRAFLAGS -DNO_SIGACTION"],
  [#include <signal.h>])
    
AC_CHECK_FUNC(rand_r,,
  [EXTRAFLAGS="$EXTRAFLAGS -DNO_RAND_R"])
    
AC_CHECK_FUNC(strtok_r,,
  [EXTRAFLAGS="$EXTRAFLAGS -DNO_STRTOK_R"])

AC_CHECK_FUNC(strerror_r,,
  [EXTRAFLAGS="$EXTRAFLAGS -DNO_STRERROR_R"])
    
AC_CHECK_HEADER([fenv.h],[EXTRAFLAGS="$EXTRAFLAGS -DROUNDING_FE"],
  [AC_CHECK_HEADER([float.h],[EXTRAFLAGS="$EXTRAFLAGS -DROUNDING_MS"])])

case "$CC" in
  *gcc* )
    EXTRAFLAGS="$EXTRAFLAGS -Wno-overlength-strings"
    ;;
esac
 
CFLAGS="$CFLAGS $EXTRAFLAGS"
CXXFLAGS="$CXXFLAGS $EXTRAFLAGS"

#############################################################################
#                                Finish up                                  #
#############################################################################

AC_CONFIG_FILES([Makefile coinscip.pc coinscip-uninstalled.pc])

# dummy header file to avoid many extra compiler flags
AC_CONFIG_HEADER([config_scip.h])
 
AC_COIN_FINALIZE
