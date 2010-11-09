Name: libbltscommon1
Summary: Common BLTS functions
Version: 0.3.8
Release: 1
License: GPLv2
Group: Development/Testing
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: flex bison libxml2-devel
Requires: libxml2

%package -n libbltscommon-devel
Summary: Common BLTS functions devel package
Group: Development/Libraries

%description
Common functions used in BLTS project test assets gathered in one library

%description -n libbltscommon-devel
This package contains libbltscommon1 development files

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
%doc README README.ParameterVariation COPYING
/usr/lib/*.so.*

%files -n libbltscommon-devel
%doc README README.ParameterVariation COPYING
/usr/lib/*.so
/usr/include/*.h
/usr/lib/pkgconfig/*.pc
