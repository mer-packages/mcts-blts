# spec file for mwts-multimedia

%define buildroot	%{_topdir}/%{name}-%{version}-root

BuildRoot:      %{buildroot}
Name:           mwts-multimedia
Summary:        Test asset for Qt Mobility Multimedia API
License:        LGPL
Version:        0.1.0
Release:        0
Prefix:         /usr
Group:          Development/Tools
BuildRequires:  qt-devel, min-devel, min, mwts-common-devel, qt-mobility-devel, libqtopengl-devel
Requires:       mwts-common, qt-mobility, libqtmultimediakit1
Source:         %{name}-%{version}.tar.gz

%description
Test asset for Qt Mobility Multimedia API

%package                devel
Summary:                mwts-multimedia development files
Group:                  Development/Tools
Requires:               qt-devel, mwts-multimedia
%description            devel
Development headers and libraries for mwts-multimedia
            
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
Requires:	mwts-multimedia, mwts-multimedia-devel, mwts-multimedia-scripts-generic, mwts-multimedia-config-generic
%description	all-generic
Meta package for installing all needed packages for generic version of mwts-multimedia


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
%doc COPYING
%doc DEPENDENCIES.png
%doc doc/MWTS.README
/usr/lib/libmwts-multimedia.so.*
/usr/lib/min/libmin-mwts-multimedia.so.*

%files devel
%defattr (-,root,root)
%doc README
%doc COPYING
%doc DEPENDENCIES.png
%doc doc/MWTS.README
/usr/lib/libmwts-multimedia.so
/usr/lib/min/libmin-mwts-multimedia.so
/usr/include/MultimediaTest.h

%files scripts-generic
%defattr (-,root,root)
/etc/min.d/*.min.conf
/usr/share/mwts-multimedia-tests/tests.xml
/usr/lib/min/*.cfg

%files config-generic
%defattr (-,root,root)
/usr/lib/tests/*

%files all-generic
%defattr (-,root,root)
%doc README

%post
ldconfig

%postun
ldconfig

%changelog
* Thu Dec 09 2010 Jan Grela <jan.grela@digia.com> - 0.1.0
- added playback functionality, codecs and containters setting moved to .conf file, added new test cases
* Tue Sep 29 2010 Balazs Sipos <balazs.sipos@digia.com> 0.0.1
- Initial version
