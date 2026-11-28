#include "mainwindow.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(QStringLiteral(":/app.ico")));
    MainWindow w;
    w.resize(1150, 650);
    w.show();
    return QApplication::exec();
}
