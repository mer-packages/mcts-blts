# spec file for mwts-telepathy

%define buildroot	%{_topdir}/%{name}-%{version}-root

BuildRoot:		%{buildroot}
Summary: 		Mwts-telepathy is a MWTS test asset for Telepathy-QT4
License: 		LGPL
Name: 			mwts-telepathy
Version: 		0.1.3
Release: 		0
Prefix: 		/usr
Group: 			Development/Tools
BuildRequires:	qt-devel, min-devel, min, telepathy-qt4-devel
Requires:		min, telepathy-qt4
Source: 		%{name}-%{version}.tar.gz

%description
Mwts-telepathy is a MWTS test asset for Telepathy-QT4. It provides test cases for all available account types.

%package		tests
Summary:		Mwts-telepathy MIN files
Prefix: 		/usr
Group: 			Development/Tools
Requires:		min, mwts-common
%description	tests
MIN test cases for mwts-telepathy

%package		cli
Summary:		Command line interface for mwts-telepathy
Prefix: 		/usr
Group: 			Development/Tools
Requires:		min, mwts-common
%description	cli
Command line interface for mwts-telepathy.

%prep
%setup -q -n %{name}-%{version}

%build
qmake "CONFIG+=plugin"
make

%install
make install INSTALL_ROOT=%{buildroot}

%files 
%doc README
/usr/lib/libmwts-telepathy.*
/usr/share/telepathy/clients/*.client
/usr/lib/tests/*.conf


%files tests
%doc README
/etc/min.d/*.min.conf
/usr/lib/min/*.cfg
/usr/lib/min/libmin-mwts-telepathy.*

%files cli
%doc README
/usr/bin/mwts-telepathy-cli

%post
mkdir -p /var/log/tests
chmod 777 /var/log/tests

%postun
ldconfig
