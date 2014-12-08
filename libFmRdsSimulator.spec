Name:           libFmRdsSimulator
Version:        1.0.0
Release:        0%{?dist}
Summary:        RF Simulator Library - FmRdsSimulator

Group:          REDHAWK
License:        GPLv3
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  autoconf automake libtool 
%if 0%{?rhel} >= 6 || 0%{?fedora} >= 12
BuildRequires:  boost-devel >= 1.41
%else
BuildRequires:  boost141-devel
%endif

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
pushd cpp
./reconf
%configure
make %{?_smp_mflags}
popd


%install
rm -rf $RPM_BUILD_ROOT
pushd cpp
make install DESTDIR=$RPM_BUILD_ROOT
popd


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%{_prefix}/dom/deps/%{name}/dsp.spd.xml
%{_prefix}/dom/deps/%{name}/cpp

%files devel
%defattr(-,root,root,-)
%{_prefix}/dom/deps/%{name}/cpp/include
%{_prefix}/dom/deps/%{name}/cpp/lib/pkgconfig

