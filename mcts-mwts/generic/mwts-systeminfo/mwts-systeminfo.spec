# spec file for mwts-systemInfo

%define buildroot	%{_topdir}/%{name}-%{version}-root

BuildRoot:      %{buildroot}
Name:           mwts-systemInfo
Summary:        systemInfo test asset
License:        LGPL
Version:        %{version}
Release:        0
Prefix:         /usr
Group:          Development/Tools
BuildRequires:  qt-devel, min-devel, min, mwts-common-devel
Requires:       mwts-common
Source:         %{name}-%{version}.tar.gz

%description
systemInfo test asset.
            
%package        scripts-generic
Summary:        MIN test case scripts for mwts-systemInfo
Requires:       mwts-systemInfo
%description    scripts-generic
MIN test case scripts for mwts-systemInfo

%package        config-generic
Summary:        Generic configuration file for mwts-systemInfo
Requires:       mwts-systemInfo
%description    config-generic
Generic configuration file for mwts-systemInfo

%package	all-generic
Summary:	meta package containing everything for mwts-systemInfo (generic)
Requires:	mwts-systemInfo, mwts-systemInfo-scripts-generic, mwts-systemInfo-config-generic
%description	all-generic
Meta package for installing all needed packages for generic version of mwts-systemInfo


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
/usr/lib/libmwts-systemInfo*
/usr/lib/min/*.so*


%files scripts-generic
/etc/min.d/*.min.conf
/usr/share/mwts-systemInfo-scripts/tests.xml
/usr/lib/min/*.cfg

%files config-generic
/usr/lib/tests/*

%files all-generic
