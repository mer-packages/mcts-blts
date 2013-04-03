Summary: BLTS OpenGL ES2 test set
Name: blts-opengles2-tests
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
BuildRequires: libXext-devel
BuildRequires: pkgconfig(egl)
BuildRequires: pkgconfig(glesv2)
%define _prefix /opt/tests/%{name}

%description
This package contains functional and performance tests for OpenGL ES2.

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
