Summary: Display test set 
Name: blts-display-test
Version: 0.0.10
Release: 1
License: GPLv2
Group: Development/Testing
URL: http://wiki.meego.com/Quality/TestSuite/MCTS
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: libbltscommon-devel libX11-devel libXrandr-devel
BuildRequires: libXrandr-devel libXrender-devel
Requires: libbltscommon1 libX11 libXrandr libXrender

%description
This package contains functional tests for display (through Xrandr).

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
/usr/share/%{name}/*
