%define _unpackaged_files_terminate_build 0 

Summary: Template meego core test suite
Name: mcts-pulseaudio-tests
Version: 1.0.0
Release: 2
License: GPLv2
Group: System/Libraries
Source: %name-%version.tar.gz
BuildRequires: pulseaudio-devel
#BuildRoot: %_topdir/%name-%version-buildroot

%description

This is meego test suite package for common components


%prep
%setup -q

%build
unset LD_AS_NEEDED   
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
cd /opt/%name/ && chmod +x *
