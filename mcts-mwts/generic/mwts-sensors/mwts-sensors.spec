# spec file for mwts-sensors

%define release         0
%define buildroot       %{_topdir}/%{name}-%{version}-root

BuildRoot:              %{buildroot}
Summary:                mwts-sensors is a Qt based framework library used by all MWTS test assets
License:                LGPL
Name:                   mwts-sensors-tests
Version:                1.0.0
Release:                0
Prefix:                 /usr
Group:                  Development/Tools
BuildRequires:          qt-devel, min-devel, min, mwts-common-devel, qt-mobility-devel
Requires:               qt-x11, min
Source:                 %{name}-%{version}.tar.gz

%description
mwts-sensors is a common library for MWTS test assets. It provides tool for sensors testing, logging and reporting results.

%package                devel
Summary:                mwts-sensors development files
Prefix:                 /usr
Group:                  Development/Tools
Requires:               qt-devel, mwts-sensors-tests
%description            devel
Development headers and libraries for mwts-sensors

%package                scripts
Summary:                mwts-sensors MIN files
Prefix:                 /usr
Group:                  Development/Tools
Requires:               min, mwts-sensors-tests
%description            scripts
MIN test cases for mwts-sensors

%package                cli
Summary:                mwts-sensors command line tool
Prefix:                 /usr/bin
Group:                  Development/Tools
Requires:               mwts-sensors
%description            cli
mwts-sensors command line tool

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
/usr/lib/libmwts-sensors.*

%files devel
%doc README
%doc COPYING
%doc DEPENDENCIES.png
%doc doc/MWTS.README
/usr/include/SensorsTest.h

%files scripts
%doc README
%doc COPYING
%doc DEPENDENCIES.png
%doc doc/MWTS.README
/etc/min.d/*.min.conf
/usr/lib/min/*.cfg
/usr/lib/min/libmin-mwts-sensors.*
/usr/share/mwts-sensors-scripts/tests.xml


%files cli
%doc README
%doc COPYING
%doc DEPENDENCIES.png
%doc doc/MWTS.README
/usr/bin/mwts-sensors-cli


%post
mkdir -p /var/log/tests
chmod 777 /var/log/tests

%postun
ldconfig
