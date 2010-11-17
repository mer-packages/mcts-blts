# spec file for mwts-template

%define buildroot	%{_topdir}/%{name}-%{version}-root

BuildRoot:      %{buildroot}
Name:           mwts-template
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
Summary:        MIN test case scripts for mwts-template
Requires:       mwts-template
%description    scripts-generic
MIN test case scripts for mwts-template

%package        config-generic
Summary:        Generic configuration file for mwts-template
Requires:       mwts-template
%description    config-generic
Generic configuration file for mwts-template

%package	all-generic
Summary:	meta package containing everything for mwts-template (generic)
Requires:	mwts-template, mwts-template-scripts-generic, mwts-template-config-generic
%description	all-generic
Meta package for installing all needed packages for generic version of mwts-template


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
/usr/lib/libmwts-template*
/usr/lib/min/*.so*


%files scripts-generic
/etc/min.d/*.min.conf
/usr/share/mwts-template-scripts/tests.xml
/usr/lib/min/*.cfg

%files config-generic
/usr/lib/tests/*

%files all-generic
