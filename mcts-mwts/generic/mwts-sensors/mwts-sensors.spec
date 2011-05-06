# spec file for mwts-sensors

%define release         0
%define buildroot	%{_topdir}/%{name}-%{version}-%{release}-root

BuildRoot:              %{buildroot}
Summary:                A Qt based framework library used by all MWTS test assets
License:                LGPL
Name:                   mwts-sensors
Version:                1.0.7
Release:                0
Prefix:                 /usr
Group:                  Development/Tools
BuildRequires:          qt-devel, min-devel, min, mwts-common-devel, qt-mobility-devel
Requires:               qt-x11, min
Source:                 %{name}-%{version}.tar.gz

%description
mwts-sensors is a common library for MWTS test assets. It provides tool for sensors testing, logging and reporting results.

%package                generic-tests
Summary:                mwts-sensors MIN files
Prefix:                 /usr
Group:                  Development/Tools
Requires:               min, mwts-sensors
%description            generic-tests
MIN test cases for mwts-sensors


%package                generic-config
Summary:                mwts-sensors generic config
Prefix:                 /usr
Group:                  Development/Tools
Requires:               mwts-sensors
%description            generic-config
mwts-sensors generic config


%package                generic-all
Summary:                mwts-sensors generic meta package
Prefix:                 /usr
Group:                  Development/Tools
Requires:               mwts-sensors, mwts-sensors-generic-all, mwts-sensors-generic-config
%description            generic-all
mwts-sensors generic meta package


%package                cli
Summary:                mwts-sensors command line tool
Prefix:                 /usr/bin
Group:                  Development/Tools
Requires:               mwts-sensors
%description            cli
mwts-sensors command line tool

%package                test
Summary:                mwts-sensors command line test tool without min or mwts-common dependency
Prefix:                 /usr/bin
Group:                  Development/Tools
Requires:
%description            test
mwts-sensors command line test tool without min or mwts-common dependency

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
/usr/lib/*.so*
/usr/lib/min/*.so

%files generic-tests
/etc/min.d/*.min.conf
/usr/lib/min/*.cfg
/usr/share/mwts-sensors-tests/tests.xml

%files cli
/usr/bin/mwts-sensors-cli

%files test
/usr/bin/mwts-sensors-test

%files generic-config
/usr/lib/tests/*

%files generic-all
%doc README

%post
ldconfig

%postun
ldconfig
