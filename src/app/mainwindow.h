#pragma once

#include <QMainWindow>

class QTextBrowser;

class MainWindow final : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void openFile();
    void openLink(const QUrl& url);

private:
    QTextBrowser* viewer_ = nullptr;
};
