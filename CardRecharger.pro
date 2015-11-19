#-------------------------------------------------
#
# Project created by QtCreator 2015-08-28T11:32:55
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CardRecharger
TEMPLATE = app


SOURCES += main.cpp\
        cardrecharger.cpp \
    iccarddriver.cpp \
    inifile.c \
    httpclient.cpp \
    rechargerhandling.cpp \
    timestamphandling.cpp \
    cJSON.c \
    errordialog.cpp \
    advertisementhandling.cpp

HEADERS  += cardrecharger.h \
    iccarddriver.h \
    inifile.h \
    httpclient.h \
    rechargerhandling.h \
    timestamphandling.h \
    cJSON.h \
    errordialog.h \
    advertisementhandling.h

FORMS    += cardrecharger.ui \
    errordialog.ui


LIBS += -lpthread
LIBS += -lcurl
LIBS += -lqrencode

DEFINES += CURL_STATICLIB

#DEFINES += EMBEDDED_BOARD
#DEFINES += CHINESE_OUTPUT
