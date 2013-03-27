Name:       blts-tools
Summary:    Baselayer test tools
Version:    0.1
Release:    1
Group:      Development/Tools
License:    GPLv2
URL:        http://github.com/mer-packages/mcts-blts
Source0:    %{name}-%{version}.tar.gz

%description
Tools for running baselayer tests which need to run with root access.

%prep
%setup -q -n %{name}-%{version}

%build
make %{?jobs:-j%jobs}

%install
%make_install DESTDIR=%{buildroot}

%files
%defattr(-,root,root,-)
%attr(4755, root, root) /usr/sbin/run-blts-root
