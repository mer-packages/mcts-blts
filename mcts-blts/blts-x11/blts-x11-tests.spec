Summary: BLTS X11 test set
Name: blts-x11-tests
Version: 0.0.21
Release: 1
License: GPLv2
Group: Development/Testing
URL: https://github.com/mer-packages/mcts-blts
Source0: %{name}-%{version}.tar.gz
BuildRequires: libbltscommon-devel
BuildRequires: libX11-devel
BuildRequires: libXdamage-devel
BuildRequires: libXcomposite-devel
BuildRequires: libXrandr-devel
BuildRequires: libXrender-devel
BuildRequires: libXtst-devel
BuildRequires: libXv-devel
BuildRequires: libXi-devel
%define _prefix /opt/tests/%{name}

%description
This package contains functional tests for X11.

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
