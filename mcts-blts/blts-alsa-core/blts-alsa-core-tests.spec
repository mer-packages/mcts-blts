Summary: BLTS ALSA core test set
Name: blts-alsa-core-tests
Version: 0.0.11
Release: 1
License: GPLv2
Group: Development/Testing
URL: http://wiki.meego.com/Quality/TestSuite/Audio_Driver_Test_Specification
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: libbltscommon-devel kernel-headers dbus-glib-devel glib2-devel
Requires: blts-alsa-core-tests-config

%package config-example
Summary: BLTS ALSA core test config example
Provides: blts-alsa-core-tests-config

%description
This package contains functional tests for ALSA core.

%description config-example
This package contains configuration example for functional tests for ALSA core.

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

%files config-example
%defattr(-,root,root,-)
%config /etc/blts/blts-alsa-core-tests.cnf
/usr/share/blts-alsa-core-tests/tests.xml

