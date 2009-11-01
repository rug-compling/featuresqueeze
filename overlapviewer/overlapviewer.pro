QMAKE_CXXFLAGS += -O2 -Wall -Wextra
QMAKE_LFLAGS += -O2

TEMPLATE = app
TARGET = ../bin/overlapviewer
CONFIG += qt \
    debug_and_release \
    warn_on

FORMS += OverlapMainWindow.ui
HEADERS += OverlapMainWindow.hh
SOURCES += overlapviewer.cpp

mac {
        CONFIG -= app_bundle
}
