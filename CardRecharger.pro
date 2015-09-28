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
    messagequeue.cpp \
    messagequeuenode.cpp \
    messagehandling.cpp \
    rechargerhandling.cpp \
    timestamphandling.cpp \
    cJSON.c \
    errordialog.cpp

HEADERS  += cardrecharger.h \
    iccarddriver.h \
    inifile.h \
    httpclient.h \
    messagequeue.h \
    messagequeuenode.h \
    messagehandling.h \
    rechargerhandling.h \
    timestamphandling.h \
    cJSON.h \
    errordialog.h

FORMS    += cardrecharger.ui \
    errordialog.ui


LIBS += -lpthread
LIBS += -lcurl

#DEFINES += EMBEDDED_BOARD
#DEFINES += CHINESE_OUTPUT
