#pragma once

#include <QMainWindow>
#include <memory>

class EditorUi;

class MainWindow final : public QMainWindow {
    Q_OBJECT
    friend class EditorUi;
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;
    void openFile(const QString& path);
private slots:
    void newFile(); void open(); void save(); void saveAs(); void updateNodes(); void nodeActivated(int row);
    void newNode(); void renameNode(); void deleteNode(); void titleEdited(); void propertyEdited(); void flagEdited(bool enabled);
    void documentProperties();
private:
    bool maybeSave(); bool saveTo(const QString& path); void setDocumentPath(const QString& path); void loadSource(const QString& source);
    bool applyNodeEdit(const QString& operation, int row); void setNodeSelection(int row); void refreshProperties(int row);
    void replaceSource(const std::string& source, int row, const QString& message);

    std::unique_ptr<EditorUi> ui_;
    QString filePath_;
    bool updating_ = false;
};
