Summary: BLTS example sensor plug-in
Name: libbltssensorplugin-example
Version: 0.1.2
Release: 1
License: GPLv2
Group: Development/Testing
URL: http://wiki.meego.com/Quality/TestSuite/MCTS
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: libbltscommon-devel blts-sensors-frontend
Requires: libbltscommon1 blts-sensors-frontend

%description
Example sensor test plug-in. Use blts-sensors-frontend to execute the tests.

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
/usr/lib/tests/blts-sensor-plugins/*
/etc/blts/blts-sensor-example.cnf
