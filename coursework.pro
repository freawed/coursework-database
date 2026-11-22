QT += widgets
CONFIG += c++17
TEMPLATE = app
TARGET = coursework-qt

SOURCES += qt_main.cpp mainwindow.cpp \
           Client.cpp Date.cpp ClientDatabase.cpp ClientFile.cpp

HEADERS += mainwindow.h Client.h Date.h \
           ClientDatabase.h ClientFile.h Vector.h

FORMS += mainwindow.ui