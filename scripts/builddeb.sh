#!/bin/sh

VERSION=${1}
VERSION_HEADER="indigo_control_panel_src/version.h"

DEBFULLNAME="Rumen Bogdanovski"
EMAIL="rumenastro@gmail.com"

__check_file_exits() {
    [ ! -f ${1} ] && { echo "file '${1}' not found"; exit 1; }
}

# Check for files where version number shall be replaced.
__check_file_exits ${VERSION_FILE}

# Make sure debian/changelog does not exists because we will genrate it.
rm -f debian/changelog

# Check for Debian package building executables and tools.
[ ! $(which dch) ] && { echo "executable 'dch' not found install package: 'devscripts'"; exit 1; }
[ ! $(which dpkg-buildpackage) ] && { echo "executable 'dpkg-buildpackage' not found install package: 'dpkg-dev'"; exit 1; }
[ ! $(which cdbs-edit-patch) ] && { echo "executable 'cdbs' not found install package: 'cdbs'"; exit 1; }

# Build dependencies
./build_libs.sh

# Create entry in debian/changelog.
dch --create --package "indigo-control-panel" --newversion ${VERSION} --distribution unstable --nomultimaint -t "Build from official upstream."

# Update version.h.
sed -i "s/\(PANEL_VERSION \).*/\1\"${VERSION}\"/g" ${VERSION_FILE}

# Finally build the package.
dpkg-buildpackage \-us \-uc \-I.git \-I\*.out[0-9]\* \-I\*.swp

# Cleanup debian/changelog.
rm -f debian/changelog
