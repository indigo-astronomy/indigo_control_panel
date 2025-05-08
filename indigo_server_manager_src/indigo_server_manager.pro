QT += core gui widgets network concurrent
CONFIG += c++11

TARGET = indigo_server_manager
TEMPLATE = app

RESOURCES += \
	../qdarkstyle/style.qrc \
	../resource/server_manager.png \
	../resource/led-red.png \
	../resource/led-grey.png \
	../resource/led-green.png \
	../resource/led-orange.png \
	../resource/indigo_logo.png

# Source files
SOURCES += \
	main.cpp \
	IndigoManagerWindow.cpp

# Header files
HEADERS += \
	IndigoManagerWindow.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /usr/bin
!isEmpty(target.path): INSTALLS += target