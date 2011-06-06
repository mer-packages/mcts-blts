# spec file for mwts-gstreamer

#%define release		0
%define buildroot	%{_topdir}/%{name}-%{version}-%{release}-root

BuildRoot:	%{buildroot}
Summary: 	Test libraries for GStreamer.
License: 	LGPL
Name: 		mwts-gstreamer
Version: 	0.0.9
Release: 	%{release}
Prefix: 	/usr
Group: 		Development/Tools
BuildRequires:	qt-devel, min-devel, mwts-common-devel, glib2, dbus-devel, dbus-glib-devel, gstreamer-devel
Requires:	mwts-common, glib2, dbus, dbus-glib, gstreamer
Source: 	%{name}-%{version}.tar.gz
%description
Test libraries for GStreamer.

%package	generic-scripts
Summary:	Test cases for GStreamer.
Prefix: 	/usr
Group: 		Development/Tools
Requires:	mwts-gstreamer
%description	generic-scripts
MIN test cases for measuring performance and reliability

%package        generic-config
Summary:       	Generic configuration file for mwts-gstreamer
Requires:       mwts-gstreamer
%description    generic-config
Generic configuration file for mwts-gstreamer

%package	generic-all
Summary:	meta package containing everything for mwts-gstreamer (generic)
Requires:	mwts-gstreamer, mwts-gstreamer-generic-scripts, mwts-gstreamer-generic-config
%description	generic-all
Meta package for installing all needed packages for generic version of mwts-gstreamer

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
/usr/lib/*.so*
/usr/lib/min/*.so
/usr/lib/tests/*

%files generic-scripts
/etc/min.d/mwts-gstreamer.min.conf
/usr/lib/min/*.cfg

%files generic-config
/usr/lib/tests/*

%files generic-all

%post
ldconfig

%postun
ldconfig

%changelog
* Thu Dec 09 2010 Jan Grela <jan.grela@digia.com> - 0.0.7
- Added playback functionality, codecs and containers setting moved to .conf file, added new test cases
* Tue Sep 29 2010 Balazs Sipos <balazs.sipos@digia.com> - 0.0.6
- Initial MeeGo version
* Wed Sep 22 2010 Esa-Pekka Miettinen <esa-pekka.miettinen@digia.com> - 0.0.5
- MeeGo Audio and Video test cases added
* Wed Aug 11 2010 Esa-Pekka Miettinen <esa-pekka.miettinen@digia.com> - 0.0.4
- FPS performance measurement functionality (max FPS)
- Changed timeout to use mwts-gstreamer timeout, not mwts-common
* Tue Jun 15 2010 Esa-Pekka Miettinen <esa-pekka.miettinen@digia.com> - 0.0.3
- Added ART test cases
- Fixed scripts, removed useless spaces and URI away
- Fixed alltests.xml and tests.xml
* Wed May 19 2010 Esa-Pekka Miettinen <esa-pekka.miettinen@digia.com> - 0.0.2
- Fixed scripts
* Tue May 11 2010 Esa-Pekka Miettinen <esa-pekka.miettinen@digia.com> - 0.0.1
- Initial version
