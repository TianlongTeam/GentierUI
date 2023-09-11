DEPENDPATH += $$PWD

INCLUDEPATH += $$PWD


contains(DEFINES,DEVICE_TYPE_TL22){
FORMS +=    $$PWD/gruneditor.ui }
contains(DEFINES,DEVICE_TYPE_TL23){
FORMS +=   $$PWD/gruneditor-23.ui  }
contains(DEFINES,DEVICE_TYPE_TL13){
FORMS +=   $$PWD/gruneditor-13.ui }
contains(DEFINES,DEVICE_TYPE_TL12){
FORMS +=   $$PWD/gruneditor-12.ui }

HEADERS += \
    $$PWD/gruneditor.h \
    $$PWD/gchannelsetdelegate.h \
    $$PWD/gfluoreditor.h \
    $$PWD/gstageadddelegate.h \
    $$PWD/ggradientdetail.h \
    $$PWD/geditdeletgate.h

SOURCES += \
    $$PWD/gruneditor.cpp \
    $$PWD/gchannelsetdelegate.cpp \
    $$PWD/gfluoreditor.cpp \
    $$PWD/gstageadddelegate.cpp \
    $$PWD/ggradientdetail.cpp \
    $$PWD/geditdeletgate.cpp



