# dream.spec
#
%define name            dream
%define version         1.16
%define release         1
%define revision        365
 
Summary:	Digital Radio Mondiale (DRM) software receiver
Name:		%{name}
Version:	%{version}
Release:	%{release}
License:	GPL-2.0
Group:		Productivity/Hamradio/Other
Source0:	%{name}-%{version}-%{revision}.tar.gz
URL:		http://drm.sourceforge.net/
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires:	qt-devel make gcc-c++ libQtWebKit-devel
BuildRequires:	qwt-devel
BuildRequires:	fftw-devel
BuildRequires:	gpsd-devel
BuildRequires:	libsndfile-devel
BuildRequires:	libpcap-devel
#BuildRequires:	hamlib, hamlib-devel
BuildRequires:  portaudio-devel alsa-devel

%description
Digital Radio Mondiale (DRM) is the digital radio standard for
the long, medium and short wave ranges.

Dream is an open source software implementation of a DRM receiver.

%prep
#zypper ar http://download.opensuse.org/repositories/hamradio/openSUSE_12.2/hamradio.repo
%setup -n %{name}-%{version}-%{revision}

%build
qmake
make

%install
rm -rf %{buildroot}
make INSTALL_ROOT=%{buildroot} install

%clean
rm -rf %{buildroot}

%files
%defattr(-, root, root)
%doc README 
%{_mandir}/man1/dream.1.gz
%{_bindir}/dream

%changelog
* Sat Nov 17 2012 Julian Cable <jcable@sf.net> 1.16
- Fully implemented Qt4 and others
* Tue Oct 23 2012 Julian Cable <jcable@sf.net> 1.15
- Qt4 and others
* Wed Apr 24 2005 Alexander Kurpiers <kurpiers@sf.net>
- add libjournaline and hamlib
* Wed Dec 17 2003 Alexander Kurpiers <kurpiers@sf.net>
- some changes for RedHat
* Sun Nov 30 2003 Tomi Manninen <oh2bns@sral.fi>
- First try



