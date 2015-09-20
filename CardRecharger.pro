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
    messagequeuenode.cpp

HEADERS  += cardrecharger.h \
    iccarddriver.h \
    inifile.h \
    httpclient.h \
    messagequeue.h \
    messagequeuenode.h

FORMS    += cardrecharger.ui


LIBS += -lpthread

#DEFINES += EMBEDDED_BOARD
