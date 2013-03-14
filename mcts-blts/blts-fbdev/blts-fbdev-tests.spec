Summary: BLTS FBdev test set
Name: blts-fbdev-tests
Version: 0.0.13
Release: 1
License: GPLv2
Group: Development/Testing
URL: http://wiki.meego.com/Quality/TestSuite/Video_Playback_Driver_Test_Specification
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: libbltscommon-devel glib2-devel
Requires: blts-fbdev-tests-config
%define _prefix /opt/tests/%{name}

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
%attr(4755, root, root) %{_prefix}/bin/run-binary

%files config-example
%defattr(-,root,root,-)
%{_prefix}/cnf/blts-fbdev-tests.cnf
%{_prefix}/tests.xml
