#sbs-git:slp/pkgs/e/elementary elementary 1.0.0+svn.70492slp2+build11
Name:       elementary
Summary:    EFL toolkit for small touchscreens
Version:    1.7.1+svn.77535+build249b26
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
BuildRequires:  libx11-devel
%if %{_repository} == "mobile"
BuildRequires:  emotion-devel
BuildRequires:  app-svc-devel
%endif
%if %{_repository} == "wearable"
BuildRequires:  libslp-db-util-devel
%endif

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
%if %{_repository} == "wearable"
export CFLAGS+=" -DELM_FOCUSED_UI"
export CFLAGS+=" -DBUILD_B2"
%endif

cd %{_repository} && %autogen \
	   --disable-static \
       --enable-dependency-tracking \
       --disable-web

make %{?jobs:-j%jobs}
msgfmt -c --statistics -o po/dt_fmt.mo po/dt_fmt.po

%install
#rm -rf %{buildroot}
cd %{_repository} && %make_install
mkdir -p %{buildroot}/usr/share/license
cat %{_builddir}/%{buildsubdir}/COPYING* > %{buildroot}/usr/share/license/%{name}
cat %{_builddir}/%{buildsubdir}/COPYING* > %{buildroot}/usr/share/license/%{name}-tools
mkdir -p %{buildroot}/usr/share/locale/en_US/LC_MESSAGES/
cp %{_builddir}/%{buildsubdir}/%{_repository}/po/dt_fmt.mo %{buildroot}/usr/share/locale/en_US/LC_MESSAGES/

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
/usr/lib/libelementary*
/usr/lib/elementary/modules/*/*/*.so
/usr/lib/edje/modules/elm/*/module.so
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
/usr/lib/libelementary.so
/usr/lib/pkgconfig/elementary.pc

%files tools
%defattr(-,root,root,-)
/usr/bin/elementary_*
/usr/lib/elementary_testql.so
%manifest %{name}-tools.manifest
/usr/share/license/%{name}-tools
