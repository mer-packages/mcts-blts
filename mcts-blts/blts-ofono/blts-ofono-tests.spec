Summary: BLTS oFono functional tests
Name: blts-ofono-tests
Version: 0.1.11
Release: 1
License: GPLv2
Group: Development/Testing
URL: https://cwiki.nokia.com/BaseLayerTestSuite/
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
#BuildRequires: libbltscommon-dev
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
/usr/share/%{name}/tests.xml

%files config
/etc/blts/blts-ofono-tests.cnf
