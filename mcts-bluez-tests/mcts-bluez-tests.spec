%define _unpackaged_files_terminate_build 0 

Summary: Template meego core test suite
Name: mcts-bluez-tests
Version: 2.0.0
Release: 2
License: GPLv2
Group: System/Libraries
Source: %name-%version.tar.gz
Requires: pygobject2  
#BuildRoot: %_topdir/%name-%version-buildroot

%description

This is meego test suite package for common components


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
rm -rf $RPM_BUILD_ROOT

%files
/opt/%name
/usr/share/%name

%changelog

%post
cd /opt/mcts-bluez-tests/data && chmod +x *

