%define _unpackaged_files_terminate_build 0 

Summary: Template meego core test suite
Name: mcts-tablet-tests
Version: 1.0.0
Release: 4
License: GPLv2
Group: System/Libraries
Source: %name-%version.tar.gz
Requires: mcts-bluez-tests
Requires: mcts-connman-tests
Requires: mcts-display-tests
Requires: mcts-geoclue-tests
Requires: mcts-gupnp-tests
Requires: mcts-lsw-tests
Requires: mcts-ofono-tests
Requires: mcts-packagekit-tests
Requires: mcts-pulseaudio-tests
Requires: mcts-qtgfx-tests
Requires: mcts-rendercheck-tests
Requires: mcts-system-tests
Requires: mcts-tracker-tests
Requires: mcts-x-tests
Requires: mwts-accounts-generic-all
Requires: mwts-bluetooth-generic-all
Requires: mwts-buteo-generic-all
Requires: mwts-feedback-generic-all
Requires: mwts-filesystem-generic-all
Requires: mwts-gcamera-generic-all
Requires: mwts-gstreamer-generic-all
Requires: mwts-location-generic-all
Requires: mwts-messaging-generic-all
Requires: mwts-multimedia-generic-all
Requires: mwts-network-generic-all mwts-network-generic-config mwts-network-generic-tests mwts-network
Requires: mwts-ofono-generic-all
Requires: mwts-pim-generic-all
Requires: mwts-sensors-generic-all
Requires: mwts-systeminfo-generic-all
Requires: mwts-telepathy-generic-all
Requires: mwts-usb-generic-all

#BuildRoot: %_topdir/%name-%version-buildroot

%description

This is meego test suite package for common components


%prep
%setup -q

%build
./autogen
./configure --prefix=/usr
make

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
/opt/%name
/usr/share/%name

%changelog

