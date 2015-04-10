/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK librfsimulators.
 *
 * REDHAWK librfsimulators is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK librfsimulators is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(librfsimulators, 1.0)
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([subdir-objects])
AC_PROG_CC
AC_PROG_CXX

AX_BOOST_BASE([1.41])
AX_BOOST_SYSTEM
AX_BOOST_FILESYSTEM
AX_BOOST_THREAD
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS([tinyxml.h],,[AC_MSG_ERROR(tinyxml-devel must be installed)])
AC_CHECK_HEADERS([sndfile.h],,[AC_MSG_ERROR(libsndfile-devel must be installed)])
AC_CHECK_HEADERS([log4cxx/log4cxx.h],,[AC_MSG_ERROR(log4cxx-devel must be installed)])

PKG_CHECK_MODULES([PROJECTDEPS],   [fftw3 >= 3.0 fftw3f >= 3.0])

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
