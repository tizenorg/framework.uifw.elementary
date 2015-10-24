%define dbus_unavailable 1

%bcond_with wayland
Name:           elementary
Version:        1.13.33
Release:        0
License:        LGPL-2.1+
Summary:        EFL toolkit for small touchscreens
Url:            http://trac.enlightenment.org/e/wiki/Elementary
Group:          Graphics & UI Framework/Development
Source0:        elementary-%{version}.tar.bz2
Source1001:     elementary.manifest
BuildRequires:  doxygen
BuildRequires:  gettext-devel
BuildRequires:  pkgconfig(ecore)
BuildRequires:  pkgconfig(ecore-evas)
BuildRequires:  pkgconfig(ecore-file)
BuildRequires:  pkgconfig(ecore-imf)

%if %{with wayland}
BuildRequires:  pkgconfig(ecore-wayland)
%endif

BuildRequires:  pkgconfig(ecore-x)
BuildRequires:  pkgconfig(x11)

BuildRequires:  pkgconfig(eldbus)
BuildRequires:  pkgconfig(edje)
BuildRequires:  pkgconfig(eet)
BuildRequires:  pkgconfig(efreet)
BuildRequires:  pkgconfig(eina)
BuildRequires:  pkgconfig(evas)
BuildRequires:  pkgconfig(ethumb)
# TIZEN_ONLY(20150709): Don't build unused widget
#BuildRequires:  pkgconfig(emotion)
#
# TINEN ONLY (20150112) : NOT FIXED
BuildRequires:  pkgconfig(edbus)
#
BuildRequires:  pkgconfig(icu-i18n)
BuildRequires:  eet-tools
BuildRequires:  eolian-devel
BuildRequires:  python-devel
### Recommends:     %{name}-locale = %{version}

%description
Elementary is a widget set. It is a new-style of widget set much more canvas
object based than anything else. Why not ETK? Why not EWL? Well they both
tend to veer away from the core of Evas, Ecore and Edje a lot to build their
own worlds. Also I wanted something focused on embedded devices -
specifically small touchscreens. Unlike GTK+ and Qt, 75% of the "widget set"
is already embodied in a common core - Ecore, Edje, Evas etc. So this
fine-grained library splitting means all of this is shared, just a new
widget "personality" is on top. And that is... Elementary, my dear watson.
Elementary.

%package examples
Summary:   EFL elementary examples

%description examples
EFL elementary examples

%package tools
Summary:   EFL elementary configuration and test apps

%description tools
EFL elementary configuration and test apps package

%package devel
Summary:        Development files for elementary
Group:          Development/Libraries
Requires:       %{name} = %{version}

%description devel
Development components for the elementary package


%prep
%setup -q
cp %{SOURCE1001} .

%build

%if "%{?tizen_profile_name}" == "wearable"
    export CFLAGS+=" -DELM_FEATURE_WEARABLE"
    %if "%{?model_build_feature_formfactor}" == "circle"
        export CFLAGS+=" -DELM_FEATURE_WEARABLE_C1"
    %endif
%endif

%autogen --disable-static \
%if %{with wayland}
         --enable-ecore-wayland \
%endif
%if %dbus_unavailable
         --disable-build-examples
%else
         --enable-build-examples
%endif

%__make %{?_smp_mflags}

%install
%make_install

%find_lang %{name}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%lang_package
%manifest %{name}.manifest

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
#%license COPYING
%{_bindir}/elementary_quicklaunch
%{_bindir}/elementary_run
%{_libdir}/edje/*
%{_libdir}/elementary/modules/*
%{_libdir}/*.so.*
%{_datadir}/elementary/*
%{_datadir}/icons/elementary.png

%if ! %dbus_unavailable
%files examples
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/elementary/examples/*
%endif

## theme is installed from efl-theme-tizen
## config is installed from elm-misc
%exclude %{_datadir}/elementary
#%exclude %{_datadir}/elementary/themes
#%exclude %{_datadir}/elementary/config
%exclude %{_libdir}/elementary/modules/test_*

%files tools
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_datadir}/applications/*
%{_bindir}/elementary_config
%{_bindir}/elementary_test*
%{_bindir}/elementary_codegen
%{_bindir}/elm_prefs_cc

%files devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/%{name}-1/*.h*
%{_datadir}/eolian/include/%{name}-1/*.eo
%{_libdir}/*.so
%{_libdir}/pkgconfig/*.pc
%{_libdir}/cmake/Elementary/ElementaryConfig.cmake
%{_libdir}/cmake/Elementary/ElementaryConfigVersion.cmake

