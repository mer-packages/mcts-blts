Summary: BLTS XVideo test set
Name: blts-xvideo-tests
Version: 0.0.7
Release: 1
License: GPLv2
Group: Development/Testing
URL: http://wiki.meego.com/Quality/TestSuite/Video_Playback_Driver_Test_Specification
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: libbltscommon-devel libbltspixelformat-devel
BuildRequires: libX11-devel libXv-devel
Requires: blts-xvideo-tests-config libbltscommon1 libbltspixelformat1
Requires: libX11 libXv

%package config-example
Summary: BLTS XVideo test config example
Provides: blts-xvideo-tests-config

%description
This package contains functional tests for XVideo.

%description config-example
This package contains configuration example for XVideo functional tests.

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
/etc/blts/blts-xvideo-tests.cnf
/usr/share/blts-xvideo-tests/tests.xml
