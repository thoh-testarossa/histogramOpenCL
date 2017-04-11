TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    aggregation.cpp \
    data.cpp

mac: LIBS += -framework OpenCL
else:unix|win32: LIBS += -lOpenCL

HEADERS += \
    data.h \
    aggregation.h

DISTFILES += \
    cubeAggregationGeneration.cl \
    deviceAggregationGeneration.cl \
    cAggAggregate.cl
