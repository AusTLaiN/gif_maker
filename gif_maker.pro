#-------------------------------------------------
#
# Project created by QtCreator 2017-05-12T20:31:10
#
#-------------------------------------------------

QT       += core gui multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gif_maker
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

FORMS += \
    Dialogs/waitdialog.ui \
    Dialogs/makemoviedialog.ui \
    Widgets/fileoptions.ui \
    Widgets/marker.ui

SOURCES += \
    Dialogs/waitdialog.cpp \
    Widgets/myvideowidget.cpp \
    Widgets/playercontrols.cpp \
    Widgets/seekbuttons.cpp \
    Dialogs/makemoviedialog.cpp \
    Widgets/fileoptions.cpp \
    Widgets/marker.cpp \
    Widgets/durationslider.cpp \
    Base/ffmpeg.cpp \
    Base/main.cpp \
    Base/myplayer.cpp \
    Widgets/volumeslider.cpp

HEADERS  += \
    Dialogs/waitdialog.h \
    Widgets/myvideowidget.h \
    Widgets/playercontrols.h \
    Widgets/seekbuttons.h \
    Widgets/widgets.h \
    Dialogs/makemoviedialog.h \
    Widgets/fileoptions.h \
    Widgets/marker.h \
    Widgets/durationslider.h \
    Base/ffmpeg.h \
    Base/myplayer.h \
    Widgets/volumeslider.h

DISTFILES += \
    .gitignore
