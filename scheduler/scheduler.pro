#-------------------------------------------------
#
# Project created by QtCreator 2025-12-07T01:09:06
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = scheduler
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        models/task.cpp \
        models/user.cpp \
        models/project.cpp \
        models/reminder.cpp \
        managers/command.cpp \
        managers/remindermanager.cpp \
        ui/mainwindow.cpp \
        ui/taskeditor.cpp \
        ui/usermanager.cpp \
        ui/projectmanager.cpp \
        ui/tasklistwidget.cpp \
        data/taskrepository.cpp \
        data/userrepository.cpp \
        data/projectrepository.cpp \
        data/taskservice.cpp \
        data/strategies.cpp

HEADERS += \
        models/task.h \
        models/user.h \
        models/project.h \
        models/reminder.h \
        managers/command.h \
        managers/remindermanager.h \
        ui/mainwindow.h \
        ui/taskeditor.h \
        ui/usermanager.h \
        ui/projectmanager.h \
        ui/tasklistwidget.h \
        data/repositories.h \
        data/strategies.h \
        data/taskrepository.h \
        data/userrepository.h \
        data/projectrepository.h \
        data/taskservice.h

FORMS += \
        ui/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
