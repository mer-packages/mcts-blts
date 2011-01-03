# spec file for mwts-feedback

%define buildroot	%{_topdir}/%{name}-%{version}-root

BuildRoot:      %{buildroot}
Name:           mwts-feedback
Summary:        Feedback test asset
License:        LGPL
Version:        0.0.1
Release:        0
Prefix:         /usr
Group:          Development/Tools
BuildRequires:  qt-devel, min-devel, min, mwts-common-devel
Requires:       mwts-common
Source:         %{name}-%{version}.tar.gz

%description
Feedback test asset.
            
%package        scripts
Summary:        MIN test case scripts for mwts-feedback
Prefix:         /usr/bin
Group:          Development/Tools
Requires:       min, mwts-feedback
%description    scripts
MIN test case scripts for mwts-feedback


%package                cli
Summary:                mwts-feedback command line tool
Prefix:                 /usr/bin
Group:                  Development/Tools
Requires:               mwts-feedback
%description            cli
mwts-feedback command line tool


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
/usr/lib/tests/FeedbackTest.conf

%files scripts
%doc README
%doc COPYING
%doc DEPENDENCIES.png
%doc doc/MWTS.README
/etc/min.d/*.min.conf
/usr/lib/min/*.cfg
/usr/lib/min/libmin-mwts-feedback.*
/usr/share/mwts-feedback-scripts/tests.xml

%files cli
%doc README
%doc COPYING
%doc DEPENDENCIES.png
%doc doc/MWTS.README
/usr/bin/mwts-feedback-cli


%postun
ldconfig
