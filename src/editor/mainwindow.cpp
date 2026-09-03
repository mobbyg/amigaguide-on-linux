#include "mainwindow.h"

#include "amigaguide/document_editor.h"
#include "amigaguide/parser.h"

#include <algorithm>
#include <QAction>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QSplitter>
#include <QStatusBar>
#include <QTextCursor>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("AmigaGuide Editor");
    resize(1100, 700);

    auto* splitter = new QSplitter(this);
    auto* leftPanel = new QWidget(splitter);
    auto* leftLayout = new QVBoxLayout(leftPanel);
    nodeLabel_ = new QLabel("Node", leftPanel);
    titleEdit_ = new QLineEdit(leftPanel);
    titleEdit_->setPlaceholderText("Node title");
    nodes_ = new QListWidget(leftPanel);
    leftLayout->addWidget(nodeLabel_);
    leftLayout->addWidget(titleEdit_);
    leftLayout->addWidget(nodes_, 1);

    editor_ = new QPlainTextEdit(splitter);
    editor_->setLineWrapMode(QPlainTextEdit::NoWrap);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({300, 800});
    setCentralWidget(splitter);

    auto* fileMenu = menuBar()->addMenu("&File");
    auto* newAction = new QAction("&New", this);
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::newFile);
    fileMenu->addAction(newAction);

    auto* openAction = new QAction("&Open...", this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::open);
    fileMenu->addAction(openAction);

    auto* saveAction = new QAction("&Save", this);
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::save);
    fileMenu->addAction(saveAction);

    auto* saveAsAction = new QAction("Save &As...", this);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveAs);
    fileMenu->addAction(saveAsAction);

    fileMenu->addSeparator();
    auto* exitAction = new QAction("E&xit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(exitAction);

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
    auto* cutAction = new QAction("&Cut", this);
    cutAction->setShortcut(QKeySequence::Cut);
    connect(cutAction, &QAction::triggered, editor_, &QPlainTextEdit::cut);
    editMenu->addAction(cutAction);

    auto* copyAction = new QAction("&Copy", this);
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, editor_, &QPlainTextEdit::copy);
    editMenu->addAction(copyAction);

    auto* pasteAction = new QAction("&Paste", this);
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(pasteAction, &QAction::triggered, editor_, &QPlainTextEdit::paste);
    editMenu->addAction(pasteAction);

    auto* nodeMenu = menuBar()->addMenu("&Node");
    auto* newNodeAction = new QAction("&New Node...", this);
    newNodeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N));
    connect(newNodeAction, &QAction::triggered, this, &MainWindow::newNode);
    nodeMenu->addAction(newNodeAction);

    auto* renameNodeAction = new QAction("&Rename Node...", this);
    connect(renameNodeAction, &QAction::triggered, this, &MainWindow::renameNode);
    nodeMenu->addAction(renameNodeAction);

    auto* deleteNodeAction = new QAction("&Delete Node", this);
    deleteNodeAction->setShortcut(QKeySequence::Delete);
    connect(deleteNodeAction, &QAction::triggered, this, &MainWindow::deleteNode);
    nodeMenu->addAction(deleteNodeAction);

    connect(editor_, &QPlainTextEdit::textChanged, this, &MainWindow::updateNodes);
    connect(nodes_, &QListWidget::currentRowChanged, this, &MainWindow::nodeActivated);
    connect(titleEdit_, &QLineEdit::editingFinished, this, &MainWindow::titleEdited);
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
    const QString selectedName = nodes_->currentItem() ? nodes_->currentItem()->text() : QString{};
    const int oldRow = nodes_->currentRow();
    const std::string source = editor_->toPlainText().toUtf8().toStdString();
    amigaguide::Document document;
    amigaguide::ParseError error;
    if (!amigaguide::Parser{}.parse(source, document, &error)) {
        nodes_->clear();
        titleEdit_->clear();
        nodeLabel_->setText("Node");
        statusBar()->showMessage(QString("Parse error on line %1: %2").arg(error.line).arg(QString::fromStdString(error.message)));
        return;
    }

    updating_ = true;
    nodes_->clear();
    int selectedRow = -1;
    for (std::size_t i = 0; i < document.nodes().size(); ++i) {
        const auto& node = document.nodes()[i];
        nodes_->addItem(QString::fromStdString(node.name));
        if (!selectedName.isEmpty() && selectedName == QString::fromStdString(node.name))
            selectedRow = static_cast<int>(i);
    }
    if (selectedRow < 0 && oldRow >= 0 && oldRow < nodes_->count()) selectedRow = oldRow;
    if (selectedRow >= 0) nodes_->setCurrentRow(selectedRow);
    updating_ = false;
    nodeActivated(nodes_->currentRow());
    statusBar()->showMessage(QString("%1 node(s)").arg(document.nodes().size()));
}

void MainWindow::nodeActivated(int row)
{
    if (row < 0) {
        titleEdit_->clear();
        nodeLabel_->setText("Node");
        return;
    }
    const std::string source = editor_->toPlainText().toUtf8().toStdString();
    amigaguide::Document document;
    if (!amigaguide::Parser{}.parse(source, document)) return;
    if (row >= static_cast<int>(document.nodes().size())) return;
    const auto& node = document.nodes()[static_cast<std::size_t>(row)];
    updating_ = true;
    nodeLabel_->setText(QString::fromStdString(node.name));
    titleEdit_->setText(QString::fromStdString(node.title));
    updating_ = false;

    const QString prefix = QString::fromUtf8(source.data(), static_cast<qsizetype>(node.source_begin));
    QTextCursor cursor = editor_->textCursor();
    cursor.setPosition(prefix.size());
    editor_->setTextCursor(cursor);
    editor_->centerCursor();
}

void MainWindow::setNodeSelection(int row)
{
    if (row >= 0 && row < nodes_->count()) nodes_->setCurrentRow(row);
}

bool MainWindow::applyNodeEdit(const QString& operation, int row)
{
    const std::string source = editor_->toPlainText().toUtf8().toStdString();
    amigaguide::Document document;
    amigaguide::ParseError parseError;
    if (!amigaguide::Parser{}.parse(source, document, &parseError)) {
        QMessageBox::warning(this, "Cannot edit node", QString("The document must parse before a node can be %1.").arg(operation));
        return false;
    }
    if (row < 0 || row >= static_cast<int>(document.nodes().size())) return false;
    return true;
}

void MainWindow::newNode()
{
    bool ok = false;
    const QString name = QInputDialog::getText(this, "New Node", "Node name:", QLineEdit::Normal, "NewNode", &ok);
    if (!ok) return;
    const QString title = QInputDialog::getText(this, "New Node", "Node title:", QLineEdit::Normal, name, &ok);
    if (!ok) return;

    auto document = amigaguide::Document{};
    amigaguide::ParseError parseError;
    if (!amigaguide::Parser{}.parse(editor_->toPlainText().toUtf8().toStdString(), document, &parseError)) {
        QMessageBox::warning(this, "Cannot add node", "Fix the document's parse errors before adding a node.");
        return;
    }
    amigaguide::DocumentEditor edit(document);
    std::string error;
    if (!edit.add_node(name.toUtf8().toStdString(), title.toUtf8().toStdString(), &error)) {
        QMessageBox::warning(this, "Cannot add node", QString::fromStdString(error));
        return;
    }

    updating_ = true;
    editor_->setPlainText(QString::fromUtf8(document.source().data(), static_cast<qsizetype>(document.source().size())));
    updating_ = false;
    editor_->document()->setModified(true);
    updateNodes();
    setNodeSelection(nodes_->count() - 1);
    statusBar()->showMessage("Added node " + name);
}

void MainWindow::renameNode()
{
    const int row = nodes_->currentRow();
    if (!applyNodeEdit("rename", row)) return;
    const QString current = nodes_->currentItem()->text();
    bool ok = false;
    const QString name = QInputDialog::getText(this, "Rename Node", "Node name:", QLineEdit::Normal, current, &ok);
    if (!ok || name == current) return;

    amigaguide::Document document;
    if (!amigaguide::Parser{}.parse(editor_->toPlainText().toUtf8().toStdString(), document)) return;
    amigaguide::DocumentEditor edit(document);
    std::string error;
    if (!edit.rename_node(static_cast<std::size_t>(row), name.toUtf8().toStdString(), &error)) {
        QMessageBox::warning(this, "Cannot rename node", QString::fromStdString(error));
        return;
    }
    updating_ = true;
    editor_->setPlainText(QString::fromUtf8(document.source().data(), static_cast<qsizetype>(document.source().size())));
    updating_ = false;
    editor_->document()->setModified(true);
    updateNodes();
    setNodeSelection(row);
    statusBar()->showMessage("Renamed node to " + name);
}

void MainWindow::deleteNode()
{
    const int row = nodes_->currentRow();
    if (!applyNodeEdit("delete", row)) return;
    const QString name = nodes_->currentItem()->text();
    if (QMessageBox::question(this, "Delete Node", "Delete node \"" + name + "\"? This removes its source block but does not rewrite links to it.", QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;

    amigaguide::Document document;
    if (!amigaguide::Parser{}.parse(editor_->toPlainText().toUtf8().toStdString(), document)) return;
    amigaguide::DocumentEditor edit(document);
    std::string error;
    if (!edit.remove_node(static_cast<std::size_t>(row), &error)) {
        QMessageBox::warning(this, "Cannot delete node", QString::fromStdString(error));
        return;
    }
    updating_ = true;
    editor_->setPlainText(QString::fromUtf8(document.source().data(), static_cast<qsizetype>(document.source().size())));
    updating_ = false;
    editor_->document()->setModified(true);
    updateNodes();
    setNodeSelection(std::min(row, nodes_->count() - 1));
    statusBar()->showMessage("Deleted node " + name);
}

void MainWindow::titleEdited()
{
    if (updating_) return;
    const int row = nodes_->currentRow();
    if (!applyNodeEdit("change its title", row)) return;

    amigaguide::Document document;
    if (!amigaguide::Parser{}.parse(editor_->toPlainText().toUtf8().toStdString(), document)) return;
    amigaguide::DocumentEditor edit(document);
    std::string error;
    const QString title = titleEdit_->text();
    if (!edit.set_node_title(static_cast<std::size_t>(row), title.toUtf8().toStdString(), &error)) {
        QMessageBox::warning(this, "Cannot change title", QString::fromStdString(error));
        return;
    }
    updating_ = true;
    editor_->setPlainText(QString::fromUtf8(document.source().data(), static_cast<qsizetype>(document.source().size())));
    updating_ = false;
    editor_->document()->setModified(true);
    updateNodes();
    setNodeSelection(row);
    statusBar()->showMessage("Updated node title");
}

bool MainWindow::maybeSave()
{
    if (!editor_->document()->isModified()) return true;
    const auto answer = QMessageBox::warning(this, "Unsaved changes", "The document has unsaved changes.", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (answer == QMessageBox::Save) { save(); return !editor_->document()->isModified(); }
    return answer == QMessageBox::Discard;
}
