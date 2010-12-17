# spec file for mwts-common

%define release         0
%define buildroot       %{_topdir}/%{name}-%{version}-root

BuildRoot:              %{buildroot}
Summary:                Mwts-common is a Qt based framework library used by all MWTS test assets
License:                LGPL
Name:                   mwts-common
Version:                1.0.2
Release:                %{release}
Prefix:                 /usr
Group:                  Development/Tools
BuildRequires:          qt-devel, min-devel, min, gcc-c++
Requires:               libqtcore4, libqtgui4, min
Source:                 %{name}-%{version}.tar.gz

%description
Mwts-common is a common library for MWTS test assets. It provides tool for memory and cpu monitoring, logging and reporting results.

%package                devel
Summary:                Mwts-common development files
Prefix:                 /usr
Group:                  Development/Tools
Requires:               qt-devel, mwts-common
%description            devel
Development headers and libraries for mwts-common

%package                tests
Summary:                Mwts-common MIN files
Prefix:                 /usr
Group:                  Development/Tools
Requires:               min, mwts-common
%description            tests
MIN test cases for mwts-common

%prep
%setup -q -n %{name}-%{version}

%build
qmake
make

%install
make install INSTALL_ROOT=%{buildroot}

%files
%doc README COPYING DEPENDENCIES.png
/usr/lib/libmwts-common.so.*
/usr/bin/*.py

%files devel
%doc README COPYING DEPENDENCIES.png
/usr/lib/libmwts-common.so
/usr/lib/min/libmin-mwts-common.so
/usr/include/MwtsCommon
/usr/include/mwts*.h

%files tests
%doc README COPYING DEPENDENCIES.png
/etc/min.d/*.min.conf
/usr/lib/min/*.cfg
/usr/lib/min/libmin-mwts-common.so.*
/usr/share/applications/*.desktop

%post
mkdir -p /var/log/tests
chmod 777 /var/log/tests

%postun
ldconfig
