Name: msxbas2rom
Version: 0.3.2.9
Release: 1%{?dist}
Summary: MSX BASIC to ROM compiler

Group: Applications
License: GPL-3
URL: https://github.com/amaurycarvalho/msxbas2rom
Packager: amauryspires@gmail.com
Vendor: amauryspires@gmail.com

Source: msxbas2rom.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-buildroot
BuildArch: x86_64

%description
msxbas2rom is a command-line tool that converts MSX BASIC source files into ROM format.

%prep

%setup -n bin/Release

%build

%install
mkdir -p %{buildroot}/usr/bin
install -m 0755 msxbas2rom %{buildroot}/usr/bin/msxbas2rom

%clean

%files
/usr/bin/msxbas2rom

%changelog
* Mon Feb 11 2019 Amaury Carvalho <amauryspires@gmail.com>
- Initial release