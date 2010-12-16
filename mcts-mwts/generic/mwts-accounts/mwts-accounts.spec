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
BuildRequires:  qt-devel, min-devel, min, mwts-common-devel, libsignon-devel libaccounts-qt-devel
Requires:       mwts-common
Source:         %{name}-%{version}.tar.gz

%description
Accounts test asset.
            
%package        scripts
Summary:        MIN test case scripts for mwts-accounts
Requires:       mwts-accounts
%description    scripts
MIN test case scripts for mwts-accounts


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
/usr/lib/tests/*
/etc/min.d/mwts-accounts.min.conf 


%files scripts
%doc README
%doc doc/MWTS.README
#/etc/min.d/*.min.conf
/usr/share/mwts-accounts-scripts/tests.xml
/usr/lib/min/*.cfg

