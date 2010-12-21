# spec file for mwts-pim

%define release         0
%define buildroot       %{_tmppath}/%{name}-%{version}-%{release}-root

BuildRoot:              %{buildroot}
Summary:                Test asset for testing personal information managagement in qt mobility 
License:                LGPL
Name:                   mwts-pim
Version:                0.0.2
Release:                %{release}
Prefix:                 /usr
Group:                  Development/Tools
BuildRequires:          libqt-devel, qt-mobility-devel, min-devel, min, mwts-common-devel
Requires:               qt-x11, min
Source:                 %{name}-%{version}.tar.gz

%description
mwts-pim is a test asset for testing personal information managagement in qt mobility. It provides tool for pim testing, logging and reporting results.

%package                devel
Summary:                Development files for PIM
Prefix:                 /usr
Group:                  Development/Tools
Requires:               min, mwts-pim
%description            devel
Development files for PIM

%package                tests
Summary:                Test case files
Prefix:                 /usr
Group:                  Development/Tools
Requires:               min, mwts-pim
%description            tests
MIN test cases for mwts-pim

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
/usr/lib/*.so.*
/usr/lib/min/*.so.*
/usr/lib/tests/PimTest.conf
/usr/lib/tests/mwts-pim/*.vcf

%files devel
/usr/lib/*.so
/usr/lib/min/*.so


%files tests
%doc README
%doc COPYING
%doc DEPENDENCIES.png
%doc doc/MWTS.README
/etc/min.d/*.min.conf
/usr/lib/min/*.cfg
/user/share/mwts-pim-tests/tests.xml

%post
ldconfig

%postun
ldconfig
