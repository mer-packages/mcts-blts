# spec file for mwts-common

%define release         0
%define buildroot       %{_topdir}/%{name}-%{version}-%{release}-root

BuildRoot:              %{buildroot}
Summary:                Mwts-common is a Qt based framework library used by all MWTS test assets
License:                LGPL
Name:                   mwts-common
Version:                1.2.4
Release:                %{release}
Prefix:                 /usr
Group:                  Development/Tools
BuildRequires:          qt-devel, min-devel, min, gcc-c++
Requires:               libqtcore4, libqtgui4, min, procps
Source:                 %{name}-%{version}.tar.gz

%description
Mwts-common is a common library for MWTS test assets. It provides tool for memory and cpu monitoring, logging and reporting results.

%package                devel
Summary:                Mwts-common development files
Prefix:                 /usr
Group:                  Development/Tools
Requires:               qt-devel, mwts-common
%description            devel
Development headers and libraries for mwts-common

%package                tests
Summary:                Mwts-common MIN files
Prefix:                 /usr
Group:                  Development/Tools
Requires:               min, mwts-common
%description            tests
MIN test cases for mwts-common

%prep
%setup -q -n %{name}-%{version}

%build
qmake
make

%install
make install INSTALL_ROOT=%{buildroot}

%files
%doc README COPYING DEPENDENCIES.png
/usr/lib/libmwts-common.so.*
/usr/bin/*.py
/etc/min.d/min.conf

%files devel
%doc README COPYING DEPENDENCIES.png
/usr/lib/libmwts-common.so
/usr/include/MwtsCommon
/usr/include/mwts*.h

%files tests
%doc README COPYING DEPENDENCIES.png
/etc/min.d/*.min.conf
/usr/lib/min/*.cfg
/usr/lib/min/libmin-mwts-common.so*
/usr/share/applications/*.desktop

%post
mkdir -p /var/log/tests
chmod 777 /var/log/tests

%postun
ldconfig

%changelog
* Tue Jun 14 2011 Balazs Sipos <balazs.sipos@digia.com> - 1.2.5
- Fixes: MB#19137 - [FEA] mwts-common .csv file syntax to support the <series> tag in testrunner

* Wed Jun 1 2011 Balazs Sipos <balazs.sipos@digia.com> - 1.2.4
- Fixes: MB#18304 - .csv files written in wrong format

* Mon May 23 2011 Balazs Sipos <balazs.sipos@digia.com> - 1.2.3
- Fixed test case verdict (passed, failed) to be in sync at the min interface and..result file

* Wed May 18 2011 Rauno Vartiainen <rauno.vartiainen@digia.com> - 1.2.2
- Fixed log writing to be thread safe

* Fri May 13 2011 Balazs Sipos <balazs.sipos@digia.com> - 1.2.1
- Added memory and cpu usage format support to qa-reports

* Mon Mar 28 2011 Tuomo Pelkonen <tuomo.pelkonen@digia.com> - 1.2.0
- Added series measurement feature
- Support common configuration file MwtsGlobal.conf
- Support reading NFT limit files
- Support reporting NFT limits to OTS

* Tue Mar 15 2011 Rauno Vartiainen <rauno.vartiainen@digia.com> - 1.0.6
- Fixed the handling of direction parameter in StartClientThroughput function

* Thu Mar 10 2011 Reijo Korhonenn <reijo.korhonen@digia.com> - 1.0.5
- Fixed g_pApp pointer setting correctly, when creating application

* Tue Feb 22 2011 Rauno Vartiainen <rauno.vartiainen@digia.com> - 1.0.4
- Fixed packaging

* Tue Feb 1 2011 Rauno Vartiainen <rauno.vartiainen@digia.com> - 1.0.3
- Removed hard coded path from iperf starting and fine tuned QProcess error handling

* Thu Dec 16 2010 Tuomo Pelkonen <tuomo.pelkonen@digia.com> - 1.0.2
- added OBS support

* Mon Nov 22 2010 Tuomo Pelkonen <tuomo.pelkonen@digia.com> - 1.0.1
- Fixes crashes on some environments

* Mon Sep 20 2010 Tuomo Pelkonen <tuomo.pelkonen@digia.com> - 1.0.0
- Initial version
