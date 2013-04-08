Summary: BLTS XVideo test set
Name: blts-xvideo-tests
Version: 0.0.14
Release: 1
License: GPLv2
Group: Development/Testing
URL: https://github.com/mer-packages/mcts-blts
Source0: %{name}-%{version}.tar.gz
BuildRequires: libbltscommon-devel
BuildRequires: libbltspixelformat-devel
BuildRequires: libX11-devel
BuildRequires: libXv-devel
Requires: blts-xvideo-tests-config
%define _prefix /opt/tests/%{name}

%package config-example
Summary: BLTS XVideo test config example
Provides: blts-xvideo-tests-config

%description
This package contains functional tests for XVideo.

%description config-example
This package contains configuration example for XVideo functional tests.

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
%{_prefix}/bin/*
%{_prefix}/cfg/*

%files config-example
%defattr(-,root,root,-)
%{_prefix}/cnf/*
%{_prefix}/tests.xml
