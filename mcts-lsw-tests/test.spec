%define name mcts-lsw-tests
%define version 1.0.0
%define release 2
%define _unpackaged_files_terminate_build 0 


Summary: meego core test suite for libsocialweb component
Name: %{name}
Version: %{version}
Release: %{release}
License: GPLv2
Group: System/Libraries
Source: %{name}-%{version}.tar.gz
Requires:  dbus  
Requires:  libsocialweb
Requires:  dbus-python
Requires:  python-lxml  
Requires:  GConf-dbus  
BuildRoot: %{_topdir}/tmp/%{name}-%{version}-buildroot

%description

This is meego test suite package for libsocialweb component.   


%prep
%setup -q

%build
./autogen
%configure --prefix=/usr
make   

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
/opt/%name
/usr/share/%name

%post
cp /opt/%name/utils/uexec /usr/bin

%changelog
