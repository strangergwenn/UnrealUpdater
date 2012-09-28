#-------------------------------------------------
#
# Project created by QtCreator 2012-09-17T23:28:45
#
#-------------------------------------------------

QT       += core gui
QT       += network
QT       += xml

TARGET = UnrealUpdater
TEMPLATE = app


SOURCES += main.cpp\
    about.cpp \
    downloader.cpp \
    updater.cpp

HEADERS  += \
    about.h \
    common.h \
    downloader.h \
    updater.h \
    Res/project.h

FORMS    += \
    about.ui \
    updater.ui

RESOURCES += \
    UnrealUpdater.qrc

RC_FILE = UnrealUpdater.rc

CONFIG += static
static {
    DEFINES += STATIC
}

OTHER_FILES += \
    UnrealUpdater.rc
