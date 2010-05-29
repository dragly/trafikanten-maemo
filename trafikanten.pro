#-------------------------------------------------
#
# Project created by QtCreator 2010-05-29T13:11:08
#
#-------------------------------------------------

QT       += core gui

TARGET = trafikanten
TEMPLATE = app


SOURCES += main.cpp\
        trafikantenwindow.cpp

HEADERS  += trafikantenwindow.h

FORMS    += trafikantenwindow.ui

CONFIG += mobility
MOBILITY = 

symbian {
    TARGET.UID3 = 0xe9d84f35
    # TARGET.CAPABILITY += 
    TARGET.EPOCSTACKSIZE = 0x14000
    TARGET.EPOCHEAPSIZE = 0x020000 0x800000
}
