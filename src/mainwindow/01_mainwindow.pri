DEPENDPATH += $${PWD}

INCLUDEPATH += $${PWD}

HEADERS += \
        $${PWD}/gwidget.h \
    $$PWD/gtransportunlockwizard.h

SOURCES += \
        $${PWD}/gwidget.cpp \
    $$PWD/gtransportunlockwizard.cpp

contains(DEFINES,DEVICE_TYPE_TL22) {
    FORMS +=    $$PWD/gwidget.ui
}

