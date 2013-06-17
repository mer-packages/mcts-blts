Summary: BLTS Watchdog test set
Name: blts-watchdog-tests
Version: 0.0.14
Release: 1
License: GPLv2
Group: Development/Testing
URL: https://github.com/mer-packages/mcts-blts
Source0: %{name}-%{version}.tar.gz
BuildRequires: libbltscommon-devel
Requires: blts-tools
%define _prefix /opt/tests/%{name}

%description
This package contains functional tests for watchdog drivers.

%prep
%setup -q

%build
./autogen.sh
%configure --prefix=%{_prefix} --libdir=%{_prefix}
make

%install
make install DESTDIR=$RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%doc README COPYING
%{_prefix}/*
