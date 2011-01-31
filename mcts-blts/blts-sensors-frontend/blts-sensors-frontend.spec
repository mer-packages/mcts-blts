Summary: BLTS Sensor test front-end module
Name: blts-sensors-frontend
Version: 0.1.4
Release: 1
License: GPLv2
Group: Development/Testing
URL: http://wiki.meego.com/Quality/TestSuite/MCTS
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: libbltscommon-devel

%description
Main package for the BLTS sensor test module, containing the test runner
front-end. Sensor plug-ins are required for executing tests.

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
%doc README README.api COPYING
/usr/bin/*
# TODO: Create separate devel package
/usr/include/blts_sensor_plugin.h
/usr/include/blts_sensor_variable.h
