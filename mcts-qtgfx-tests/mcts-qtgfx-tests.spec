%define _unpackaged_files_terminate_build 0 

Summary: MeeGo 1.0 Netbook mnts-qtgfx-tests suite
Name: mnts-qtgfx-tests
Version: 1.0.0
Release: 2
License: GPLv2
Group: System/Libraries
Source: %name-%version.tar.gz

%description
This is meego test suite package for Netbook

%prep
%setup -q

%build
./autogen
./configure --prefix=/usr
make

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf "$RPM_BUILD_ROOT" 

%files
/opt/%name
/usr/share/%name

%changelog
