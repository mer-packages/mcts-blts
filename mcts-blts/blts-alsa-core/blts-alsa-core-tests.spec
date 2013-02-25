Summary: BLTS ALSA core test set
Name: blts-alsa-core-tests
Version: 0.0.14
Release: 1
License: GPLv2
Group: Development/Testing
URL: http://wiki.meego.com/Quality/TestSuite/Audio_Driver_Test_Specification
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: libbltscommon-devel kernel-headers dbus-glib-devel glib2-devel
Requires: blts-alsa-core-tests-config
%define _prefix /opt/tests/%{name}

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
%configure --prefix=%{_prefix} --libdir=%{_prefix}
make

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%doc README COPYING
%{_prefix}/bin/*
%{_prefix}/cfg/blts-alsa-core-req-files.cfg

%files config-example
%defattr(-,root,root,-)
%{_prefix}/cnf/blts-alsa-core-tests.cnf
%{_prefix}/tests.xml

