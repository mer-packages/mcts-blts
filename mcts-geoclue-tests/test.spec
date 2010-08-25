%define name mcts-geoclue-tests
%define version 1.0.0
%define release 2
%define _unpackaged_files_terminate_build 0 


Summary: meego core test suite for geoclue component
Name: %{name}
Version: %{version}
Release: %{release}
License: GPLv2
Group: System/Libraries
Source: %{name}-%{version}.tar.gz
Requires:   dbus  
Requires:   geoclue
BuildRequires:	geoclue-devel
BuildRequires:  glib2-devel  
BuildRequires:  dbus-glib-devel 
BuildRequires:  libxml2-devel  
BuildRequires:  GConf-dbus-devel  
BuildRoot: %{_topdir}/tmp/%{name}-%{version}-buildroot

%description

This is meego test suite package for geoclue component. Geoclue test suites are developed with focusing on API level testing. Each API is tested with a dedicated test suite. More complex usage scenarios can be covered by the geoclue package built-in tool: geoclue-test-gui.  


%prep
%setup -q

%build
./autogen
%configure --prefix=/usr
make %{?jobs:-j%jobs}  

%install
rm -rf $RPM_BUILD_ROOT
%makeinstall TEST_BIN_DIR="$RPM_BUILD_ROOT/opt/%{name}-%{version}" XML_DATA_DIR="$RPM_BUILD_ROOT/usr/share/%{name}"

%post 
cd /opt
ln -s %{name}-%{version} %{name}

%postun
rm -f /opt/%{name}

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)  
# >> files  
%doc README  
%dir /opt/%{name}-%{version}
%{_datadir}/dbus-1/services/org.freedesktop.Geoclue.Providers.Test.service
%{_datadir}/geoclue-providers/geoclue-test.provider 
%{_bindir}/uexec  
%{_libexecdir}/geoclue-test 
/opt/%{name}-%{version}/*
%{_datadir}/%{name}/*
# << files  

%changelog
