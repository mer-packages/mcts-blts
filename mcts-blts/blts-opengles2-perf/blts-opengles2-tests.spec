Summary: BLTS OpenGL ES2 test set
Name: blts-opengles2-tests
Version: 0.0.11
Release: 1
License: GPLv2
Group: Development/Testing
URL: http://wiki.meego.com/Quality/TestSuite/OpenGLGLES_Performance_Test_Specification
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: libbltscommon-devel libX11-devel libXdamage-devel libXcomposite-devel
BuildRequires: libEGL-devel libGLESv2-devel
Requires: libbltscommon1 libX11 libXdamage libXcomposite libEGL libGLESv2

%description
This package contains functional and performance tests for OpenGL ES2.

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
