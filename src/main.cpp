#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow w;
    w.showMaximized(); // tam ekran başlat
    return app.exec();
}
