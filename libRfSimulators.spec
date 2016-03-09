#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK librfsimulators.
#
# REDHAWK librfsimulators is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK librfsimulators is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
Name:           libRfSimulators
Version:        1.1.0
Release:        3%{?dist}
Summary:        RF Simulator Library - FM RDS Simulator.

Group:          REDHAWK
License:        GPLv3
Source0:        %{name}-%{version}.tar.gz
Source1:        libFmRdsSimulatorExamples.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  autoconf automake libtool gcc-c++
BuildRequires:  boost-devel >= 1.41
BuildRequires:  tinyxml-devel >= 2.6.1
BuildRequires:  libsndfile-devel >= 1.0
BuildRequires:  log4cxx-devel >= 0.10.0
BuildRequires:  fftw-devel >= 3.0.0

%description
RF Simulator Library %{name}
 * Commit: __REVISION__
 * Source Date/Time: __DATETIME__

%package devel
Summary:        %{name} development package
Group:          REDHAWK
Requires:       %{name} = %{version}
Requires:       boost-devel >= 1.41
Requires:       tinyxml-devel >= 2.6.1
Requires:       libsndfile-devel >= 1.0
Requires:       log4cxx-devel >= 0.10.0
Requires:       fftw-devel >= 3.0.0

%description devel
Development headers and libraries for %{name}


%prep
%setup -q


%build
./reconf
%configure
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/%{_datadir}
tar -xzvf %{_sourcedir}/libFmRdsSimulatorExamples.tar.gz -C $RPM_BUILD_ROOT/%{_datadir}

%clean
rm -rf $RPM_BUILD_ROOT

%files
%{_libdir}/*.so.*
%{_datadir}/*

%files devel
%{_includedir}/*
%{_libdir}/*.so
%{_libdir}/*.la
%{_libdir}/*.a
%{_libdir}/pkgconfig/librfsimulators.pc
