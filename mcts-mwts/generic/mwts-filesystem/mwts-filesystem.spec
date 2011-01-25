# spec file for mwts-filesystem

%define release         0
%define buildroot       %{_tmppath}/%{name}-%{version}-%{release}-root

BuildRoot:              %{buildroot}
Summary:                Generic File system test asset
License:                LGPL
Name:                   mwts-filesystem
Version:                1.0.2
Release:                %{release}
Prefix:                 /usr
Group:                  Development/Tools
BuildRequires:          qt-devel, min-devel, min, mwts-common-devel
Requires:               qt-x11, min
Source:                 %{name}-%{version}.tar.gz

%description
mwts-filesystem is a common library for MWTS test assets. It provides tool for filesystems testing, logging and reporting results.

%package                devel
Summary:                mwts-filesystem development files
Prefix:                 /usr
Group:                  Development/Tools
Requires:               qt-devel, mwts-filesystem
%description            devel
Development headers and libraries for mwts-filesystem

%package                generic-tests
Summary:                mwts-filesystem MIN files
Prefix:                 /usr
Group:                  Development/Tools
Requires:               min, mwts-filesystem
%description            generic-tests
MIN test cases for mwts-filesystem

%package                generic-config
Summary:                mwts-filesystem generic config file
Prefix:                 /usr
Group:                  Development/Tools
Requires:               mwts-filesystem
%description            generic-config
mwts-filesystem generic config file

%package                generic-utils
Summary:                mwts-filesystem generic utils
Prefix:                 /usr
Group:                  Development/Tools
Requires:               mwts-filesystem
%description            generic-utils
mwts-filesystem generic utils

%package                generic-all
Summary:                mwts-filesystem meta package for generic version
Prefix:                 /usr
Group:                  Development/Tools
Requires:               min, mwts-filesystem, mwts-filesystem-generic-config, mwts-filesystem-generic-tests
%description            generic-all
mwts-filesystem meta package for generic version


%package                cli
Summary:                mwts-filesystem command line tool
Prefix:                 /usr/bin
Group:                  Development/Tools
Requires:               mwts-filesystem
%description            cli
mwts-filesystem command line tool

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
/usr/lib/libmwts-filesystem.so.*
/usr/lib/min/libmin-mwts-filesystem*

%files devel
/usr/lib/libmwts-filesystem.so
/usr/lib/min/libmin-mwts-filesystem.so
/usr/include/FilesystemTest.h

%files generic-tests
/etc/min.d/*.min.conf
/usr/lib/min/*.cfg
/usr/share/mwts-filesystem-tests/tests.xml

%files generic-config
/usr/lib/tests/FilesystemTest.conf

%files generic-utils
/usr/bin/*.sh

%files generic-all

%files cli
/usr/bin/mwts-filesystem-cli

%post
ldconfig

%postun
ldconfig
