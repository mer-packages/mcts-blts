# spec file for mwts-pim

%define release         0
%define buildroot       %{_tmppath}/%{name}-%{version}-%{release}-root

BuildRoot:              %{buildroot}
Summary:                Test asset for testing personal information managagement in qt mobility 
License:                LGPL
Name:                   mwts-pim
Version:                0.0.3
Release:                %{release}
Prefix:                 /usr
Group:                  Development/Tools
BuildRequires:          libqt-devel, qt-mobility-devel, min-devel, min, mwts-common-devel
Requires:               qt-x11, min
Source:                 %{name}-%{version}.tar.gz

%description
mwts-pim is a test asset for testing personal information managagement in qt mobility. It provides tool for pim testing, logging and reporting results.

%package                generic-scripts
Summary:                Test case files
Prefix:                 /usr
Group:                  Development/Tools
Requires:               min, mwts-pim
%description            generic-scripts
MIN test cases for mwts-pim

%package                generic-config
Summary:                mwts-pim generic config
Prefix:                 /usr
Group:                  Development/Tools
Requires:               mwts-pim
%description            generic-config
mwts-pim generic config

%package                generic-all
Summary:                mwts-pim generic meta package
Prefix:                 /usr
Group:                  Development/Tools
Requires:               mwts-pim, mwts-pim-generic-config, mwts-pim-generic-scripts
%description            generic-all
mwts-pim generic meta package

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
/usr/lib/tests/mwts-pim/*.vcf
/usr/lib/tests/mwts-pim/*.ics

%files generic-scripts
/etc/min.d/*.min.conf
/usr/lib/min/*.cfg

%files generic-config
/usr/lib/tests/PimTest.conf

%files generic-all

%post
ldconfig

%postun
ldconfig
