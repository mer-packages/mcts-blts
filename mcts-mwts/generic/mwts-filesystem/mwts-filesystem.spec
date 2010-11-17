# spec file for mwts-filesystem

%define release         0
%define buildroot       %{_topdir}/%{name}-%{version}-root

BuildRoot:              %{buildroot}
Summary:                mwts-filesystem is generic File system test asset API libraries and generic configuration
License:                LGPL
Name:                   %{name}
Version:                %{version}
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

%package                scripts
Summary:                mwts-filesystem MIN files
Prefix:                 /usr
Group:                  Development/Tools
Requires:               min, mwts-filesystem
%description            scripts
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
/usr/lib/libmwts-filesystem.*
/usr/lib/tests/FilesystemTest.conf

%files devel
%doc README
%doc COPYING
%doc DEPENDENCIES.png
%doc doc/MWTS.README
/usr/include/FilesystemTest.h

%files scripts
%doc README
%doc COPYING
%doc DEPENDENCIES.png
%doc doc/MWTS.README
/etc/min.d/*.min.conf
/usr/lib/min/*.cfg
/usr/lib/min/libmin-mwts-filesystem.*
/usr/share/mwts-filesystem-scripts/tests.xml

%files cli
%doc README
%doc COPYING
%doc DEPENDENCIES.png
%doc doc/MWTS.README
/usr/bin/mwts-filesystem-cli


%postun
ldconfig
