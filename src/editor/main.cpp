#include "mainwindow.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("AmigaGuide Editor");
    app.setApplicationVersion("0.1.0");

    MainWindow window;
    window.show();
    if (argc > 1) window.openFile(QString::fromLocal8Bit(argv[1]));
    return app.exec();
}
