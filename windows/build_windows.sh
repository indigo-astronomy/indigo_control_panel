#!/bin/bash

set -x

APP=indigo_control_panel
MANAGER_APP=indigo_server_manager
QT_VER=5.15.2
MINGW_VER=81

SAVE_PATH=$PATH:/c/Program\ Files\ \(x86\)/Inno\ Setup\ 6/

#export PATH=/c/Qt/${QT_VER}/mingw${MINGW_VER}_32/bin:/c/Qt/Tools/mingw${MINGW_VER}0_32/bin/:$SAVE_PATH

#pushd .
#cd ../external/indigo_sdk
#rm -r lib/
#mkdir lib/
#cp -r lib86/* lib/
#popd

#pushd .
#[ ! -d "${APP}_32" ] && mkdir ${APP}_32
#cd ${APP}_32
#qmake ../../${APP}.pro
#mingw32-make -f Makefile.release

#[ ! -d "${APP}" ] && mkdir ${APP}
#cd ${APP}
#cp ../release/${APP}.exe .
#cp ../../../external/indigo_sdk/lib/libindigo_client.dll .
#windeployqt ${APP}.exe
#popd

export PATH=/c/Qt/${QT_VER}/mingw${MINGW_VER}_64/bin:/c/Qt/Tools/mingw${MINGW_VER}0_64/bin/:$SAVE_PATH

pushd .
cd ../external/indigo_sdk
rm -r lib/
mkdir lib/
cp -r lib64/* lib/
popd

pushd .
[ ! -d "${APP}_64" ] && mkdir ${APP}_64
cd ${APP}_64
qmake ../../${APP}_src/${APP}.pro
mingw32-make -f Makefile.release

[ ! -d "${APP}" ] && mkdir ${APP}
cd ${APP}
cp ../release/${APP}.exe .
cp ../../../external/indigo_sdk/lib/libindigo_client.dll .
windeployqt ${APP}.exe
popd

pushd .
[ ! -d "${MANAGER_APP}_64" ] && mkdir ${MANAGER_APP}_64
cd ${MANAGER_APP}_64
qmake ../../${MANAGER_APP}_src/${MANAGER_APP}.pro
mingw32-make -f Makefile.release

# the server manager is shipped in the control panel installer
cd ../${APP}_64/${APP}
cp ../../${MANAGER_APP}_64/release/${MANAGER_APP}.exe .
windeployqt ${MANAGER_APP}.exe
popd

APP_VERSION=`grep "VERSION " ../indigo_control_panel_src/version.h | sed 's/"//g' |awk '{ print $3 }'`

#iscc -DArch=32 -DMyAppVersion=$APP_VERSION ${APP}.iss
iscc -DArch=64 -DMyAppVersion=$APP_VERSION ${APP}.iss
