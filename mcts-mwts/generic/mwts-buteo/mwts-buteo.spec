# spec file for mwts-buteo

%define buildroot	%{_topdir}/%{name}-%{version}-root

BuildRoot:      %{buildroot}
Name:           mwts-buteo
Summary:        Mwts-buteo syncing test asset 
License:        LGPL
<<<<<<< HEAD
Version:        0.0.4
=======
Version:        0.0.2
>>>>>>> 308e180f60e37c8ba0688d1b18807b55ff28c9f8
Release:        0
Prefix:         /usr
Group:          Development/Tools
BuildRequires:  qt-devel, min-devel, min, mwts-common-devel, buteo-syncfw-devel
Requires:       mwts-common
Source:         %{name}-%{version}.tar.gz

%description
mwts-buteo syncing test asset 

%package        generic-tests
Summary:        mwts-buteo MIN files
Prefix:		/usr
Requires:       mwts-buteo
%description    generic-tests
MIN test cases for mwts-buteo

%package        generic-config
Summary:        mwts-buteo generic configuration
Prefix:		/usr
Requires:       mwts-buteo
%description    generic-config
mwts-buteo generic configuration

%package        generic-all
Summary:        mwts-buteo generic meta package
Prefix:		/usr
<<<<<<< HEAD
Requires:       mwts-buteo, mwts-buteo-generic-config, mwts-buteo-generic-tests, buteo-service-google, buteo-service-memotoo, buteo-service-mobical
=======
Requires:       mwts-buteo, mwts-buteo-generic-config, mwts-buteo-generic-tests
>>>>>>> 308e180f60e37c8ba0688d1b18807b55ff28c9f8
%description    generic-all
mwts-buteo generic meta package


%prep
%setup -q

%build
qmake "CONFIG+=plugin"
make

%install
make install INSTALL_ROOT=%{buildroot}

%files 
%doc README
%doc doc/MWTS.README
/usr/lib/*.so*
/usr/lib/min/*.so

%files generic-tests
/etc/min.d/mwts-buteo.min.conf
/usr/share/mwts-buteo-tests/tests.xml
/usr/lib/min/*.cfg

%files generic-config
/usr/lib/tests/*

%files generic-all

%post
ldconfig

%postun
ldconfig

