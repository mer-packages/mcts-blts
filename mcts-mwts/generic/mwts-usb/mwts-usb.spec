# spec file for mwts-usb

%define release		0
%define buildroot	%{_topdir}/%{name}-%{version}-root

BuildRoot:		%{buildroot}
Summary: 		Test libraries for USB networking.
License: 		LGPL
Name: 			mwts-usb
Version: 		1.0.3
Release: 		0
Prefix: 		/usr
Group: 			Development/Tools
BuildRequires:		qt-devel, min-devel, mwts-common-devel
Requires:		mwts-common
Source: 		%{name}-%{version}.tar.gz
%description
Test libraries for USB networking. C++ and MIN test modules.

%package		generic-scripts
Summary:		Test cases for USB networking.
Prefix: 		/usr
Group: 			Development/Tools
Requires:		mwts-usb
%description		generic-scripts
MIN test cases for measuring performance and reliability

%package		generic-config
Summary:		mwts-usb generic configuration file
Prefix: 		/usr
Group: 			Development/Tools
Requires:		mwts-usb
%description		generic-config
mwts-usb generic configuration file


%package		generic-all
Summary:		mwts-usb generic meta package
Prefix: 		/usr
Group: 			Development/Tools
Requires:		mwts-usb, mwts-usb-generic-config, mwts-usb-generic-scripts
%description		generic-all
mwts-usb generic meta package
            
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

%files generic-scripts
/etc/min.d/mwts-usb.min.conf
/usr/lib/min/*.cfg

%files generic-config
/usr/lib/tests/*

%files generic-all
%doc README

%post
ldconfig

%postun
ldconfig
