# spec file for n900-jkl-core-tests

%define buildroot	%{_tmppath}/%{name}-%{version}-%{release}-root  

BuildRoot:		%{buildroot}
Summary: 		Core tests for MeeGo N900 automatic testing from JKL
License: 		LGPL
Name: 			n900-jkl-core-tests
Version: 		0.0.1
Release: 		1
Prefix: 		/usr/share
Group: 			Testing
BuildRequires:	qt-devel
Requires:		mwts-network-generic-all, mwts-filesystem-generic-all, mwts-qtmultimedia-generic-all
Source: 		%{name}-%{version}.tar.gz

%description
Core tests for MeeGo N900 automatic testing from JKL

%prep
%setup -q -n %{name}-%{version}

%build
qmake
make

%install
make install INSTALL_ROOT=%{buildroot}

%files
/usr/share/n900-jkl-core-tests/tests.xml
