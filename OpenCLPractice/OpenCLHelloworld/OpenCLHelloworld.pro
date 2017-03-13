TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

DISTFILES += \
    hello.cl \
    README

mac: LIBS += -framework OpenCL
else:unix|win32: LIBS += -lOpenCL
