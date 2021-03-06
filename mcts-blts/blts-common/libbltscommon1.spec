Name: libbltscommon1
Summary: Common BLTS functions
Version: 0.4.5
Release: 1
License: GPLv2
Group: Development/Testing
URL: https://github.com/mer-packages/mcts-blts
Source0: %{name}-%{version}.tar.gz
BuildRequires: flex
BuildRequires: bison
BuildRequires: libxml2-devel

%package devel
Summary: Common BLTS functions devel package
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}
Provides: libbltscommon-devel

%description
Common functions used in BLTS project test assets gathered in one library

%description devel
This package contains libbltscommon1 development files

%prep
%setup -q -n %{name}-%{version}

%build
./autogen.sh
%configure
make

%install
make install DESTDIR=$RPM_BUILD_ROOT
rm $RPM_BUILD_ROOT/usr/lib/*.la
rm $RPM_BUILD_ROOT/usr/lib/*.a
mkdir -p $RPM_BUILD_ROOT/var/log/tests/blts/

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%attr(1777,root,root) /var/log/tests/blts
%doc README README.ParameterVariation COPYING
/usr/lib/*.so.*

%files devel
%defattr(-,root,root,-)
%doc README README.ParameterVariation COPYING
/usr/lib/*.so
/usr/include/*.h
/usr/lib/pkgconfig/*.pc

