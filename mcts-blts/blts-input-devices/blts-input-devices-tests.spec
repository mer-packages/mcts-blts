Summary: BLTS input devices test set
Name: blts-input-devices-tests
Version: 0.0.6
Release: 1
License: GPLv2
Group: Development/Testing
URL: https://github.com/mer-packages/mcts-blts
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: libbltscommon-devel libX11-devel kernel-headers
Requires: blts-input-devices-tests-config
%define _prefix /opt/tests/%{name}

%package config-example
Summary: BLTS input devices test config example
Provides: blts-input-devices-tests-config

%description
This package contains functional tests for input devices.

%description config-example
This package contains configuration example for input devices functional tests.

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

%files config-example
%defattr(-,root,root,-)
%{_prefix}/cnf/blts-input-devices-tests.cnf
%{_prefix}/tests.xml
