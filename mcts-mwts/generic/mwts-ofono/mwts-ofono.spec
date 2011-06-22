# spec file for mwts-ofono

#%define release		0
%define buildroot	 %{_tmppath}/%{name}-%{version}-%{release}-root

BuildRoot:	%{buildroot}
Summary: 	Test libraries for ofono-qt
License: 	LGPL
Name: 		mwts-ofono
Version: 	0.1.2
Release: 	%{release}
Prefix: 	/usr
Group: 		Development/Tools
BuildRequires:	qt-devel, min-devel, mwts-common-devel, libofono-qt-devel
Requires:	mwts-common, libofono-qt
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
/usr/share/mwts-ofono-generic-tests/tests.xml
/usr/lib/min/*.cfg

%files generic-config
/usr/lib/tests/*

%files generic-all

%post
ldconfig

%postun
ldconfig

%changelog
* Tue Jan 18 2011 Balazs Sipos <balazs.sipos@digia.com> - 0.1.0
- Added classes for sim manager and voice call (forwarding, waiting, barring)
- Refactored package names in mwts-ofono.spec
- Added functionality test cases to mwts-ofono-call.cfg
- Added content to README
* Fri Jan 07 2011 Balazs Sipos <balazs.sipos@digia.com> - 0.0.2
- Added functionality test cases to mwts-ofono-sim.cfg
* Wed Dec 22 2010 Balazs Sipos <balazs.sipos@digia.com> - 0.0.1
- Intial version


