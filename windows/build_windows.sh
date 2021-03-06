#!/bin/bash

APP=indigo_control_panel

SAVE_PATH=$PATH:/c/Program\ Files\ \(x86\)/Inno\ Setup\ 6/

export PATH=/c/Qt/5.12.10/mingw73_32/bin:/c/Qt/Tools/mingw730_32/bin/:$SAVE_PATH

pushd .
[ ! -d "${APP}_32" ] && mkdir ${APP}_32
cd ${APP}_32
qmake ../../${APP}.pro
mingw32-make -f Makefile.release

[ ! -d "${APP}" ] && mkdir ${APP}
cd ${APP}
cp ../release/${APP}.exe .
windeployqt ${APP}.exe
popd

pushd .
export PATH=/c/Qt/5.12.10/mingw73_64/bin:/c/Qt/Tools/mingw730_64/bin/:$SAVE_PATH
[ ! -d "${APP}_64" ] && mkdir ${APP}_64
cd ${APP}_64
qmake ../../${APP}.pro
mingw32-make -f Makefile.release

[ ! -d "${APP}" ] && mkdir ${APP}
cd ${APP}
cp ../release/${APP}.exe .
windeployqt ${APP}.exe
popd

APP_VERSION=`grep "VERSION " ../version.h | sed 's/"//g' |awk '{ print $3 }'`

iscc -DArch=32 -DMyAppVersion=$APP_VERSION ${APP}.iss
iscc -DArch=64 -DMyAppVersion=$APP_VERSION ${APP}.iss
