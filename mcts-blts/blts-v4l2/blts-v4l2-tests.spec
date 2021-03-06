Summary: BLTS V4L2 test set
Name: blts-v4l2-tests
Version: 0.3.13
Release: 1
License: GPLv2
Group: Development/Testing
URL: https://github.com/mer-packages/mcts-blts
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: libbltscommon-devel libbltspixelformat-devel
BuildRequires: libjpeg-devel libX11-devel libXv-devel
Requires: blts-v4l2-tests-config
%define _prefix /opt/tests/%{name}

%package config-example
Summary: BLTS V4L2 test config example
Provides: blts-v4l2-tests-config

%description
This package contains functional tests for V4L2.

%description config-example
This package contains configuration example for V4L2 functional tests.

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
%{_prefix}/cfg/*

%files config-example
%defattr(-,root,root,-)
%{_prefix}/cnf/*
%{_prefix}/tests.xml
