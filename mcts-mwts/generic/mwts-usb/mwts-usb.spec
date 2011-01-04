# spec file for mwts-usb

%define release		0
%define buildroot	%{_topdir}/%{name}-%{version}-root

BuildRoot:		%{buildroot}
Summary: 		Test libraries for USB networking.
License: 		LGPL
Name: 			mwts-usb
Version: 		0.0.6
Release: 		0
Prefix: 		/usr
Group: 			Development/Tools
BuildRequires:	qt-devel, min-devel, mwts-common-devel
Requires:		mwts-common
Source: 		%{name}-%{version}.tar.gz
%description
Test libraries for USB networking. C++ and MIN test modules.

%package		tests-generic
Summary:		Test cases for USB networking.
Prefix: 		/usr
Group: 			Development/Tools
Requires:		mwts-usb-tests
%description	tests-generic
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

%files tests-generic
%doc README
%doc doc/MWTS.README
/etc/min.d/mwts-usb.min.conf
/usr/share/mwts-usb-scripts/tests.xml
/usr/lib/min/*.cfg