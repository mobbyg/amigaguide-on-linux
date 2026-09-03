#include <QApplication>

#include "mainwindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("AmigaGuide on Linux"));
    app.setApplicationName(QStringLiteral("AmigaGuide on Linux"));

    MainWindow window;
    window.show();
    return app.exec();
}
