# spec file for mwts-multimedia

%define buildroot	%{_topdir}/%{name}-%{version}-root

BuildRoot:      %{buildroot}
Name:           mwts-multimedia
Summary:        Test asset for Qt Mobility Multimedia API
License:        LGPL
Version:        0.1.1
Release:        0
Prefix:         /usr
Group:          Development/Tools
BuildRequires:  qt-devel, min-devel, min, mwts-common-devel, qt-mobility-devel, libqtopengl-devel
Requires:       mwts-common, qt-mobility, libqtmultimediakit1
Source:         %{name}-%{version}.tar.gz

%description
Test asset for Qt Mobility Multimedia API

%package        generic-tests
Summary:        MIN test case scripts for mwts-multimedia
Requires:       mwts-multimedia
%description    generic-tests
MIN test case scripts for mwts-multimedia

%package        generic-config
Summary:        Generic configuration file for mwts-multimedia
Requires:       mwts-multimedia
%description    generic-config
Generic configuration file for mwts-multimedia

%package	generic-all
Summary:	meta package containing everything for mwts-multimedia (generic)
Requires:	mwts-multimedia, mwts-multimedia-generic-tests, mwts-multimedia-generic-config
%description	generic-all
Meta package for installing all needed packages for generic version of mwts-multimedia


%prep
%setup -q -n %{name}-%{version}

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
/usr/lib/*.so*
/usr/lib/min/*.so

%files generic-tests
%defattr (-,root,root)
/etc/min.d/*.min.conf
/usr/share/mwts-multimedia-tests/tests.xml
/usr/lib/min/*.cfg

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

%changelog
* Fri Jan 14 2011 Jan Grela <jan.grela@digia.com> - 0.1.1
 - Added bug fix for FM radio test cases, UnInitialize fails fixed (QRadioTuner pointer issue), minor changes to codecs and container setting
* Thu Dec 09 2010 Jan Grela <jan.grela@digia.com> - 0.1.0
- added playback functionality, codecs and containters setting moved to .conf file, added new test cases
* Tue Sep 29 2010 Balazs Sipos <balazs.sipos@digia.com> - 0.0.1
- Initial version
