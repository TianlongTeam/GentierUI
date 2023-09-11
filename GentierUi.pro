#-------------------------------------------------
#
# Project created by QtCreator 2014-11-17T10:43:05
#
#-------------------------------------------------

QT       += core gui declarative network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport

TEMPLATE = app
DEFINES += DEVICE_TYPE_TL22

TARGET = ui_tl22

MOC_DIR      = ./tmp
OBJECTS_DIR  = ./tmp
UI_DIR       = ./tmp
RCC_DIR      = ./tmp

#DEFINES += QT_NO_DEBUG_OUTPUT

unix{
    target.files = ui_tl22
    target.path = ~/app/ui

    INSTALLS += target
    QMAKE_RPATHDIR += ~/app/ui/lib
}

DESTDIR = ./bin

INCLUDEPATH += \
    $$PWD/comm/inc \
    $$(QWT_DIR)/include

DEPENDPATH += ./src

include (./src/mainwindow/01_mainwindow.pri)
include (./src/overview/02_overview.pri)
include (./src/runeditor/03_runeditor.pri)
include (./src/rawdata/04_rawdata.pri)
include (./src/utilities/05_utilities.pri)
include (./src/widgetkeyboard/07_widgetkeyboard.pri)
include (./src/quazip-0.7.1/quazip/08_quazip.pri)
include (./src/tempctrlimage/09_tempctrlimage.pri)
include (./src/common/10_common.pri)


SOURCES += ./src/main.cpp

LIBS += -L$$PWD/comm/lib -lqwt

TRANSLATIONS += \
    armui_zh.ts

RESOURCES += \
    src/armui_rcs.qrc

QMAKE_CXXFLAGS += -fPIC

DISTFILES += \
    src/armuircs/png/back.png \
    src/armuircs/png/folder.png \
    src/armuircs/png/la1.png \
    src/armuircs/png/la2.png
