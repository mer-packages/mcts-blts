# spec file for mwts-filesystem

%define release         0
%define buildroot       %{_tmppath}/%{name}-%{version}-%{release}-root

BuildRoot:              %{buildroot}
Summary:                Generic File system test asset
License:                LGPL
Name:                   mwts-filesystem
Version:                1.0.1
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
Requires:               qt-devel, mwts-filesystem-tests
%description            devel
Development headers and libraries for mwts-filesystem

%package                tests
Summary:                mwts-filesystem MIN files
Prefix:                 /usr
Group:                  Development/Tools
Requires:               min, mwts-filesystem-tests
%description            tests
MIN test cases for mwts-filesystem

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
/usr/lib/tests/FilesystemTest.conf

%files devel
%doc README
%doc COPYING
%doc DEPENDENCIES.png
%doc doc/MWTS.README
/usr/lib/libmwts-filesystem.so
/usr/lib/min/libmin-mwts-filesystem.so
/usr/include/FilesystemTest.h

%files tests
%doc README
%doc COPYING
%doc DEPENDENCIES.png
%doc doc/MWTS.README
/etc/min.d/*.min.conf
/usr/lib/min/*.cfg
/usr/lib/min/libmin-mwts-filesystem.so.*
/usr/share/mwts-filesystem-scripts/tests.xml

%files cli
%doc README
%doc COPYING
%doc DEPENDENCIES.png
%doc doc/MWTS.README
/usr/bin/mwts-filesystem-cli

%post
ldconfig

%postun
ldconfig
