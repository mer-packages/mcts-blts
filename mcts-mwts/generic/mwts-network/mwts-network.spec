# spec file for mwts-network

%define buildroot	%{_tmppath}/%{name}-%{version}-%{release}-root  

BuildRoot:		%{buildroot}
Summary: 		A wlan/psd networking test asset
License: 		LGPL
Name: 			mwts-network
Version: 		0.0.7
Release: 		0
Prefix: 		/usr
Group: 			Testing
BuildRequires:		qt-devel, min-devel, mwts-common-devel
Requires:		qt-x11, min
Source: 		%{name}-%{version}.tar.gz

%description
Mwts-network is a wlan/psd networking test asset

%package                tests
Summary:                Min test-case scripts for mwts-network
Prefix:                 /usr
Group:                  Development/Scripts
Requires:               mwts-network
%description            tests
MIN test case scripts for mwts-network


%prep
%setup -q -n %{name}-%{version}

%build
qmake
make

%install
make install INSTALL_ROOT=%{buildroot}

%files
%doc README
%doc doc/MWTS.README
%doc DEPENDENCIES.png
%doc COPYING
/usr/lib/*.so*
/usr/lib/min/*.so
/usr/lib/tests/*

%files tests
%doc README
%doc doc/MWTS.README
/etc/min.d/mwts-network.min.conf
/usr/share/mwts-network-tests/tests.xml
/usr/lib/min/*.cfg

%post
ldconfig

%postun
ldconfig

