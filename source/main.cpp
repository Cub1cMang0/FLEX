#include "mainwindow.h"

#include <QApplication>
#include <QScreen>
#include <QRect>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    // Maybe use this if screen size doesn't dynamically update across platforms.
    //QScreen *screen = QGuiApplication::primaryScreen();
    //QRect screen_geometry = screen->geometry();
    //w.resize(screen_geometry.width(), screen_geometry.height());
    w.show();
    return a.exec();
}
