Name: libbltspixelformat1
Summary: BLTS pixel format conversion library
Version: 0.1.4
Release: 1
License: GPLv2
Group: Development/Testing
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: libbltscommon-devel

%package devel
Summary: BLTS pixel format conversion library dev package
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}
Provides: libbltspixelformat-devel

%description
Pixel format conversion functions for the BLTS test assets. Based on ffmpeg libswscale.

%description devel
This package contains libbltspixelformat1 development files

%prep
%setup -q -n %{name}-%{version}

%build
./autogen.sh
%configure
make

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
rm $RPM_BUILD_ROOT/usr/lib/*.la
rm $RPM_BUILD_ROOT/usr/lib/*.a

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%doc README COPYING
/usr/lib/*.so.*

%files devel
%defattr(-,root,root,-)
%doc README COPYING
/usr/lib/*.so
/usr/include/*.h
/usr/lib/pkgconfig/*.pc
