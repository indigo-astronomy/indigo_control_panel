QT += core gui widgets network concurrent
CONFIG += c++11 debug
QMAKE_CXXFLAGS += -O3
QMAKE_CXXFLAGS_RELEASE += -O3

unix:mac {
	CONFIG += app_bundle
	ICON=$$PWD/../resource/appicon.icns
}

OBJECTS_DIR=object
MOC_DIR=moc

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS QZEROCONF_STATIC

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
	main.cpp \
	qservicemodel.cpp \
	browserwindow.cpp \
	qindigoservice.cpp \
	propertymodel.cpp \
	indigoclient.cpp \
	qindigoproperty.cpp \
	qindigoswitch.cpp \
	qindigotext.cpp \
	qindigonumber.cpp \
	qindigolight.cpp \
	qindigoblob.cpp \
	qindigoservers.cpp \
	blobpreview.cpp \
	fits/fits.c \
	imagepreview.cpp \
	stretcher.cpp \
	utils.cpp


RESOURCES += \
	../resource/fonts.qrc \
	../qdarkstyle/style.qrc \
	../resource/images.qrc


# Additional import path used to resolve QML modules in Qt Creator\'s code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /usr/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
	version.h \
	qservicemodel.h \
	browserwindow.h \
	qindigoservice.h \
	propertymodel.h \
	indigoclient.h \
	qindigoproperty.h \
	qindigoswitch.h \
	qindigotext.h \
	qindigonumber.h \
	qindigolight.h \
	qindigoblob.h \
	blobpreview.h \
	qindigoservers.h \
	logger.h \
	fits/fits.h \
	pixelformat.h \
	imagepreview.h \
	image_preview_lut.h \
	stretcher.h \
	utils.h \
	conf.h


#unix:!mac {
#    CONFIG += link_pkgconfig
#    PKGCONFIG += indigo
#}

INCLUDEPATH += "$${PWD}/../indigo/indigo_libs"

INDIGO_LIB_DIR = $$PWD/../indigo/build/lib

unix:mac {
	INCLUDEPATH += "$${PWD}/../libjpeg"
	LIBS += -L"$$PWD/../libjpeg/.libs" -L"$$INDIGO_LIB_DIR"
	exists($$INDIGO_LIB_DIR/libindigo_client.dylib) | exists($$INDIGO_LIB_DIR/libindigo_client.a) {
		LIBS += -lindigo_client
	} else {
		INDIGO_SYS = $$system(ls /usr/local/lib /usr/lib /opt/homebrew/lib 2>/dev/null | grep -q libindigo_client && echo yes || echo no)
		contains(INDIGO_SYS, yes) {
			LIBS += -lindigo_client
		} else {
			LIBS += -lindigo
		}
	}
	LIBS += -ljpeg -ldl
}

unix:!mac {
	INCLUDEPATH += "$${PWD}/../libjpeg"
	LIBS += -L"$$PWD/../libjpeg/.libs" -L"$$INDIGO_LIB_DIR"
	exists($$INDIGO_LIB_DIR/libindigo_client.so) | exists($$INDIGO_LIB_DIR/libindigo_client.a) {
		LIBS += -lindigo_client -ljpeg -ldl
	} else {
		INDIGO_SYS = $$system(ls /usr/local/lib /usr/lib /lib /usr/lib64 /usr/lib/x86_64-linux-gnu 2>/dev/null | grep -q libindigo_client && echo yes || echo no)
		contains(INDIGO_SYS, yes) {
			LIBS += -lindigo_client -ljpeg -ldl
		} else {
			LIBS += -lindigo -ljpeg -ldl
		}
	}
}

#unix {
#	INCLUDEPATH += "$${PWD}/../libjpeg"
#	# LIBS += -L"$${PWD}/../libjpeg/.libs" -L"$${PWD}/../indigo/build/lib" -Wl,-Bstatic -lindigo -ljpeg -Wl,-Bdynamic -ldl
#	LIBS += -L"$${PWD}/../libjpeg/.libs" -L"$${PWD}/../indigo/build/lib" -lindigo -ljpeg -ldl
#}

win32 {
	DEFINES += INDIGO_WINDOWS
	INCLUDEPATH += $${PWD}/../external/indigo_sdk/include
	LIBS += $${PWD}/../external/indigo_sdk/lib/libindigo_client.lib -lws2_32
}
