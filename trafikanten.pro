#-------------------------------------------------
#
# Project created by QtCreator 2010-05-29T13:11:08
#
#-------------------------------------------------

QT       += core gui maemo5 network xml

TARGET = trafikanten
TEMPLATE = app


SOURCES += main.cpp\
        trafikantenwindow.cpp \
    searchwindow.cpp \
    departureswindow.cpp

HEADERS  += trafikantenwindow.h \
    searchwindow.h \
    departureswindow.h

FORMS    += trafikantenwindow.ui \
    searchwindow.ui \
    departureswindow.ui

CONFIG += mobility
MOBILITY = location bearer

symbian {
    TARGET.UID3 = 0xe9d84f35
    # TARGET.CAPABILITY += 
    TARGET.EPOCSTACKSIZE = 0x14000
    TARGET.EPOCHEAPSIZE = 0x020000 0x800000
}
