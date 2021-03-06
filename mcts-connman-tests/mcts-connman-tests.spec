%define _unpackaged_files_terminate_build 0 

Summary: Template meego core test suite
Name: mcts-connman-tests
Version: 1.0.4
Release: 1
License: GPLv2
Group: System/Libraries
Source: %name-%version.tar.gz
BuildRequires: python, libqt-devel, qt-mobility-devel, gcc-c++
Requires: connman
Requires: connman-test
Requires: pygobject2, bind-utils
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
%defattr(-,root,root,-)
/opt/%name
/usr/share/%name

%changelog

