Summary: BLTS oFono functional tests
Name: blts-ofono-tests
Version: 0.1.12
Release: 1
License: GPLv2
Group: Development/Testing
URL: http://wiki.meego.com/Quality/TestSuite/oFono_Modem_API_Test_Plan
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: libbltscommon-devel
Requires: blts-ofono-tests-config

%package config
Summary: BLTS oFono tests default config

%description
This package contains oFono tests

%description config
This package contains default configuration for oFono tests

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

%files config
/etc/blts/blts-ofono-tests.cnf
/usr/share/blts-ofono-tests/tests.xml
