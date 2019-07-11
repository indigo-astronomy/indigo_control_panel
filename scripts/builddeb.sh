#!/bin/sh

GIT_VERSION="0.1-2-poc"
DEB_VERSION=""

DEBFULLNAME="Rumen Bogdanovski"
EMAIL="rumen@skyarchive.org"

__check_file_exits() {
    [ ! -f ${1} ] && { echo "file '${1}' not found"; exit 1; }
}

# Check for files where version number shall be replaced.
__check_file_exits "debian/changelog"
__check_file_exits "version.h"

# Check for Debian package building executables and tools.
[ ! $(which dch) ] && { echo "executable 'dch' not found install package: 'devscripts'"; exit 1; }
[ ! $(which dpkg-buildpackage) ] && { echo "executable 'dpkg-buildpackage' not found install package: 'dpkg-dev'"; exit 1; }
[ ! $(which cdbs-edit-patch) ] && { echo "executable 'cdbs' not found install package: 'cdbs'"; exit 1; }

# Read DEB package version from debian/changelog
DEB_VERSION=$(cat debian/changelog | grep -m 1 -Po '^indigo-control-panel.*\(\K[^)]+')
[ ! ${DEB_VERSION} ] && { echo "Changelog version cannot be read from debian/changelog"; exit 1; }

# Read GIT TAG version via command git describe
# TODO.
[ ! ${GIT_VERSION} ] && { echo "GIT tag version cannot be read"; exit 1; }

# Update debian/changelog when GIT version does not match with debian/changelog version.
if [ "${GIT_VERSION}" != "${DEB_VERSION}" ]; then
    dch --newversion ${GIT_VERSION} --distribution unstable --nomultimaint -t "Build from official master upstream."
else
    echo "Debian changelog version and GIT tag version are equal, no changelog entry update required"
    exit 1
fi

# Update version.h.
sed -i "s/\(PANEL_VERSION \).*/\1\"${GIT_VERSION}\"/g" version.h

# Finally build the package.
dpkg-buildpackage \-us \-uc \-I.git \-I\*.out[0-9]\* \-I\*.swp
