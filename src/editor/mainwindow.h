#pragma once

#include <QMainWindow>

class QCheckBox;
class QLineEdit;
class QListWidget;
class QPlainTextEdit;
class QSpinBox;

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    void openFile(const QString& path);
private slots:
    void newFile(); void open(); void save(); void saveAs(); void updateNodes(); void nodeActivated(int row);
    void newNode(); void renameNode(); void deleteNode(); void titleEdited(); void propertyEdited(); void flagEdited(bool enabled);
    void documentProperties();
private:
    bool maybeSave(); bool saveTo(const QString& path); void setDocumentPath(const QString& path); void loadSource(const QString& source);
    bool applyNodeEdit(const QString& operation, int row); void setNodeSelection(int row); void refreshProperties(int row);
    void replaceSource(const std::string& source, int row, const QString& message);
    QPlainTextEdit* editor_ = nullptr; QListWidget* nodes_ = nullptr; QLineEdit* titleEdit_ = nullptr;
    QLineEdit* keywordsEdit_ = nullptr; QLineEdit* prevEdit_ = nullptr; QLineEdit* nextEdit_ = nullptr; QLineEdit* helpEdit_ = nullptr;
    QLineEdit* tocEdit_ = nullptr; QLineEdit* indexEdit_ = nullptr; QLineEdit* fontEdit_ = nullptr; QSpinBox* tabWidthEdit_ = nullptr;
    QCheckBox* wordWrapEdit_ = nullptr; QCheckBox* smartWrapEdit_ = nullptr; QCheckBox* proportionalEdit_ = nullptr;
    QString filePath_; bool updating_ = false;
};
