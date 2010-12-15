# spec file for mwts-messaging

%define buildroot	%{_topdir}/%{name}-%{version}-root

BuildRoot:		%{buildroot}
Summary: 		Mwts-messaging is a test asset for Qt Mobility Messaging
License: 		GPL
Name: 			mwts-messaging-tests
Version: 		0.0.1
Release: 		0
Prefix: 		/usr
Group: 			Development/Tools
BuildRequires:	qt-devel, mwts-common-devel
Requires:		qt-x11, min, mwts-common
Source: 		%{name}-%{version}.tar.gz

%description
Mwts-messaging-tests is a test asset for Qt Mobility Messaging. It supports currently only SMS.
            
%package		tests
Summary:		Mwts-messaging MIN files
Prefix: 		/usr
Group: 			Development/Tools
Requires:		min, mwts-common, mwts-messaging-tests
%description	tests
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
/usr/lib/tests/*

%files tests
%doc README
/etc/min.d/*.min.conf
/usr/lib/min/*.cfg
/usr/lib/min/libmin-mwts-messaging.*
/usr/share/mwts-messaging-*/*.xml

%postun
ldconfig
