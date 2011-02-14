# spec file for mwts-telepathy

%define buildroot	%{_topdir}/%{name}-%{version}-%{release}-root

BuildRoot:		%{buildroot}
Summary: 		A MWTS test asset for Telepathy-QT4
License: 		LGPL
Name: 			mwts-telepathy
Version: 		0.1.4
Release: 		0
Prefix: 		/usr
Group: 			Development/Tools
BuildRequires:		libqt-devel, min-devel, min, compat-telepathy-qt4-devel, mwts-common-devel
Requires:		min, compat-telepathy-qt4, mwts-common
Source: 		%{name}-%{version}.tar.gz

%description
Mwts-telepathy is a MWTS test asset for Telepathy-QT4. It provides test cases for all available account types.

%package		generic-tests
Summary:		Mwts-telepathy MIN files
Prefix: 		/usr
Group: 			Development/Tools
Requires:		min, mwts-telepathy
%description	generic-tests
MIN test cases for mwts-telepathy


%package		generic-config
Summary:		Mwts-telepathy generic config
Prefix: 		/usr
Group: 			Development/Tools
Requires:		mwts-telepathy
%description	generic-config
Mwts-telepathy generic config


%package		generic-all
Summary:		Mwts-telepathy generic meta package
Prefix: 		/usr
Group: 			Development/Tools
Requires:		mwts-telepathy, mwts-telepathy-generic-config, mwts-telepathy-generic-tests
%description	generic-all
Mwts-telepathy generic meta package

%prep
%setup -q -n %{name}-%{version}

%build
qmake
make

%install
make install INSTALL_ROOT=%{buildroot}

%files 
%doc README
/usr/lib/libmwts-telepathy.*
/usr/share/telepathy/clients/*.client

%files generic-tests
/usr/lib/min/libmin-mwts-telepathy.*
/etc/min.d/*.min.conf
/usr/lib/min/*.cfg
/usr/share/mwts-telepathy-tests/tests.xml

%files generic-config
/usr/lib/tests/*.conf

%files generic-all

%post
ldconfig

%postun
ldconfig
