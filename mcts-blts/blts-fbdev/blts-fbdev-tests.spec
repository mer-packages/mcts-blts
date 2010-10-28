Summary: BLTS FBdev test set
Name: blts-fbdev-tests
Version: 0.0.6
Release: 1
License: GPLv2
Group: Development/Testing
URL: http://wiki.meego.com/Quality/TestSuite/Video_Playback_Driver_Test_Specification
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: libbltscommon-devel
Requires: blts-fbdev-tests-config libbltscommon1

%package config-example
Summary: BLTS fbdev test config example
Provides: blts-fbdev-tests-config

%description
This package contains functional tests for fbdev.

%description config-example
This package contains configuration example for fbdev functional tests.

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
/usr/lib/tests/%{name}/*

%files config-example
/etc/blts/blts-fbdev-tests.cnf
/usr/share/blts-fbdev-tests/tests.xml
