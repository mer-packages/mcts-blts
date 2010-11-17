# spec file for mwts-multimedia

%define buildroot	%{_topdir}/%{name}-%{version}-root

BuildRoot:      %{buildroot}
Name:           mwts-multimedia
Summary:        Test asset for Qt Mobility Multimedia API
License:        LGPL
Version:        %{version}
Release:        0
Prefix:         /usr
Group:          Development/Tools
BuildRequires:  qt-devel, min-devel, min, mwts-common-devel, qt-mobility-devel, libqtopengl-devel
Requires:       mwts-common, qt-mobility, libqtmultimediakit1
Source:         %{name}-%{version}.tar.gz

%description
Test asset for Qt Mobility Multimedia API
            
%package        scripts-generic
Summary:        MIN test case scripts for mwts-multimedia
Requires:       mwts-multimedia
%description    scripts-generic
MIN test case scripts for mwts-multimedia

%package        config-generic
Summary:        Generic configuration file for mwts-multimedia
Requires:       mwts-multimedia
%description    config-generic
Generic configuration file for mwts-multimedia

%package	all-generic
Summary:	meta package containing everything for mwts-multimedia (generic)
Requires:	mwts-multimedia, mwts-multimedia-scripts-generic, mwts-multimedia-config-generic
%description	all-generic
Meta package for installing all needed packages for generic version of mwts-multimedia


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
/usr/lib/libmwts-multimedia*
/usr/lib/min/*.so*


%files scripts-generic
/etc/min.d/*.min.conf
/usr/share/mwts-multimedia-tests/tests.xml
/usr/lib/min/*.cfg

%files config-generic
/usr/lib/tests/*

%files all-generic

%changelog
* Tue Sep 29 2010 Balazs Sipos <balazs.sipos@digia.com> 0.0.1
- Intial version


