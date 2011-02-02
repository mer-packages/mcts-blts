# spec file for mwts-template

%define buildroot	%{_tmppath}/%{name}-%{version}-%{release}-root  

BuildRoot:      %{buildroot}
Name:           mwts-template
Summary:        Template test asset
License:        LGPL
Version:        0.0.1
Release:        0
Prefix:         /usr
Group:          Development/Tools
BuildRequires:  qt-devel, min-devel, min, mwts-common-devel
Requires:       mwts-common
Source:         %{name}-%{version}.tar.gz

%description
Template test asset.

%package        generic-tests
Summary:        MIN test case scripts for mwts-template
Requires:       mwts-template
%description    generic-tests
MIN test case scripts for mwts-template

%package        generic-config
Summary:        Generic configuration file for mwts-template
Requires:       mwts-template
%description    generic-config
Generic configuration file for mwts-template

%package	generic-all
Summary:	Meta package containing everything for mwts-template (generic)
Requires:	mwts-template, mwts-template-generic-tests, mwts-template-generic-config
%description	generic-all
Meta package for installing all needed packages for generic version of mwts-template

%prep
%setup -q

%build
qmake
make

%install
make install INSTALL_ROOT=%{buildroot}

%files 
%defattr (-,root,root)
%doc README
%doc doc/MWTS.README
/usr/lib/libmwts-template.so*
/usr/lib/min/*.so

%files generic-tests
%defattr (-,root,root)
/usr/share/mwts-template-generic-tests/tests.xml
/usr/lib/min/*.cfg
/etc/min.d/*.min.conf

%files generic-config
%defattr (-,root,root)
/usr/lib/tests/*

%files generic-all
%defattr (-,root,root)
%doc README

%post  
ldconfig
   
%postun  
ldconfig  
