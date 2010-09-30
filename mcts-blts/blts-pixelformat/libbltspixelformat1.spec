Name: libbltspixelformat1
Summary: BLTS pixel format conversion library
Version: 0.1.3
Release: 1
License: GPLv2
Group: Development/Testing
Source0: blts-pixelformat-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

%package -n libbltspixelformat-devel
Summary: BLTS pixel format conversion library dev package
Group: Development/Libraries

%description
Pixel format conversion functions for the BLTS test assets. Based on ffmpeg libswscale.

%description -n libbltspixelformat-devel
This package contains libbltspixelformat1 development files

%prep
%setup -q -n blts-pixelformat-%{version}

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
%doc README COPYING
/usr/lib/*.so.*

%files -n libbltspixelformat-devel
%doc README COPYING
/usr/lib/*.so
/usr/include/*.h
/usr/lib/pkgconfig/*.pc
