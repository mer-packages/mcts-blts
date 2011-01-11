# spec file for mwts-ofono

#%define release		0
%define buildroot	%{_topdir}/%{name}-%{version}-root

BuildRoot:	%{buildroot}
Summary: 	Test libraries for ofono-qt
License: 	LGPL
Name: 		%{name}
Version: 	0.0.1
Release: 	%{release}
Prefix: 	/usr
Group: 		Development/Tools
BuildRequires:	qt-devel, min-devel, mwts-common-devel, libofono-qt-devel
Requires:	mwts-common, ofono, libofono-qt
Source: 	%{name}-%{version}.tar.gz
%description
Test cases for ofono-qt

%package	generic-tests
Summary:	Test cases for ofono-qt
Prefix: 	/usr
Group: 		Development/Tools
Requires:	mwts-ofono
%description	generic-tests
MIN test cases for measuring performance and reliability

%package        generic-config
Summary:       	Generic configuration file for mwts-ofono
Requires:       mwts-ofono
%description    generic-config
Generic configuration file for mwts-ofono

%package	generic-all
Summary:	meta package containing everything for mwts-ofono (generic)
Requires:	mwts-ofono, mwts-ofono-generic-tests, mwts-ofono-generic-config
%description	generic-all
Meta package for installing all needed packages for generic version of mwts-ofono

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

%files generic-tests
/etc/min.d/mwts-ofono.min.conf
/usr/share/mwts-ofono-scripts/tests.xml
/usr/lib/min/*.cfg

%files generic-config
/usr/lib/tests/*

%files generic-all

%post
ldconfig

%postun
ldconfig

%changelog
* Tue Dec 22 2010 Balazs Sipos <balazs.sipos@digia.com> 0.0.1
- Intial version
- Added test cases
- Added content to README
- Refactored package names in mwts-ofono.spec


