# spec file for mcore-location
%define name mcts-sensorfw-tests
%define version 1.0.0
%define release 1
%define buildroot       %{_topdir}/%{name}-%{version}-%{release}-root

BuildRoot:              %{buildroot}
Summary:                mcts-sensorfw-tests is to test Qt-mobility sensor interface. 
License:                LGPL
Name:                   %{name}
Version:                %{version}
Release:                %{release}
Prefix:                 /usr
Group:                  Development/Tools
BuildRequires:          qt-devel,qt-mobility-devel, gcc-c++, qt-qmake
Requires:               libqtsensors1
Source:                 %{name}-%{version}.tar.gz

%description
mcts-sensorfw-tests is to test Qt-mobility sensor interface.

%package                tests
Summary:                mcts sensor tester 
Prefix:                 /usr
Group:                  Development/Tools
Requires:               libqtsensors1 
%description            tests
qt-mobility sensor interface test cases

%prep
%setup -q -n %{name}-%{version}

%build
qmake SensorTest.pro
make

%install
make install INSTALL_ROOT=%{buildroot}

%files tests
/opt/mcts-sensorfw-tests/
/opt/mcts-sensorfw-tests/tests.xml

%post
mkdir -p /var/log/tests
chmod 777 /var/log/tests

%postun
ldconfig
