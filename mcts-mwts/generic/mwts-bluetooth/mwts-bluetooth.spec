# spec file for mwts-bluetooth

%define buildroot	%{_topdir}/%{name}-%{version}-%{release}-root

BuildRoot:      %{buildroot}
Summary:        A test asset for BlueZ
License:        LGPL
Name:           mwts-bluetooth
Version:        0.1.1
Release:        0
Prefix:         /usr
Group:          Development/Tools
BuildRequires:  qt-devel, min-devel, min, bluez-libs-devel, mwts-common-devel
Requires:       mwts-common, bluez-libs, openobex, obexd, obexd-server
Source:         %{name}-%{version}.tar.gz

%description
Bluetooth test asset for BlueZ and libbluetooth.
C++ and MIN interface

%package        generic-tests
Summary:        Min test cases
Prefix:         /usr
Group:          Development/Tools
Requires:       mwts-bluetooth
%description    generic-tests
Min test cases, generic version

%package        generic-config
Summary:        configuration file, generic
Prefix:         /usr
Group:          Development/Tools
Requires:       mwts-bluetooth
%description    generic-config
configuration file, generic

%package        generic-all
Summary:        mwts-bluetooth, meta package, generic
Prefix:         /usr
Group:          Development/Tools
Requires:       mwts-bluetooth, mwts-bluetooth-generic-config, mwts-bluetooth-generic-tests
%description    generic-all
mwts-bluetooth, meta package, generic

            
# %package      cli
# Summary:      Command line interface for mwts-bluetooth
# Prefix:       /usr
# Group:        Development/Tools
# Requires:     mwts-bluetooth
# %description  cli
# Command line interface for mwts-bluetooth

%package        tools
Summary:        Bluetooth test tools
Prefix:         /usr
Group:          Development/Tools
Requires:       bluez-libs, openobex, obexd, obexd-server, python
%description    tools
Bluetooth testing tools

%prep
%setup -q

%build
qmake
make

%install
make install INSTALL_ROOT=%{buildroot}

%files 
%doc README
%doc doc/MWTS.README
/usr/lib/*.so*
/usr/lib/min/*.so

%files generic-tests
/etc/min.d/mwts-bluetooth.min.conf
/usr/share/mwts-bluetooth-tests/tests.xml
/usr/lib/min/*.cfg

%files generic-config
/usr/lib/tests/*.conf

%files generic-all
%doc README


# %files cli
# %doc README
# %doc doc/MWTS.README
# /usr/bin/*

%files tools
%doc README
%doc doc/MWTS.README
/usr/share/mwts-bluetooth-tools/*.py*

%post
mkdir /var/log/tests/data
chmod 777 /var/log/tests/data
dd bs=10000 count=1000 if=/dev/zero of=/var/log/tests/data/proto10
chmod 777 /var/log/tests/data/proto10
ldconfig

%postun
ldconfig

