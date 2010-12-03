# spec file for mwts-accounts

%define buildroot	%{_topdir}/%{name}-%{version}-root

BuildRoot:      %{buildroot}
Name:           mwts-accounts
Summary:        Template test asset
License:        LGPL
Version:        %{version}
Release:        0
Prefix:         /usr
Group:          Development/Tools
BuildRequires:  qt-devel, min-devel, min, mwts-common-devel
Requires:       mwts-common
Source:         %{name}-%{version}.tar.gz

%description
Template test asset.
            
%package        scripts-generic
Summary:        MIN test case scripts for mwts-accounts
Requires:       mwts-accounts
%description    scripts-generic
MIN test case scripts for mwts-accounts

%package        config-generic
Summary:        Generic configuration file for mwts-accounts
Requires:       mwts-accounts
%description    config-generic
Generic configuration file for mwts-accounts

%package	all-generic
Summary:	meta package containing everything for mwts-accounts (generic)
Requires:	mwts-accounts, mwts-accounts-scripts-generic, mwts-accounts-config-generic
%description	all-generic
Meta package for installing all needed packages for generic version of mwts-accounts


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
/usr/lib/libmwts-accounts*
/usr/lib/min/*.so*


%files scripts-generic
/etc/min.d/*.min.conf
/usr/share/mwts-accounts-scripts/tests.xml
/usr/lib/min/*.cfg

%files config-generic
/usr/lib/tests/*

%files all-generic