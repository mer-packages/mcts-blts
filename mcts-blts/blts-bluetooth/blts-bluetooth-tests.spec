Summary: BLTS Bluetooth functional tests
Name: blts-bluetooth-tests
Version: 0.2.10
Release: 1
License: GPLv2
Group: Development/Testing
URL: http://wiki.meego.com/Quality/TestSuite/MCTS
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: libbltscommon-devel bluez-libs-devel
Requires: bluez bluez-hcidump

%description
This package contains functional tests for the Bluez bluetooth stack.

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
%defattr(-,root,root,-)
%doc README COPYING
/usr/bin/*
/usr/lib/tests/%{name}/*
/usr/share/%{name}/tests.xml
%config /etc/blts/blts-bluetooth-tests.cnf
