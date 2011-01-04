# spec file for mwts-accounts

%define buildroot	%{_topdir}/%{name}-%{version}-root

BuildRoot:      %{buildroot}
Name:           mwts-accounts
Summary:        Test asset for testing SSO and account management
License:        LGPL
Version:        0.0.1
Release:        0
Prefix:         /usr
Group:          Development/Tools
BuildRequires:  qt-devel, min-devel, min, mwts-common-devel, libsignon-devel libaccounts-qt-devel
Requires:       mwts-common
Source:         %{name}-%{version}.tar.gz

%description
Accounts test asset.
            
%package        generic-tests
Summary:        MIN test case scripts for mwts-accounts
Requires:       mwts-accounts
%description    generic-tests
MIN test case scripts for mwts-accounts

%package        generic-config
Summary:        Generic configuration file for mwts-accounts
Requires:       mwts-accounts-tests
%description    generic-config
Generic configuration file for mwts-accounts

%package	generic-all
Summary:	meta package containing everything for mwts-accounts (generic)
Requires:	mwts-accounts, mwts-accounts-generic-tests, mwts-accounts-generic-config
%description	generic-all
Meta package for installing all needed packages for generic version of mwts-accounts

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
/usr/lib/libmwts-accounts*
/usr/lib/min/*.so*

%files generic-tests
/etc/min.d/mwts-accounts.min.conf 
/usr/share/mwts-accounts-tests/tests.xml
/usr/lib/min/*.cfg

%files generic-config
/usr/lib/tests/*.conf

%files generic-all
