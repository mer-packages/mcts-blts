# spec file for mwts-buteo

%define buildroot	%{_topdir}/%{name}-%{version}-root

BuildRoot:      %{buildroot}
Name:           mwts-buteo
Summary:        Mwts-buteo syncing test asset 
License:        LGPL
Version:        0.0.3
Release:        0
Prefix:         /usr
Group:          Development/Tools
BuildRequires:  qt-devel, min-devel, min, mwts-common-devel, buteo-syncfw-devel
Requires:       mwts-common
Source:         %{name}-%{version}.tar.gz

%description
mwts-buteo syncing test asset 
            
%package        devel
Summary:        mwts-buteo Development files for 
Prefix: 	/usr
Requires:       qt-devel, mwts-buteo-tests
%description    devel
Development headers and libraries for mwts-buteo

%package        tests
Summary:        mwts-buteo MIN files
Prefix:		/usr
Requires:       min, mwts-buteo-tests
%description    tests
MIN test cases for mwts-buteo

%prep
%setup -q

%build
qmake "CONFIG+=plugin"
make

%install
make install INSTALL_ROOT=%{buildroot}

%files devel
%doc README
%doc doc/MWTS.README
/usr/lib/*.so
/usr/lib/min/*.so

%files
%doc README
%doc doc/MWTS.README
%doc DEPENDENCIES.png
%doc COPYING
#/usr/lib/*.so.*
#/usr/lib/min/*.so.*
/usr/lib/tests/*

%files tests
%doc README
%doc doc/MWTS.README
/etc/min.d/mwts-buteo.min.conf
/usr/share/mwts-buteo-tests/tests.xml
/usr/lib/min/*.cfg
