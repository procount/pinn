QT += waylandcompositor core gui

HEADERS += \
    compositor.h \
    window.h

SOURCES += main.cpp \
    compositor.cpp \
    window.cpp

TARGET = compositor
