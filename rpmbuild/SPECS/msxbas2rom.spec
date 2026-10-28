Name: msxbas2rom
Version: 1.4.2.0
Release: 1%{?dist}
Summary: MSX-BASIC to ROM compiler

Group: Applications
License: GPL-3
URL: https://github.com/amaurycarvalho/msxbas2rom
Packager: amauryspires@gmail.com
Vendor: amauryspires@gmail.com

Source: msxbas2rom.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-buildroot
BuildArch: x86_64

%global debug_package %{nil}

%description
msxbas2rom is a command-line tool that converts MSX-BASIC source files into ROM format.

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
* Wed Oct 28 2026 Amaury Carvalho <amauryspires@gmail.com>
- Fix Pletter heap overflow for small resource blobs
- Fix string concatenation by giving each temporary string its own 256-byte slot
- Normalize startup to SCREEN 0 40 columns for a consistent text layout
* Tue Oct 27 2026 Amaury Carvalho <amauryspires@gmail.com>
- Normalize tile bank values above 2 to all banks and harden tile command parameter validation
- Add optional offset and size parameters to CMD RSCTORAM
- Redefine RESOURCE(n) to return a usable RAM pointer
* Fri Oct 23 2026 Amaury Carvalho <amauryspires@gmail.com>
- Fix make lint gate failures and indeterminate Logger::LogEntry fields
* Mon Jun 29 2026 Amaury Carvalho <amauryspires@gmail.com>
- Fix Windows startup Unicode argv to ensure CLI runs reliably on Windows
