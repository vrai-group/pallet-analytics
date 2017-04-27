QT       += core sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network printsupport

CONFIG   += c++11

TARGET = oos
TEMPLATE = app

SOURCES += main.cpp\
    process.cpp\
    camera.cpp \
    setup.cpp \
    sender.cpp


HEADERS  += process.h\
    camera.h \
    setup.h \
    sender.h


INCLUDEPATH += ../include/openni2

LIBS    += -L../lib/openni2 \
        -lOpenNI2 \
        -L../lib/openni2/OpenNI2/Drivers \
        -lOniFile \
        -lPS1080

LIBS    += -lopencv_core -lopencv_imgproc -lopencv_video -lopencv_features2d -lopencv_contrib -lopencv_legacy -lopencv_highgui

FORMS   +=
