Name:       elementary
Summary:    EFL toolkit for small touchscreens
Version:    1.7.1+svn.77535+build572
Release:    1
Group:      System/Libraries
License:    LGPL-2.1+ and CC-BY-SA-3.0
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
BuildRequires:  pkgconfig(icu-i18n)
BuildRequires:  model-build-features
%if "%{?tizen_profile_name}" != "mobile"
%if "%{?tizen_profile_name}" != "wearable"
BuildRequires:  emotion-devel
BuildRequires:  pkgconfig(appsvc)
%endif
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

export CFLAGS+=" -DELM_FOCUSED_UI"
export CFLAGS+=" -DELM_FEATURE_MULTI_WINDOW"

%if "%{?tizen_profile_name}" == "wearable"
    export CFLAGS+=" -DELM_FEATURE_WEARABLE"
    %if "%{?model_build_feature_formfactor}" == "circle"
        export CFLAGS+=" -DELM_FEATURE_WEARABLE_CIRCLE"
    %else
        export CFLAGS+=" -DELM_FEATURE_WEARABLE_RECTANGLE"
    %endif
%else
    export CFLAGS+=" -DELM_FEATURE_LITE"
%endif

%autogen \
%if "%{?tizen_profile_name}" == "mobile"
	--disable-emotion \
%endif
%if "%{?tizen_profile_name}" == "wearable"
	--disable-emotion \
%endif
	--disable-static \
	--enable-dependency-tracking

make %{?jobs:-j%jobs}
msgfmt -c --statistics -o po/dt_fmt.mo po/dt_fmt.po

%install
%make_install

mkdir -p %{buildroot}/%{_datadir}/locale/en_US/LC_MESSAGES/
cp %{_builddir}/%{buildsubdir}/po/dt_fmt.mo %{buildroot}/%{_datadir}/locale/en_US/LC_MESSAGES/

mkdir -p %{buildroot}/%{_datadir}/license
cp %{_builddir}/%{buildsubdir}/COPYING %{buildroot}/%{_datadir}/license/%{name}
cp %{_builddir}/%{buildsubdir}/COPYING %{buildroot}/%{_datadir}/license/%{name}-tools

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_libdir}/libelementary.so.*
%{_libdir}/elementary/modules/*/*/*.so
%{_libdir}/edje/modules/elm/*/module.so
%{_datadir}/elementary/edje_externals
%{_datadir}/locale
%{_datadir}/license/%{name}
%manifest %{name}.manifest

## theme is installed from efl-theme-tizen
## config is installed from elm-misc
%exclude %{_datadir}/elementary/themes
%exclude %{_datadir}/elementary/config
%exclude %{_libdir}/elementary/modules/test_*

%files devel
%defattr(-,root,root,-)
/usr/include/*
%{_libdir}/libelementary.so
%{_libdir}/pkgconfig/elementary.pc

%files tools
%defattr(-,root,root,-)
%{_bindir}/elementary_*
%{_libdir}/elementary_testql.so
%{_datadir}/icons
%{_datadir}/applications
%{_datadir}/elementary/objects
%{_datadir}/elementary/images
%{_datadir}/license/%{name}-tools
%manifest %{name}-tools.manifest
