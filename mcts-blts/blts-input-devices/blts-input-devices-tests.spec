Summary: BLTS input devices test set
Name: blts-input-devices-tests
Version: 0.0.1
Release: 1
License: GPLv2
Group: Development/Testing
URL: http://wiki.meego.com/Quality/TestSuite/MCTS
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: libbltscommon-devel libbltscommon-devel libX11-devel kernel-headers
Requires: blts-input-devices-tests-config libbltscommon1 libX11

%package config-example
Summary: BLTS input devices test config example
Provides: blts-input-devices-tests-config

%description
This package contains functional tests for input devices.

%description config-example
This package contains configuration example for input devices functional tests.

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

%files config-example
/etc/blts/blts-input-devices-tests.cnf
/usr/share/blts-input-devices-tests/tests.xml
