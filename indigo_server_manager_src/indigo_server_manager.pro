QT += core gui widgets network concurrent
CONFIG += c++11

TARGET = indigo_server_manager
TEMPLATE = app

RESOURCES += \
	../resource/fonts.qrc \
	../resource/images.qrc \
	../qdarkstyle/style.qrc

# Source files
SOURCES += \
	main.cpp \
	IndigoManagerWindow.cpp

# Header files
HEADERS += \
	version.h \
	IndigoManagerWindow.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /usr/bin
!isEmpty(target.path): INSTALLS += target
