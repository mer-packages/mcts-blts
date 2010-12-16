# spec file for mwts-location

%define release         0
%define buildroot       %{_topdir}/%{name}-%{version}-root

BuildRoot:              %{buildroot}
Summary:                Mwts-location is a test asset for GPS using Qt Mobility location API
License:                LGPL
Name:                   mwts-location-tests
Version:                0.0.3
Release:                %{release}
Prefix:                 /usr
Group:                  Development/Tools
BuildRequires:          qt-devel, min-devel, min, mwts-common-devel, qt-mobility-devel
Requires:               mwts-common, libqtlocation1, min
Source:                 %{name}-%{version}.tar.gz

%description
Mwts-location-tests is a test asset for GPS using Qt Mobility location API


%package               scripts
Summary:               Min interface and test cases
Prefix:                 /usr
Group:                 Development/Tools
Requires:              mwts-location-tests
%description           scripts
MIN test cases for mwts-location-tests

%prep
%setup -q -n %{name}-%{version}

%build
qmake
make

%install
make install INSTALL_ROOT=%{buildroot}

%files
%doc README
%doc COPYING
%doc DEPENDENCIES.png
%doc doc/MWTS.README
/usr/lib/libmwts-location.*
/usr/lib/tests/LocationTest.conf


%files scripts
%doc README
%doc COPYING
%doc DEPENDENCIES.png
%doc doc/MWTS.README
/etc/min.d/*.min.conf
/usr/lib/min/*.cfg
/usr/lib/min/libmin-mwts-location.*
/usr/share/mwts-location-scripts/tests.xml


%postun
ldconfig
