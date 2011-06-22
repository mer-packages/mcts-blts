# spec file for mwts-qtmultimedia

%define buildroot	%{_topdir}/%{name}-%{version}-root

BuildRoot:      %{buildroot}
Name:           mwts-qtmultimedia
Summary:        Test asset for Qt Mobility Multimedia API
License:        LGPL
Version:        0.1.8
Release:        0
Prefix:         /usr
Group:          Development/Tools
BuildRequires:  qt-devel, min-devel, min, mwts-common-devel, qt-mobility-devel, libqtopengl-devel
Requires:       mwts-common, qt-mobility, libqtmultimediakit1
Source:         %{name}-%{version}.tar.gz

%description
Test asset for Qt Mobility Multimedia API

%package        generic-tests
Summary:        MIN test case scripts for mwts-qtmultimedia
Requires:       mwts-qtmultimedia
%description    generic-tests
MIN test case scripts for mwts-qtmultimedia

%package        generic-config
Summary:        Generic configuration file for mwts-qtmultimedia
Requires:       mwts-qtmultimedia
%description    generic-config
Generic configuration file for mwts-qtmultimedia

%package	generic-all
Summary:	meta package containing everything for mwts-qtmultimedia (generic)
Requires:	mwts-qtmultimedia, mwts-qtmultimedia-generic-tests, mwts-qtmultimedia-generic-config
%description	generic-all
Meta package for installing all needed packages for generic version of mwts-qtmultimedia


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
/usr/share/mwts-qtmultimedia-generic-tests/tests.xml
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
* Fri Apr 8 2011 Balazs Sipos <balazs.sipos@digia.com> - 0.1.4
- Fix: NB#13710 - Imageviewing case always show failed though image shown on screen
- Choosing feature to use in test case added
- Audio and video playback functionality (playing, pausing, stopping, seeking) added to MediaPlayer

* Fri Jan 14 2011 Jan Grela <jan.grela@digia.com> - 0.1.1
- Added bug fix for FM radio test cases, UnInitialize fails fixed (QRadioTuner pointer issue), minor changes to codecs and container setting
* Thu Dec 09 2010 Jan Grela <jan.grela@digia.com> - 0.1.0
- added playback functionality, codecs and containters setting moved to .conf file, added new test cases
* Tue Sep 29 2010 Balazs Sipos <balazs.sipos@digia.com> - 0.0.1
- Initial version
