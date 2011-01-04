# spec file for mwts-gcamera

#%define release		0
%define buildroot	%{_topdir}/%{name}-%{version}-root

BuildRoot:	%{buildroot}
Summary: 	Test libraries for camera over gstreamer
License: 	LGPL
Name: 		mwts-gcamera
Version: 	0.0.1
Release: 	%{release}
Prefix: 	/usr
Group: 		Development/Tools
BuildRequires:	qt-devel, min-devel, mwts-common-devel, dbus-devel, dbus-glib-devel, gstreamer-devel, gst-plugins-bad-free-devel, gst-plugins-base-devel, gst-v4l2-camsrc-devel
Requires:	mwts-common, glib2, dbus, dbus-glib, gstreamer, gst-plugins-bad-free, gst-plugins-base, gst-v4l2-camsrc
Source: 	%{name}-%{version}.tar.gz
%description
Test cases for camera over gstreamer

%package	tests-generic
Summary:	Test cases for camera over gstreamer
Prefix: 	/usr
Group: 		Development/Tools
Requires:	mwts-gcamera-tests
%description	tests-generic
MIN test cases for measuring performance and reliability

%package        config-generic
Summary:       	Generic configuration file for mwts-gcamera
Requires:       mwts-gcamera-tests
%description    config-generic
Generic configuration file for mwts-gcamera

%package	all-generic
Summary:	meta package containing everything for mwts-gcamera (generic)
Requires:	mwts-gcamera, mwts-gcamera-tests-generic, mwts-gcamera-config-generic
%description	all-generic
Meta package for installing all needed packages for generic version of mwts-gcamera

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

%files tests-generic
/etc/min.d/mwts-gcamera.min.conf
/usr/share/mwts-gcamera-tests/tests.xml
/usr/lib/min/*.cfg

%files config-generic
/usr/lib/tests/*

%files all-generic

%changelog
* Tue Nov 2 2010 Balazs Sipos <balazs.sipos@digia.com> 0.0.1
- Intial version

