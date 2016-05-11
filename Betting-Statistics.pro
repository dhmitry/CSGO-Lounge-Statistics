#-------------------------------------------------
#
# Project created by QtCreator 2016-05-02T15:42:57
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = bettingstatistics
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp

HEADERS  += mainwindow.h \
    qcustomplot/qcustomplot.h \
    qcustomplot.h

FORMS    += mainwindow.ui

RC_ICONS = icon.ico
