#!/usr/bin/env bash
# Builds a .deb from an already-configured build tree.
#
#   usage: packaging/debian/build-deb.sh <build-dir> <version> <output.deb>
#
# Dependencies are computed with dpkg-shlibdeps rather than hand-maintained.
# A hardcoded Depends line is a standing invitation to ship an uninstallable
# package: library package names drift (libxml2 -> libxml2-16, the whole t64
# transition), and nothing about a successful build reveals that the list has
# gone stale. Asking dpkg what the binary actually links means the answer is
# right on every distro and release without anyone maintaining a table.
set -euo pipefail

BUILD_DIR="${1:?build dir}"
VERSION="${2:?version}"
OUTPUT="${3:?output .deb path}"

STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT

DESTDIR="$STAGE/pkg" cmake --install "$BUILD_DIR" --prefix /usr >/dev/null
mkdir -p "$STAGE/pkg/DEBIAN"

# dpkg-shlibdeps insists on running from a source tree with debian/control
# present, and writes its answer into debian/substvars.
mkdir -p "$STAGE/src/debian"
cat > "$STAGE/src/debian/control" <<CONTROL
Source: huenicorn
Package: huenicorn
Architecture: amd64
CONTROL

( cd "$STAGE/src" && dpkg-shlibdeps -O --ignore-missing-info "$STAGE/pkg/usr/bin/huenicorn" ) \
  > "$STAGE/shlibdeps" 2>/dev/null || true
DEPENDS="$(sed -n 's/^shlibs:Depends=//p' "$STAGE/shlibdeps")"

if [ -z "$DEPENDS" ]; then
  echo "dpkg-shlibdeps produced no dependencies -- refusing to ship a package that claims to need nothing" >&2
  exit 1
fi

echo "computed Depends: $DEPENDS"

cat > "$STAGE/pkg/DEBIAN/control" <<CONTROL
Package: huenicorn
Version: $VERSION
Section: utils
Priority: optional
Architecture: amd64
Depends: $DEPENDS
Maintainer: OpenJowelSofts <https://gitlab.com/openjowelsofts/huenicorn>
Homepage: https://gitlab.com/openjowelsofts/huenicorn
Description: Ambient lighting from your screen for Philips Hue
 Huenicorn captures the screen and streams matching colours to Hue
 entertainment zones, configured through a web interface it serves itself.
CONTROL

dpkg-deb --build --root-owner-group "$STAGE/pkg" "$OUTPUT" >/dev/null
echo "built $OUTPUT"
dpkg-deb --field "$OUTPUT" Package Version Depends
