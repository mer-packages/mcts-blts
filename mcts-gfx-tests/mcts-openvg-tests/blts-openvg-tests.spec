Summary: Graphics subsystem OpenVG test set
Name: blts-openvg-tests
Version: 0.0.11
Release: 1
License: GPLv2
Group: Development/Testing
URL: http://wiki.meego.com/Quality/TestSuite/
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: libbltscommon-devel libX11-devel pkgconfig(egl) pkgconfig(vg)
Requires: libbltscommon1 libX11

%description
This package contains functional and performance tests for OpenVG

%prep
%setup -q

%build
./autogen.sh
%configure
make

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
%doc README COPYING
/usr/bin/*
/usr/share/%{name}/*
#/usr/lib/tests/%{name}/*
