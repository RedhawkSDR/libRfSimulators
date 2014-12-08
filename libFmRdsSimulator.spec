Name:           libFmRdsSimulator
Version:        1.0.0
Release:        0%{?dist}
Summary:        RF Simulator Library - FmRdsSimulator

Group:          REDHAWK
License:        GPLv3
Source0:        %{name}-%{version}.tar.gz
Source1:	libFmRdsSimulatorExamples.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  autoconf automake libtool gcc-c++
BuildRequires:  boost-devel >= 1.41
BuildRequires:  tinyxml-devel >= 2.6.1
BuildRequires:  libsndfile-devel >= 1.0
BuildRequires:  log4cxx-devel >= 0.10.0
BuildRequires:  fftw-devel >= 3.0.0

Requires:  	libsndfile >= 1.0
Requires:	boost >= 1.41
Requires:	log4cxx >= 0.10.0
Requires:	fftw >= 3.0.0
%description
RF Simulator Library %{name}
 * Commit: __REVISION__
 * Source Date/Time: __DATETIME__

%package devel
Summary:        %{name} development package
Group:          REDHAWK
Requires:       %{name} = %{version}
%if 0%{?rhel} >= 6 || 0%{?fedora} >= 12
Requires:       boost-devel >= 1.41
%else
Requires:       boost141-devel
%endif

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
mkdir -p $RPM_BUILD_ROOT/%{_datarootdir}
tar -xzvf %{_sourcedir}/libFmRdsSimulatorExamples.tar.gz -C $RPM_BUILD_ROOT/%{_datarootdir}

%clean
rm -rf $RPM_BUILD_ROOT

%files
%doc AUTHORS COPYING
%{_libdir}/*.so.*
%{_datarootdir}/*

%files devel
%{_includedir}/*
%{_libdir}/*.so
%{_libdir}/*.la
%{_libdir}/*.a
