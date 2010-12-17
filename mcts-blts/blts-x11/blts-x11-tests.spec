Summary: BLTS X11 test set
Name: blts-x11-tests
Version: 0.0.15
Release: 1
License: GPLv2
Group: Development/Testing
URL: http://wiki.meego.com/Quality/TestSuite/MCTS
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: libbltscommon-devel libX11-devel libXdamage-devel
BuildRequires: libXcomposite-devel libXrandr-devel libXrender-devel
BuildRequires: libXtst-devel libXv-devel libXi-devel
Requires: libbltscommon1 libX11 libXdamage libXcomposite libXrandr
Requires: libXrender libXtst libXv libXi

%description
This package contains functional tests for X11.

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
/usr/bin/*
/usr/lib/tests/%{name}/*
/usr/share/%{name}/*
