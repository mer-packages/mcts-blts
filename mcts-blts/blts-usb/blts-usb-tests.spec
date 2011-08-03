Summary: BLTS USB low-level tests
Name: blts-usb-tests
Version: 0.0.20
Release: 1
License: GPLv2
Group: Development/Testing
URL: http://wiki.meego.com/Quality/TestSuite/MCTS
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: libbltscommon-devel, kernel-devel
Requires: blts-usb-tests-config

%package config-example
Summary: BLTS USB low-level tests config example
Provides: blts-usb-tests-config

%package config-n900
Summary: BLTS USB low-level tests config for N900
Provides: blts-usb-tests-config

%description
This package contains functional tests for USB multi-endpoint transfers.

%description config-example
This package contains an example configuration for USB functional tests.

%description config-n900
This package contains a configuration for N900 USB functional tests.

%prep
%setup -q

%build
./autogen.sh
%configure
KERNELDIR=`rpm -ql --whatprovides kernel-devel | grep -m 1 -E '^\/usr\/src\/kernels\/[^\/]*/Makefile' | xargs dirname` make

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%doc README COPYING
/usr/bin/*
/usr/lib/tests/%{name}/*

%files config-example
%defattr(-,root,root,-)
%config /etc/blts/blts-usb-testrunner-default.cnf
%config /etc/blts/ep-configuration-default.cfg
/usr/share/blts-usb-tests/tests.xml

%files config-n900
%defattr(-,root,root,-)
%config /etc/blts/blts-usb-testrunner-bulk.cnf
%config /etc/blts/blts-usb-testrunner-isoc.cnf
%config /etc/blts/blts-usb-testrunner-int.cnf
%config /etc/blts/ep-configuration-bulk.cfg
%config /etc/blts/ep-configuration-isoc.cfg
%config /etc/blts/ep-configuration-int.cfg
/usr/share/blts-usb-tests/tests.xml
