# spec file for mwts-messaging

%define buildroot	%{_topdir}/%{name}-%{version}-root

BuildRoot:		%{buildroot}
Summary: 		Mwts-messaging is a test asset for Qt Mobility Messaging
License: 		LGPL
Name: 			mwts-messaging
Version: 		0.0.1
Release: 		0
Prefix: 		/usr
Group: 			Development/Tools
BuildRequires:	qt-devel, mwts-common-devel
Requires:		qt-x11, min, mwts-common
Source: 		%{name}-%{version}.tar.gz

%description
Mwts-messaging-tests is a test asset for Qt Mobility Messaging. It supports currently only SMS.
            
%package		tests-generic
Summary:		Mwts-messaging MIN files
Prefix: 		/usr
Group: 			Development/Tools
Requires:		min, mwts-common, mwts-messaging
%description	tests-generic
MIN test cases for mwts-messaging-tests

%prep
%setup -q -n %{name}-%{version}

%build
qmake "CONFIG+=plugin"
make

%install
make install INSTALL_ROOT=%{buildroot}

%files 
%doc README
/usr/lib/libmwts-messaging.*
/usr/lib/min/libmin-mwts-messaging.*
/usr/lib/tests/*
/usr/bin/mwts-messaging-cli

%files tests-generic
%doc README
/etc/min.d/*.min.conf
/usr/lib/min/*.cfg
/usr/share/mwts-messaging-*/*.xml

%postun
ldconfig
