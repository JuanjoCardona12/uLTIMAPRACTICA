#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("BattlePhysics");
    app.setStyle("Fusion");

    MainWindow w;
    w.setWindowTitle("BattlePhysics — Práctica 5");
    w.show();

    return app.exec();
}
