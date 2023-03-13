#!/bin/bash

APP=indigo_control_panel
QT_VER=5.15.2
MINGW_VER=81

SAVE_PATH=$PATH:/c/Program\ Files\ \(x86\)/Inno\ Setup\ 6/

export PATH=/c/Qt/${QT_VER}/mingw${MINGW_VER}_32/bin:/c/Qt/Tools/mingw${MINGW_VER}0_32/bin/:$SAVE_PATH

pushd .
cd ../external/indigo_sdk
rm -r lib/
mkdir lib/
cp -r lib86/* lib/
popd

pushd .
[ ! -d "${APP}_32" ] && mkdir ${APP}_32
cd ${APP}_32
qmake ../../${APP}.pro
mingw32-make -f Makefile.release

[ ! -d "${APP}" ] && mkdir ${APP}
cd ${APP}
cp ../release/${APP}.exe .
cp ../../../external/indigo_sdk/lib/libindigo_client.dll .
windeployqt ${APP}.exe
popd

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
qmake ../../${APP}.pro
mingw32-make -f Makefile.release

[ ! -d "${APP}" ] && mkdir ${APP}
cd ${APP}
cp ../release/${APP}.exe .
cp ../../../external/indigo_sdk/lib/libindigo_client.dll .
windeployqt ${APP}.exe
popd

APP_VERSION=`grep "VERSION " ../version.h | sed 's/"//g' |awk '{ print $3 }'`

iscc -DArch=32 -DMyAppVersion=$APP_VERSION ${APP}.iss
iscc -DArch=64 -DMyAppVersion=$APP_VERSION ${APP}.iss
