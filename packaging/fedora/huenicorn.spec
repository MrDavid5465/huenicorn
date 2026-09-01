# Builds from the tree this spec ships in, not from a fresh upstream clone --
# so an rpm's contents are the commit it was built from. (The alternative,
# cloning master in %prep, means a fix on the branch being released is absent
# from its own release; that bug shipped in a sibling project for months.)
#
# No explicit Requires: rpmbuild derives them from the linked libraries. A
# hand-maintained list only drifts out of date, and unlike a build failure the
# breakage shows up at install time on someone else's machine.
Name:           huenicorn
Version:        %{?_version}%{!?_version:1.2.0}
Release:        1%{?dist}
Summary:        Ambient lighting from your screen for Philips Hue
License:        GPL-3.0-only
URL:            https://gitlab.com/openjowelsofts/huenicorn
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc-c++ cmake pkgconf-pkg-config
BuildRequires:  opencv-devel libcurl-devel mbedtls-devel
BuildRequires:  libX11-devel libXext-devel libXrandr-devel libXi-devel libXcursor-devel
BuildRequires:  pipewire-devel glib2-devel
BuildRequires:  json-devel glm-devel

%description
Huenicorn captures the screen and streams matching colours to Philips Hue
entertainment zones, giving an ambient lighting effect around whatever is
playing. It is configured through a web interface served by the application
itself. Screen capture uses the desktop portal on Wayland and X11 directly
elsewhere.

%prep
%autosetup -n %{name}-%{version}

%build
# HUENICORN_FETCH_DEPS is on because cpp-httplib is not packaged for Fedora;
# json and glm are found as system packages and only the missing one is
# fetched, at the version upstream's own CMakeLists pins.
# The fetched dependency installs itself into the prefix unless told not to,
# which rpmbuild rightly refuses as unpackaged files -- and which dpkg-deb
# silently accepts, so the .deb was shipping httplib.h and its cmake files
# under the huenicorn package until this was noticed here.
%cmake -DCMAKE_BUILD_TYPE=Release -DHUENICORN_FETCH_DEPS=ON \
       -DHTTPLIB_INSTALL=OFF -DJSON_Install=OFF -DGLM_BUILD_INSTALL=OFF
%cmake_build

%install
%cmake_install

%files
%license LICENSE.txt
%{_bindir}/huenicorn
%{_datadir}/applications/io.gitlab.openjowelsofts.huenicorn.desktop
%{_datadir}/metainfo/io.gitlab.openjowelsofts.huenicorn.metainfo.xml
%{_datadir}/icons/hicolor/scalable/apps/io.gitlab.openjowelsofts.huenicorn.svg

%changelog
* Mon Aug 31 2026 Packaging <packaging@example.invalid> - 1.2.0-1
- Initial packaging
