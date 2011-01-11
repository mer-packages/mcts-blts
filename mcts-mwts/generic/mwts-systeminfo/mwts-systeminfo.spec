# spec file for mwts-systeminfo

%define buildroot	%{_topdir}/%{name}-%{version}-root

BuildRoot:      %{buildroot}
Name:           mwts-systeminfo
Summary:        systeminfo test asset
License:        LGPL
Version:        1.0.0
Release:        0
Prefix:         /usr
Group:          Development/Tools
BuildRequires:  qt-devel, qt-mobility-devel, min-devel, min, mwts-common-devel
Requires:       mwts-common
Source:         %{name}-%{version}.tar.gz

%description
systeminfo test asset.
            
%package        generic-tests
Summary:        MIN test case scripts for mwts-systeminfo
Requires:       min, mwts-systeminfo
Prefix:          /usr
Group:          Development/Tools
%description    generic-tests
MIN test case scripts for mwts-systeminfo


%package        generic-config
Summary:        Generic configuration file for mwts-systeminfo
Requires:       mwts-systeminfo
%description    generic-config
Generic configuration file for mwts-systeminfo

%package	generic-all
Summary:	meta package containing everything for mwts-systeminfo (generic)
Requires:	mwts-systeminfo, mwts-systeminfo-generic-tests, mwts-systeminfo-generic-config
%description	generic-all
Meta package for installing all needed packages for generic version of mwts-systeminfo


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
%doc COPYING
%doc DEPENDENCIES.png
/usr/lib/libmwts-systeminfo.*
/usr/lib/min/*.so

%files generic-tests
/etc/min.d/*.min.conf
/usr/share/mwts-systeminfo-scripts/tests.xml
/usr/lib/min/*.cfg

%files generic-config
/usr/lib/tests/*

%files generic-all

%post
ldconfig

%postun
ldconfig
