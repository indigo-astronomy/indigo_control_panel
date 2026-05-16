#!/bin/sh

VERSION=${1}
FLAVOR=${2:-indigo}
VERSION_FILE="indigo_control_panel_src/version.h"

DEBFULLNAME="Rumen Bogdanovski"
EMAIL="rumenastro@gmail.com"

__check_file_exits() {
    [ ! -f ${1} ] && { echo "file '${1}' not found"; exit 1; }
}

# Validate the requested INDIGO flavor.
if [ "${FLAVOR}" != "indigo" ] && [ "${FLAVOR}" != "indigo3" ]; then
    echo "Invalid INDIGO flavor '${FLAVOR}', expected 'indigo' or 'indigo3'"
    exit 1
fi

if [ "${FLAVOR}" = "indigo3" ]; then
    PACKAGE="indigo-control-panel-indigo3"
else
    PACKAGE="indigo-control-panel"
fi

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

# For an indigo3 build swap in the flavored debian/control and a matching
# install file. The originals are restored after the package is built.
if [ "${FLAVOR}" = "indigo3" ]; then
    __check_file_exits "debian/control.indigo3"
    cp debian/control debian/control.icp-bak
    cp debian/control.indigo3 debian/control
    cp debian/indigo-control-panel.install debian/${PACKAGE}.install
fi

# Tell qmake (via indigo_control_panel.pro) which INDIGO client
# library to link against.
export ICP_INDIGO_FLAVOR=${FLAVOR}

# Create entry in debian/changelog.
dch --create --package "${PACKAGE}" --newversion ${VERSION} --distribution unstable --nomultimaint -t "Build from official upstream."

# Update version.h.
sed -i "s/\(PANEL_VERSION \).*/\1\"${VERSION}\"/g" ${VERSION_FILE}

# Finally build the package.
dpkg-buildpackage \-us \-uc \-I.git \-I\*.out[0-9]\* \-I\*.swp
BUILD_RESULT=$?

# Cleanup debian/changelog.
rm -f debian/changelog

# Restore the original packaging files for an indigo3 build.
if [ "${FLAVOR}" = "indigo3" ]; then
    mv debian/control.icp-bak debian/control
    rm -f debian/${PACKAGE}.install
fi

if [ ${BUILD_RESULT} -ne 0 ]; then
    echo "dpkg-buildpackage failed with exit code ${BUILD_RESULT}"
    exit ${BUILD_RESULT}
fi
