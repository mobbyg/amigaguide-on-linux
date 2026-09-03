#pragma once

#include <QMainWindow>

class QListWidget;
class QPlainTextEdit;

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

    void openFile(const QString& path);

private slots:
    void newFile();
    void open();
    void save();
    void saveAs();
    void updateNodes();
    void nodeActivated(int row);

private:
    bool maybeSave();
    bool saveTo(const QString& path);
    void setDocumentPath(const QString& path);
    void loadSource(const QString& source);

    QPlainTextEdit* editor_ = nullptr;
    QListWidget* nodes_ = nullptr;
    QString filePath_;
    bool updating_ = false;
};
