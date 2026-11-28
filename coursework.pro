QT += widgets
CONFIG += c++17
TEMPLATE = app
TARGET = coursework-qt
RC_ICONS = app.ico

app_icon.files = app.ico
app_icon.prefix = /
RESOURCES += app_icon

SOURCES += qt_main.cpp mainwindow.cpp \
           Client.cpp Date.cpp ClientDatabase.cpp ClientFile.cpp \
           clientdialog.cpp

HEADERS += mainwindow.h Client.h Date.h \
           ClientDatabase.h ClientFile.h Vector.h \
           clientdialog.h

FORMS += mainwindow.ui \
    clientdialog.ui

DEFINES += $$shell_quote(COURSEWORK_PROJECT_DIR=\"$$PWD\")
