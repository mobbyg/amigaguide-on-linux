#pragma once

#include <QListWidget>
#include <QStatusBar>

class QCheckBox;
class QLineEdit;
class QPlainTextEdit;
class QSpinBox;
class MainWindow;

class EditorUi final {
public:
    explicit EditorUi(MainWindow* window);
    ~EditorUi();

    void setup();

    QPlainTextEdit* editor = nullptr;
    QListWidget* nodes = nullptr;
    QLineEdit* titleEdit = nullptr;
    QLineEdit* keywordsEdit = nullptr;
    QLineEdit* prevEdit = nullptr;
    QLineEdit* nextEdit = nullptr;
    QLineEdit* helpEdit = nullptr;
    QLineEdit* tocEdit = nullptr;
    QLineEdit* indexEdit = nullptr;
    QLineEdit* fontEdit = nullptr;
    QSpinBox* tabWidthEdit = nullptr;
    QCheckBox* wordWrapEdit = nullptr;
    QCheckBox* smartWrapEdit = nullptr;
    QCheckBox* proportionalEdit = nullptr;

private:
    MainWindow* window_ = nullptr;
};
