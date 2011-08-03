# spec file for mwts-feedback

%define buildroot	%{_topdir}/%{name}-%{version}-root

BuildRoot:      %{buildroot}
Name:           mwts-feedback
Summary:        Feedback test asset
License:        LGPL
Version:        0.0.6
Release:        0
Prefix:         /usr
Group:          Development/Tools
BuildRequires:  qt-devel, min-devel, min, mwts-common-devel, qt-mobility-devel
Requires:       mwts-common
Source:         %{name}-%{version}.tar.gz

%description
Feedback test asset.
            
%package        generic-tests
Summary:        MIN test case scripts for mwts-feedback
Prefix:         /usr/bin
Group:          Development/Tools
Requires:       min, mwts-feedback
%description    generic-tests
MIN test case scripts for mwts-feedback

%package        generic-config
Summary:        generic config for mwts-feedback
Prefix:          /usr/bin
Group:          Development/Tools
Requires:       min, mwts-feedback
%description    generic-config
generic config for mwts-feedback

%package        cli
Summary:        mwts-feedback command line tool
Prefix:          /usr/bin
Group:          Development/Tools
Requires:       mwts-feedback
%description    cli
mwts-feedback command line tool

%package        generic-all
Summary:        meta package for mwts-feedback generic
Prefix:          /usr/bin
Group:          Development/Tools
Requires:       mwts-feedback, mwts-feedback-cli, mwts-feedback-generic-tests, mwts-feedback-generic-config
%description    generic-all
meta package for mwts-feedback generic


%prep
%setup -q

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
/usr/lib/libmwts-feedback*
/usr/lib/min/libmin-mwts-feedback.*

%files generic-tests
/etc/min.d/*.min.conf
/usr/lib/min/*.cfg
/usr/share/mwts-feedback-generic-tests/tests.xml

%files generic-config
/usr/lib/tests/FeedbackTest.conf

%files generic-all

%files cli
%doc README
%doc COPYING
%doc DEPENDENCIES.png
%doc doc/MWTS.README
/usr/bin/mwts-feedback-cli

%post
ldconfig

%postun
ldconfig
