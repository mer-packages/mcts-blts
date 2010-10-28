# spec file for mwts-usb

%define release		0
%define buildroot	%{_topdir}/%{name}-%{version}-root

BuildRoot:		%{buildroot}
Summary: 		Test libraries for USB networking.
License: 		LGPL
Name: 			%{name}
Version: 		%{version}
Release: 		%{release}
Prefix: 		/usr
Group: 			Development/Tools
BuildRequires:		qt-devel, min-devel, mwts-common-devel
Requires:		mwts-common
Source: 		%{name}-%{version}.tar.gz
%description
Test libraries for USB networking. C++ and MIN test modules.

%package		scripts
Summary:		Test cases for USB networking.
Prefix: 		/usr
Group: 			Development/Tools
Requires:		mwts-usb
%description	scripts
MIN test cases for measuring performance and reliability
            
%prep
%setup -q

%build
qmake
make

%install
make install INSTALL_ROOT=%{buildroot}

%files 
%doc README
%doc doc/MWTS.README
/usr/lib/*.so*
/usr/lib/min/*.so*
/usr/lib/tests/*
#/usr/include/*

%files scripts
%doc README
%doc doc/MWTS.README
/etc/min.d/mwts-usb.min.conf
/usr/share/mwts-usb-scripts/tests.xml
/usr/lib/min/*.cfg
