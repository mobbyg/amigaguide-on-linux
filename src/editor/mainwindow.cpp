#include "mainwindow.h"

#include "amigaguide/parser.h"

#include <QAction>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QSplitter>
#include <QStatusBar>
#include <QTextCursor>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("AmigaGuide Editor");
    resize(1100, 700);

    auto* splitter = new QSplitter(this);
    nodes_ = new QListWidget(splitter);
    editor_ = new QPlainTextEdit(splitter);
    editor_->setLineWrapMode(QPlainTextEdit::NoWrap);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({260, 840});
    setCentralWidget(splitter);

    auto* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&New", this, &MainWindow::newFile, QKeySequence::New);
    fileMenu->addAction("&Open...", this, &MainWindow::open, QKeySequence::Open);
    fileMenu->addAction("&Save", this, &MainWindow::save, QKeySequence::Save);
    fileMenu->addAction("Save &As...", this, &MainWindow::saveAs, QKeySequence::SaveAs);
    fileMenu->addSeparator();
    fileMenu->addAction("E&xit", this, &QWidget::close, QKeySequence::Quit);

    auto* editMenu = menuBar()->addMenu("&Edit");
    auto* undo = new QAction("&Undo", this);
    undo->setShortcut(QKeySequence::Undo);
    connect(undo, &QAction::triggered, editor_, &QPlainTextEdit::undo);
    connect(editor_, &QPlainTextEdit::undoAvailable, undo, &QAction::setEnabled);
    undo->setEnabled(false);
    editMenu->addAction(undo);

    auto* redo = new QAction("&Redo", this);
    redo->setShortcut(QKeySequence::Redo);
    connect(redo, &QAction::triggered, editor_, &QPlainTextEdit::redo);
    connect(editor_, &QPlainTextEdit::redoAvailable, redo, &QAction::setEnabled);
    redo->setEnabled(false);
    editMenu->addAction(redo);

    editMenu->addSeparator();
    editMenu->addAction("&Cut", editor_, &QPlainTextEdit::cut, QKeySequence::Cut);
    editMenu->addAction("&Copy", editor_, &QPlainTextEdit::copy, QKeySequence::Copy);
    editMenu->addAction("&Paste", editor_, &QPlainTextEdit::paste, QKeySequence::Paste);

    connect(editor_, &QPlainTextEdit::textChanged, this, &MainWindow::updateNodes);
    connect(nodes_, &QListWidget::currentRowChanged, this, &MainWindow::nodeActivated);
    statusBar()->showMessage("Ready");
    newFile();
}

void MainWindow::newFile()
{
    if (!maybeSave()) return;
    updating_ = true;
    editor_->setPlainText("@database New AmigaGuide\n\n@node Main\n@title Main\n\n@endnode\n");
    updating_ = false;
    filePath_.clear();
    setWindowTitle("AmigaGuide Editor — Untitled");
    updateNodes();
    editor_->document()->setModified(false);
}

void MainWindow::open()
{
    const QString path = QFileDialog::getOpenFileName(this, "Open AmigaGuide", {}, "AmigaGuide (*.guide *.amigaguide);;All files (*)");
    if (!path.isEmpty()) openFile(path);
}

void MainWindow::openFile(const QString& path)
{
    if (!maybeSave()) return;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Open failed", "Could not open the selected AmigaGuide file.");
        return;
    }
    loadSource(QString::fromUtf8(file.readAll()));
    setDocumentPath(path);
    statusBar()->showMessage("Opened " + QFileInfo(path).fileName());
}

void MainWindow::loadSource(const QString& source)
{
    updating_ = true;
    editor_->setPlainText(source);
    updating_ = false;
    updateNodes();
    editor_->document()->setModified(false);
}

void MainWindow::save()
{
    if (filePath_.isEmpty()) saveAs();
    else saveTo(filePath_);
}

void MainWindow::saveAs()
{
    const QString path = QFileDialog::getSaveFileName(this, "Save AmigaGuide", filePath_.isEmpty() ? "untitled.guide" : filePath_, "AmigaGuide (*.guide);;All files (*)");
    if (!path.isEmpty()) saveTo(path);
}

bool MainWindow::saveTo(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Save failed", "Could not save the AmigaGuide file.");
        return false;
    }
    file.write(editor_->toPlainText().toUtf8());
    setDocumentPath(path);
    editor_->document()->setModified(false);
    statusBar()->showMessage("Saved " + QFileInfo(path).fileName());
    return true;
}

void MainWindow::setDocumentPath(const QString& path)
{
    filePath_ = path;
    setWindowTitle((QFileInfo(path).fileName().isEmpty() ? "Untitled" : QFileInfo(path).fileName()) + " — AmigaGuide Editor");
}

void MainWindow::updateNodes()
{
    if (updating_) return;
    const std::string source = editor_->toPlainText().toUtf8().toStdString();
    amigaguide::Document document;
    amigaguide::ParseError error;
    amigaguide::Parser parser;
    nodes_->clear();
    if (!parser.parse(source, document, &error)) {
        statusBar()->showMessage(QString("Parse error on line %1: %2").arg(error.line).arg(QString::fromStdString(error.message)));
        return;
    }
    for (const auto& node : document.nodes())
        nodes_->addItem(QString::fromStdString(node.name));
    statusBar()->showMessage(QString("%1 node(s)").arg(document.nodes().size()));
}

void MainWindow::nodeActivated(int row)
{
    if (row < 0) return;
    const std::string source = editor_->toPlainText().toUtf8().toStdString();
    amigaguide::Document document;
    if (!amigaguide::Parser{}.parse(source, document)) return;
    if (row >= static_cast<int>(document.nodes().size())) return;
    const auto& node = document.nodes()[static_cast<std::size_t>(row)];
    const int pos = static_cast<int>(node.source_begin);
    QTextCursor cursor = editor_->textCursor();
    cursor.setPosition(pos);
    editor_->setTextCursor(cursor);
    editor_->centerCursor();
}

bool MainWindow::maybeSave()
{
    if (!editor_->document()->isModified()) return true;
    const auto answer = QMessageBox::warning(this, "Unsaved changes", "The document has unsaved changes.", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (answer == QMessageBox::Save) { save(); return !editor_->document()->isModified(); }
    return answer == QMessageBox::Discard;
}
