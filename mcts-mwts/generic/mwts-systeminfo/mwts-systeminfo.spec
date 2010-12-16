# spec file for mwts-systeminfo

%define buildroot	%{_topdir}/%{name}-%{version}-root

BuildRoot:      %{buildroot}
Name:           mwts-systeminfo
Summary:        systeminfo test asset
License:        LGPL
Version:        %{version}
Release:        0
Prefix:         /usr
Group:          Development/Tools
BuildRequires:  qt-devel, qt-mobility-devel, min-devel, min, mwts-common-devel
Requires:       mwts-common
Source:         %{name}-%{version}.tar.gz

%description
systeminfo test asset.
            
%package        scripts-generic
Summary:        MIN test case scripts for mwts-systeminfo
Requires:       min, mwts-systeminfo
Prefix:          /usr
Group:          Development/Tools
%description    scripts-generic
MIN test case scripts for mwts-systeminfo


%package        config-generic
Summary:        Generic configuration file for mwts-systeminfo
Requires:       mwts-systeminfo
%description    config-generic
Generic configuration file for mwts-systeminfo

%package	all-generic
Summary:	meta package containing everything for mwts-systeminfo (generic)
Requires:	mwts-systeminfo, mwts-systeminfo-scripts-generic, mwts-systeminfo-config-generic
%description	all-generic
Meta package for installing all needed packages for generic version of mwts-systeminfo


%prep
%setup -q

%build
qmake "CONFIG+=plugin"
make

%install
make install INSTALL_ROOT=%{buildroot}

%files 
%doc README
%doc doc/MWTS.README
/usr/lib/libmwts-systeminfo*
/usr/lib/min/*.so*


%files scripts-generic
/etc/min.d/*.min.conf
/usr/share/mwts-systeminfo-scripts/tests.xml
/usr/lib/min/*.cfg

%files config-generic
/usr/lib/tests/*

%files all-generic
