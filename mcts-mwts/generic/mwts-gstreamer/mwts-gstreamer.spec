# spec file for mwts-gstreamer

#%define release		0
%define buildroot	%{_topdir}/%{name}-%{version}-%{release}-root

BuildRoot:	%{buildroot}
Summary: 	Test libraries for GStreamer.
License: 	LGPL
Name: 		mwts-gstreamer
Version: 	1.0.0
Release: 	%{release}
Prefix: 	/usr
Group: 		Development/Tools
BuildRequires:	qt-devel, min-devel, mwts-common-devel, glib2, dbus-devel, dbus-glib-devel, gstreamer-devel
Requires:	mwts-common, glib2, dbus, dbus-glib, gstreamer
Source: 	%{name}-%{version}.tar.gz
%description
Test libraries for GStreamer.

%package	generic-tests
Summary:	Test cases for GStreamer.
Prefix: 	/usr
Group: 		Development/Tools
Requires:	mwts-gstreamer
%description	generic-tests
MIN test cases for measuring performance and reliability

%package        generic-config
Summary:       	Generic configuration file for mwts-gstreamer
Requires:       mwts-gstreamer
%description    generic-config
Generic configuration file for mwts-gstreamer

%package	generic-all
Summary:	meta package containing everything for mwts-gstreamer (generic)
Requires:	mwts-gstreamer-tests, mwts-gstreamer-generic-tests, mwts-gstreamer-generic-config
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

%files generic-tests
/etc/min.d/mwts-gstreamer.min.conf
/usr/share/mwts-gstreamer-tests/tests.xml
/usr/lib/min/*.cfg

%files generic-config
/usr/lib/tests/*

%files generic-all

%post
ldconfig

%postun
ldconfig
