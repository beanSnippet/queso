# -*- Autoconf -*-
# Process this file with autoconf to produce a configure script. 

AC_PREREQ(2.65)
AC_INIT([queso], [0.50.1], [queso-users@googlegroups.com])
AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_HEADERS(config_queso.h.tmp)
dnl Prefixes everything in the temporary file with QUESO_
AX_PREFIX_CONFIG_H(config_queso.h,QUESO,config_queso.h.tmp)

AC_CONFIG_AUX_DIR([build-aux])
AC_CANONICAL_TARGET()
AM_INIT_AUTOMAKE

# Release versioning

AX_SPLIT_VERSION
GENERIC_MAJOR_VERSION=$AX_MAJOR_VERSION
GENERIC_MINOR_VERSION=$AX_MINOR_VERSION
GENERIC_MICRO_VERSION=$AX_POINT_VERSION

# Generating a libtool library

GENERIC_RELEASE=$GENERIC_MAJOR_VERSION.$GENERIC_MINOR_VERSION

AC_SUBST(GENERIC_RELEASE)
AC_SUBST(GENERIC_MAJOR_VERSION)
AC_SUBST(GENERIC_MINOR_VERSION)
AC_SUBST(GENERIC_MICRO_VERSION)

LT_INIT

# Set default CFLAGS & CXXFLAGS IF the user hasn't specified anything

AC_MSG_CHECKING(if CFLAGS is set)
if test "x$CFLAGS" = "x"; then
   CFLAGS="-g -O2 -Wall"
   AC_MSG_RESULT(no... setting to -g -O2 -Wall)
else
   AC_MSG_RESULT(yes)
fi

AC_MSG_CHECKING(if CXXFLAGS is set)
if test "x$CXXFLAGS" = "x"; then
   CXXFLAGS="-g -O2 -Wall"
   AC_MSG_RESULT(no... setting to -g -O2 -Wall)
else
   AC_MSG_RESULT(yes)
fi

# Required for some macros
# AX_AM_MACROS

#-----------------------
# Check for debug mode?
#-----------------------

AC_MSG_CHECKING(whether to enable debugging)
debug_default="no"
AC_ARG_ENABLE(debug, [  --enable-debug=[no/yes] turn on debugging
                     [default=$debug_default]],, enable_debug=$debug_default)

if test "x$enable_debug" = "xyes"; then
   CFLAGS="$CFLAGS -DDEBUG"
   CXXFLAGS="$CXXFLAGS -DDEBUG"
   AC_MSG_RESULT(yes)
else
   AC_MSG_RESULT(no)
fi

#-------------------
# Compilers and MPI
#-------------------

AC_PROG_CC
AC_PROG_CXX
AC_PROG_FC
AC_LANG([C])
ACX_MPI([CC="$MPICC"], [AC_MSG_ERROR([Could not find MPI.])])
AC_LANG([C++])
ACX_MPI([CXX="$MPICXX"], [AC_MSG_ERROR([Could not find MPI.])])

#-------------------------
# External Library Checks
#-------------------------

# Check for Trilinos (optional)
#Kemelli: The lowest version used to be 8.0.7, now is 11.0.2, to accomodate the changes in -lteuchos*.

HAVE_TRILINOS=0

AX_TRILINOS_BASE(11.0.2,[HAVE_TRILINOS=1],[HAVE_TRILINOS=0])

if test "$HAVE_TRILINOS" = "1" ;then
   #AX_TRILINOS_EPETRA([HAVE_TRILINOS=1],[HAVE_TRILINOS=0])
   AX_TRILINOS_EPETRA([HAVE_TRILINOS=1],AC_MSG_ERROR([Could not find Epetra - which is required if Trilinos is enabled.]))
fi

AC_SUBST(HAVE_TRILINOS)
AM_CONDITIONAL(TRILINOS_ENABLED,test x$HAVE_TRILINOS = x1)

AC_CACHE_SAVE

# Check for libmesh (optional)
AX_PATH_LIBMESH_NEW(0.8.0,no)
AC_CACHE_SAVE

# Check for GSL (required)

AC_LANG([C])
AX_PATH_GSL(1.10,AM_CONDITIONAL([UQBT_GSL], [test 'TRUE']),AC_MSG_ERROR([Could not find required GSL version.]))
AC_CACHE_SAVE

# Check for boost (required)

AC_LANG([C++])
BOOST_REQUIRE([1.35])
BOOST_PROGRAM_OPTIONS
AC_CACHE_SAVE

# Check for GLPK (optional)

AX_PATH_GLPK([4.35],[no])

# Check for HDF5 (optional)

AX_PATH_HDF5_NEW([1.8.0],[no])

# Check for ANN (external library)
# AX_PATH_ANN 
#### TODO: Make sure that the ANN uses L-infinity (Max) norm

# Check for ANN feature
AX_ENABLE_ANN

# Check for libGRVY (optional as of QUESO version 0.46.0)

AX_PATH_GRVY_NEW([0.29],[no])

# Check for slepc
#AX_PATH_SLEPC_NEW([3.3],[no])

AC_CACHE_SAVE

#----------------
# Doxygen support
#----------------

DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(ON)
DX_PS_FEATURE(OFF)

DX_INIT_DOXYGEN([queso],[doxygen/queso.dox],[docs])

#--------------------------
# Checks for code coverage
#-------------------------

AX_CODE_COVERAGE

# Query configuration environment

AX_SUMMARIZE_ENV

# Generate makefiles

AC_CONFIG_FILES([
  Makefile
  inc/queso/Makefile
  src/Makefile
  src/contrib/ANN/Makefile
  src/contrib/ANN/lib/Makefile
  src/contrib/ANN/test/Makefile
  src/core/inc/queso.h
  examples/Makefile        
  examples/infinite_dim/inverse_options
  examples/infinite_dim/parallel_inverse_options
  test/Makefile
  test/t01_valid_cycle/Makefile
  test/t02_sip_sfp/Makefile
  test/t03_sequence/Makefile
  test/t04_bimodal/Makefile
  test/gsl_tests/Makefile
  test/gsl_tests/input
  test/test_Environment/copy_env
  test/test_infinite/inf_options
  test/test_Regression/GaussianMean1DRegression_options
  test/test_Regression/gpmsa_cobra_input.txt
  test/test_Regression/test_cobra_samples_diff.sh
  test/test_Regression/test_gpmsa_cobra_samples.m
  test/test_Regression/ctf_dat.txt
  test/test_Regression/dakota_pstudy.dat
  doxygen/Makefile
  doxygen/queso.dox
  doxygen/txt_common/about_vpath.page
],
[
  chmod +x test/test_Regression/test_cobra_samples_diff.sh
])

AC_OUTPUT()

#---------------
# Final summary
#---------------

AX_SUMMARIZE_CONFIG
