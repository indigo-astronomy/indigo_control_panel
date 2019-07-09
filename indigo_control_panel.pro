QT += core gui widgets network
CONFIG += c++11 debug

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
	servicemodel.cpp \
	browserwindow.cpp \
	indigoservice.cpp \
	propertymodel.cpp \
	indigoclient.cpp \
	qindigoproperty.cpp \
	qindigoswitch.cpp \
	qindigotext.cpp \
	qindigonumber.cpp \
	qindigolight.cpp \
	qindigoblob.cpp \
	qindigoservers.cpp

RESOURCES += \
	qdarkstyle/style.qrc \
	resource/appicon.png \
	resource/bonjour_service.png \
	resource/manual_service.png \
	resource/led-red.png \
	resource/led-grey.png \
	resource/led-green.png \
	resource/led-orange.png \
	resource/led-grey-dev.png \
	resource/led-green-dev.png \
	resource/server.png \
	resource/ccd-grey.png \
	resource/ccd-green.png \
	resource/mount-grey.png \
	resource/mount-green.png \
	resource/wheel-grey.png \
	resource/wheel-green.png \
	resource/ao-grey.png \
	resource/ao-green.png \
	resource/focuser-grey.png \
	resource/focuser-green.png \
	resource/gps-grey.png \
	resource/gps-green.png \
	resource/guider-grey.png \
	resource/guider-green.png \
	resource/dome-grey.png \
	resource/dome-green.png

# Additional import path used to resolve QML modules in Qt Creator\'s code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
	version.h \
	servicemodel.h \
	browserwindow.h \
	indigoservice.h \
	propertymodel.h \
	indigoclient.h \
	qindigoproperty.h \
	qindigoswitch.h \
	qindigotext.h \
	qindigonumber.h \
	qindigolight.h \
	qindigoblob.h \
	qindigoservers.h \
	logger.h \
	conf.h

include(qtzeroconf/qtzeroconf.pri)

#unix:!mac {
#    CONFIG += link_pkgconfig
#    PKGCONFIG += indigo
#}

unix {
	INCLUDEPATH += "$${PWD}/indigo/indigo_libs"
	LIBS += -L"$${PWD}/indigo/build/lib" -lindigo
}

DISTFILES += \
	README.md \
	LICENCE.md \
