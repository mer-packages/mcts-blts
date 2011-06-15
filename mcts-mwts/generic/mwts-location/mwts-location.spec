# spec file for mwts-location

%define release         0
%define buildroot       %{_topdir}/%{name}-%{version}-root

BuildRoot:              %{buildroot}
Summary:                Mwts-location is a test asset for GPS using Qt Mobility location API
License:                LGPL
Name:                   mwts-location
Version:                0.0.8
Release:                %{release}
Prefix:                 /usr
Group:                  Development/Tools
BuildRequires:          qt-devel, min-devel, min, mwts-common-devel, qt-mobility-devel
Requires:               mwts-common, libqtlocation1, min
Source:                 %{name}-%{version}.tar.gz

%description
Mwts-location-tests is a test asset for GPS using Qt Mobility location API


%package               generic-tests
Summary:               Min interface and test cases
Prefix:                 /usr
Group:                 Development/Tools
Requires:              mwts-location
%description           generic-tests
MIN test cases for mwts-location-tests

%package               generic-config
Summary:               Configuration file for mwts-location / generic
Prefix:                 /usr
Group:                 Development/Tools
Requires:              mwts-location
%description           generic-config
Configuration file for mwts-location / generic

%package               generic-all
Summary:               Meta package for all needed mwts-location generic packages
Prefix:                 /usr
Group:                 Development/Tools
Requires:              mwts-location, mwts-location-generic-config, mwts-location-generic-tests
%description           generic-all
Meta package for all needed mwts-location generic packages



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
/usr/lib/min/*.so


%files generic-tests
/etc/min.d/*.min.conf
/usr/lib/min/*.cfg
/usr/share/mwts-location-scripts/tests.xml

%files generic-config
/usr/lib/tests/LocationTest.conf

%files generic-all

%post
ldconfig

%postun
ldconfig
