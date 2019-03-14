QT += core gui widgets network
CONFIG += c++11 debug

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
    qindigolight.cpp

RESOURCES += qdarkstyle/style.qrc led-red.png led-grey.png led-green.png led-orange.png

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    servicemodel.h \
    browserwindow.h \
    indigoservice.h \
    propertymodel.h \
    indigoclient.h \
    qindigoproperty.h \
    qindigoswitch.h \
    qindigotext.h \
    qindigonumber.h \
    qindigolight.h

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
    README.md
