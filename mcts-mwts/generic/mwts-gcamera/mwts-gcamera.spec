# spec file for mwts-gcamera

#%define release		0
%define buildroot	%{_topdir}/%{name}-%{version}-root

BuildRoot:	%{buildroot}
Summary: 	Test libraries for camera over gstreamer
License: 	LGPL
Name: 		mwts-gcamera
Version: 	0.0.2
Release: 	%{release}
Prefix: 	/usr
Group: 		Development/Tools
BuildRequires:	qt-devel, min-devel, mwts-common-devel, dbus-devel, dbus-glib-devel, gstreamer-devel, gst-plugins-bad-free-devel, gst-plugins-base-devel, gst-v4l2-camsrc-devel
Requires:	mwts-common, glib2, dbus, dbus-glib, gstreamer, gst-plugins-bad-free, gst-plugins-base, gst-v4l2-camsrc
Source: 	%{name}-%{version}.tar.gz
%description
Test cases for camera over gstreamer

%package	generic-scripts
Summary:	Test cases for camera over gstreamer
Prefix: 	/usr
Group: 		Development/Tools
Requires:	mwts-gcamera
%description	generic-scripts
MIN test cases for measuring performance and reliability

%package        generic-config
Summary:       	Generic configuration file for mwts-gcamera
Requires:       mwts-gcamera
%description    generic-config
Generic configuration file for mwts-gcamera

%package	generic-all
Summary:	meta package containing everything for mwts-gcamera (generic)
Requires:	mwts-gcamera, mwts-gcamera-generic-scripts, mwts-gcamera-generic-config
%description	generic-all
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
/usr/lib/min/*.so
/usr/lib/tests/*

%files generic-scripts
/etc/min.d/mwts-gcamera.min.conf
/usr/lib/min/*.cfg

%files generic-config
/usr/lib/tests/*

%files generic-all

%post
ldconfig

%postun
ldconfig

%changelog
* Tue Nov 02 2010 Balazs Sipos <balazs.sipos@digia.com> - 0.0.1
- Initial MeeGo version


