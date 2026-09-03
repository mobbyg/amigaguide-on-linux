#pragma once

#include <QMainWindow>

class QLabel;
class QListWidget;
class QLineEdit;
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
    void newNode();
    void renameNode();
    void deleteNode();
    void titleEdited();

private:
    bool maybeSave();
    bool saveTo(const QString& path);
    void setDocumentPath(const QString& path);
    void loadSource(const QString& source);
    bool applyNodeEdit(const QString& operation, int row);
    void setNodeSelection(int row);

    QPlainTextEdit* editor_ = nullptr;
    QListWidget* nodes_ = nullptr;
    QLineEdit* titleEdit_ = nullptr;
    QLabel* nodeLabel_ = nullptr;
    QString filePath_;
    bool updating_ = false;
};
