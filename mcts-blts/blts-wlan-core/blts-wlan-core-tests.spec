Summary: BLTS WLAN low-level test set
Name: blts-wlan-core-tests
Version: 0.1.4
Release: 1
License: GPLv2
Group: Development/Testing
URL: http://wiki.meego.com/Quality/TestSuite/MCTS
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: libbltscommon-devel, libnl-devel
Requires: blts-wlan-core-tests-config

%package config-example
Summary: BLTS WLAN low-level test config example
Provides: blts-wlan-core-tests-config

%description
This package contains functional tests for WLAN.

%description config-example
This package contains configuration example for WLAN functional tests.

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
/etc/blts/blts-wlan-core-tests.cnf
/usr/share/blts-wlan-core-tests/tests.xml
