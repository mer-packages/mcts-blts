# spec file for mwts-network

%define buildroot	%{_topdir}/%{name}-%{version}-root

BuildRoot:		%{buildroot}
Summary: 		Mwts-network is a wlan/psd networking test asset
License: 		LGPL
Name: 			mwts-network
Version: 		%{ver}
Release: 		0
Prefix: 		/usr
Group: 			Testing
BuildRequires:		qt-devel, min-devel, mwts-common-devel
Requires:		qt-x11, min
Source: 		%{name}-%{version}.tar.gz

%description
Mwts-network is a wlan/psd networking test asset
            
%package                scripts
Summary:                Min test-case scripts for mwts-network
Prefix:                 /usr
Group:                  Development/Scripts
Requires:               mwts-network
%description            scripts
MIN test case scripts for mwts-common


%prep
%setup -q -n %{name}-%{version}

%build
qmake "CONFIG+=plugin"
make

%install
make install INSTALL_ROOT=%{buildroot}

%files
%doc README
%doc doc/MWTS.README
/usr/lib/*.so*
/usr/lib/min/*.so*
/usr/lib/tests/*


%files scripts
%doc README
%doc doc/MWTS.README
%doc DEPENDENCIES.png
%doc COPYING
/etc/min.d/mwts-network.min.conf
/usr/share/mwts-network-scripts/tests.xml
/usr/lib/min/*.cfg


%post
mkdir -p /var/log/tests
chmod 777 /var/log/tests

%postun
ldconfig

