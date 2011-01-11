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
BuildRequires:		qt-devel, mwts-common-devel
Requires:		qt-x11, min, mwts-common
Source: 		%{name}-%{version}.tar.gz

%description
Mwts-messaging-tests is a test asset for Qt Mobility Messaging. It supports currently only SMS.
            
%package		generic-tests
Summary:		Mwts-messaging MIN scipts
Prefix: 		/usr
Group: 			Development/Tools
Requires:		mwts-messaging
%description		generic-tests
MIN test cases for mwts-messaging

%package		generic-config
Summary:		Mwts-messaging generic configuration file
Group: 			Development/Tools
Requires:		mwts-messaging
%description		generic-config
Mwts-messaging generic configuration file

%package		generic-all
Summary:		Mwts-messaging generic meta package for all needed packages
Group: 			Development/Tools
Requires:		mwts-messaging, mwts-messaging-generic-config, mwts-messaging-generic-tests
%description		generic-all
Mwts-messaging meta package for all needed generic packages


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
/usr/bin/mwts-messaging-cli
/usr/lib/tests/*.txt

%files generic-tests
/etc/min.d/*.min.conf
/usr/lib/min/*.cfg
/usr/share/mwts-messaging/*.xml

%files generic-config
/usr/lib/tests/*.conf

%files generic-all

%post
ldconfig

%postun
ldconfig
