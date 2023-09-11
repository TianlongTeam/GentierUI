DEPENDPATH += $$PWD

INCLUDEPATH += $$PWD


contains(DEFINES,DEVICE_TYPE_TL22){
FORMS +=    $$PWD/grawdata.ui }
contains(DEFINES,DEVICE_TYPE_TL23){
FORMS +=   $$PWD/grawdata.ui  }
contains(DEFINES,DEVICE_TYPE_TL13){
FORMS +=   $$PWD/grawdata-13.ui }
contains(DEFINES,DEVICE_TYPE_TL12){
FORMS +=   $$PWD/grawdata.ui  }

#FORMS += \
  #  $$PWD/grawdata.ui

HEADERS += \
    $$PWD/grawdata.h \
    $$PWD/gcolordelegate.h

SOURCES += \
    $$PWD/grawdata.cpp \
    $$PWD/gcolordelegate.cpp



