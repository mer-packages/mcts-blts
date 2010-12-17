# spec file for mwts-gstreamer

#%define release		0
%define buildroot	%{_topdir}/%{name}-%{version}-root

BuildRoot:	%{buildroot}
Summary: 	Test libraries for GStreamer.
License: 	LGPL
Name: 		mwts-gstreamer-tests
Version: 	0.0.5
Release: 	%{release}
Prefix: 	/usr
Group: 		Development/Tools
BuildRequires:	qt-devel, min-devel, mwts-common-devel, glib2, dbus-devel, dbus-glib-devel, gstreamer-devel
Requires:	mwts-common, glib2, dbus, dbus-glib, gstreamer
Source: 	%{name}-%{version}.tar.gz
%description
Test libraries for GStreamer.

%package	scripts-generic
Summary:	Test cases for GStreamer.
Prefix: 	/usr
Group: 		Development/Tools
Requires:	mwts-gstreamer-tests
%description	scripts-generic
MIN test cases for measuring performance and reliability

%package        config-generic
Summary:       	Generic configuration file for mwts-gstreamer
Requires:       mwts-gstreamer-tests
%description    config-generic
Generic configuration file for mwts-gstreamer

%package	all-generic
Summary:	meta package containing everything for mwts-gstreamer (generic)
Requires:	mwts-gstreamer-tests, mwts-gstreamer-scripts-generic, mwts-gstreamer-config-generic
%description	all-generic
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
/usr/lib/min/*.so*
/usr/lib/tests/*

%files scripts-generic
/etc/min.d/mwts-gstreamer.min.conf
/usr/share/mwts-gstreamer-scripts/tests.xml
/usr/lib/min/*.cfg

%files config-generic
/usr/lib/tests/*

%files all-generic

%changelog
* Wed Sep 22 2010 Esa-Pekka Miettinen <esa-pekka.miettinen@digia.com> 0.0.5
- MeeGo Audio and Video test cases added
- MeeGo support

