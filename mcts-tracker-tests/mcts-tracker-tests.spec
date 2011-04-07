%define _unpackaged_files_terminate_build 0 

Summary: MeeGo 1.0 Netbook mcts-tracker-tests suite
Name: mcts-tracker-tests
Version: 1.0.0
Release: 2
License: GPLv2
Group: System/Libraries
Requires: tracker-utils
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
%post
cp -rf /opt/%name/data /tmp/
chmod -R g+w /tmp/data/*
