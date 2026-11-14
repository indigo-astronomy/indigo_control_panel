#  Unit tests. Build and run with:
#    cd tests && qmake && make check
#  The binary is built with AddressSanitizer/LeakSanitizer, so memory errors
#  and leaks fail the run.

QT += testlib
QT -= gui
CONFIG += c++11 console testcase debug
CONFIG -= app_bundle

TARGET = tst_propertycopy
OBJECTS_DIR = object
MOC_DIR = moc

SRC_DIR = $$PWD/../indigo_control_panel_src

INCLUDEPATH += $$SRC_DIR "$$PWD/../indigo/indigo_libs"

SOURCES += \
	tst_propertycopy.cpp \
	$$SRC_DIR/propertycopy.cpp

HEADERS += \
	$$SRC_DIR/propertycopy.h

unix {
	QMAKE_CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer -O1
	QMAKE_LFLAGS += -fsanitize=address

	INDIGO_LIB_DIR = $$PWD/../indigo/build/lib
	LIBS += -L"$$INDIGO_LIB_DIR"
	exists($$INDIGO_LIB_DIR/libindigo_client.so) | exists($$INDIGO_LIB_DIR/libindigo_client.a) | exists($$INDIGO_LIB_DIR/libindigo_client.dylib) {
		LIBS += -lindigo_client
	} else {
		LIBS += -lindigo
	}
	QMAKE_RPATHDIR += $$INDIGO_LIB_DIR
}

win32 {
	DEFINES += INDIGO_WINDOWS
	INCLUDEPATH += $$PWD/../external/indigo_sdk/include
	LIBS += $$PWD/../external/indigo_sdk/lib/libindigo_client.lib -lws2_32
}
