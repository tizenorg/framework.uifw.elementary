#sbs-git:slp/pkgs/e/elementary elementary 1.0.0+svn.70492slp2+build11
Name:       elementary
Summary:    EFL toolkit for small touchscreens
Version:    1.7.1+svn.77535+build94r11
Release:    1
Group:      System/Libraries
License:    LGPLv2.1
URL:        http://trac.enlightenment.org/e/wiki/Elementary
Source0:    %{name}-%{version}.tar.gz
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  gettext
BuildRequires:  edje-tools
BuildRequires:  eet-tools
BuildRequires:  eina-devel
BuildRequires:  eet-devel
BuildRequires:  evas-devel
BuildRequires:  ecore-devel
BuildRequires:  edje-devel
BuildRequires:  edbus-devel
BuildRequires:  efreet-devel
BuildRequires:  ethumb-devel
BuildRequires:  emotion-devel
BuildRequires:  app-svc-devel
BuildRequires:  libx11-devel

%description
Elementary - a basic widget set that is easy to use based on EFL for mobile This package contains devel content.

%package devel
Summary:    EFL toolkit (devel)
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
EFL toolkit for small touchscreens (devel)

%package tools
Summary:    EFL toolkit (tools)
Group:      Development/Tools
Requires:   %{name} = %{version}-%{release}
Provides:   %{name}-bin
Obsoletes:  %{name}-bin

%description tools
EFL toolkit for small touchscreens (tools)

%prep
%setup -q

%build
export CFLAGS+=" -fPIC -Wall"
export LDFLAGS+=" -Wl,--hash-style=both -Wl,--as-needed"

%autogen --disable-static
%configure --disable-static \
	--enable-dependency-tracking \
	--disable-web

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install
mkdir -p %{buildroot}/usr/share/license
cp %{_builddir}/%{buildsubdir}/COPYING %{buildroot}/usr/share/license/%{name}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_libdir}/libelementary*
%{_libdir}/elementary/modules/*/*/*.so
%{_libdir}/edje/modules/elm/*/module.so
/usr/share/elementary/*
/usr/share/icons/*
/usr/share/locale/*
#exclude *.desktop files
%exclude /usr/share/applications/*
%manifest %{name}.manifest
/usr/share/license/%{name}

%files devel
%defattr(-,root,root,-)
/usr/include/*
%{_libdir}/libelementary.so
%{_libdir}/pkgconfig/elementary.pc

%files tools
%defattr(-,root,root,-)
/usr/bin/elementary_*
%{_libdir}/elementary_testql.so

