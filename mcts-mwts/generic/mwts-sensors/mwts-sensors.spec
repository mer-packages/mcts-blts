# spec file for mwts-sensors

%define release         0
%define buildroot	%{_topdir}/%{name}-%{version}-%{release}-root

BuildRoot:              %{buildroot}
Summary:                A Qt based framework library used by all MWTS test assets
License:                LGPL
Name:                   mwts-sensors
Version:                1.0.1
Release:                0
Prefix:                 /usr
Group:                  Development/Tools
BuildRequires:          qt-devel, min-devel, min, mwts-common-devel, qt-mobility-devel
Requires:               qt-x11, min
Source:                 %{name}-%{version}.tar.gz

%description
mwts-sensors is a common library for MWTS test assets. It provides tool for sensors testing, logging and reporting results.

%package                tests
Summary:                mwts-sensors MIN files
Prefix:                 /usr
Group:                  Development/Tools
Requires:               min, mwts-sensors-tests
%description            tests
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

%files tests
%doc README
%doc COPYING
%doc DEPENDENCIES.png
%doc doc/MWTS.README
/etc/min.d/*.min.conf
/usr/lib/min/*.cfg
/usr/lib/min/libmin-mwts-sensors.so
/usr/share/mwts-sensors-tests/tests.xml

%files cli
%doc README
%doc COPYING
%doc DEPENDENCIES.png
%doc doc/MWTS.README
/usr/bin/mwts-sensors-cli


%post
mkdir -p /var/log/tests
chmod 777 /var/log/tests
ldconfig

%postun
ldconfig
