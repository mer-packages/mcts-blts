Summary: BLTS oFono functional tests
Name: blts-ofono-tests
Version: 0.1.24
Release: 1
License: GPLv2
Group: Development/Testing
URL: https://github.com/mer-packages/mcts-blts
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: libbltscommon-devel dbus-devel dbus-glib-devel glib2-devel
Requires: blts-ofono-tests-config dbus
%define _prefix /opt/tests/%{name}

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

%files config
%defattr(-,root,root,-)
%{_prefix}/cnf/blts-ofono-tests.cnf
%{_prefix}/tests.xml
