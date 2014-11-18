dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(libdigitizersim, 1.0)
AC_PROG_CC
AC_PROG_CXX
AC_CANONICAL_SYSTEM

AX_BOOST_BASE([1.41])
AX_BOOST_SYSTEM
AX_BOOST_FILESYSTEM
AX_BOOST_THREAD
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS([tinyxml.h],,[AC_MSG_ERROR(tinyxml-devel must be installed)])
AC_CHECK_HEADERS([sndfile.h],,[AC_MSG_ERROR(libsndfile-devel must be installed)])

dnl Initialize automake
AM_INIT_AUTOMAKE

dnl this allows us specify individual liking flags for each target
AM_PROG_CC_C_O 

dnl Initialize Libtool
LT_INIT

dnl Check if Libtool is present
dnl Libtool is used for building share libraries 
AC_PROG_LIBTOOL

AC_CONFIG_FILES(Makefile
                exampleProgram/Makefile
                src/Makefile
                include/Makefile)
AC_OUTPUT
