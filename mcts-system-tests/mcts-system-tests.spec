%define _unpackaged_files_terminate_build 0 

Summary: Template meego core test suite
Name: mcts-system-tests
Version: 1.0.0
Release: 1
License: GPLv2
Group: System/Libraries
Source: %name-%version.tar.gz
#Requires:
#BuildRequires:
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
* Mon Jan 18 2011 Wei, Zhang <wei.z.zhang@intel.com> 1.0.0-1
- Initial mcts-system-tests:
- + process memory check: memory for each process is less than m%
- + process cpu check: cpu for each process is less than p%
