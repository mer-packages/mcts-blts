# spec file for mwts-network

%define buildroot	%{_tmppath}/%{name}-%{version}-%{release}-root  

BuildRoot:		%{buildroot}
Summary: 		A wlan/psd networking test asset
License: 		LGPL
Name: 			mwts-network
Version: 		1.1.1
Release: 		0
Prefix: 		/usr
Group: 			Testing
BuildRequires:		qt-devel, min-devel, mwts-common-devel
Requires:		qt-x11, min
Source: 		%{name}-%{version}.tar.gz

%description
Mwts-network is a wlan/psd networking test asset

%package                generic-tests
Summary:                Min test-case scripts for mwts-network
Prefix:                 /usr
Group:                  Development/Scripts
Requires:               mwts-network
%description            generic-tests
MIN test case scripts for mwts-network


%package                generic-config
Summary:                mwts-network generic config
Prefix:                 /usr
Group:                  Development/Scripts
Requires:               mwts-network
%description            generic-config
mwts-network generic config

%package                generic-all
Summary:                meta package for generic mwts-network
Prefix:                 /usr
Group:                  Development/Scripts
Requires:               mwts-network, mwts-network-generic-config, mwts-network-generic-tests
%description            generic-all
meta package for generic mwts-network



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

%files generic-tests
/etc/min.d/mwts-network.min.conf
/usr/share/mwts-network-tests/tests.xml
/usr/lib/min/*.cfg

%files generic-config
/usr/lib/tests/*

%files generic-all

%post
ldconfig

%postun
ldconfig

