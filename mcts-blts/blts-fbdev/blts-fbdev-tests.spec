Summary: BLTS FBdev test set
Name: blts-fbdev-tests
Version: 0.0.14
Release: 1
License: GPLv2
Group: Development/Testing
URL: https://github.com/mer-packages/mcts-blts
Source0: %{name}-%{version}.tar.gz
BuildRequires: libbltscommon-devel
BuildRequires: glib2-devel
Requires: blts-fbdev-tests-config
%define _prefix /opt/tests/%{name}

%package config-example
Summary: BLTS fbdev test config example
Provides: blts-fbdev-tests-config
Requires: blts-tools

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
make install DESTDIR=$RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%doc README COPYING
%{_prefix}/bin/*

%files config-example
%defattr(-,root,root,-)
%{_prefix}/cnf/blts-fbdev-tests.cnf
%{_prefix}/tests.xml
