#include "editorui.h"
#include "mainwindow.h"

#include "amigaguide/document_editor.h"

#include <QAction>
#include <QCheckBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenuBar>
#include <QPlainTextEdit>
#include <QScrollArea>
#include <QSpinBox>
#include <QSplitter>
#include <QStatusBar>
#include <QVBoxLayout>

namespace {
amigaguide::NodeProperty propertyFor(QObject* o){const auto n=o->objectName();if(n=="keywords")return amigaguide::NodeProperty::Keywords;if(n=="prev")return amigaguide::NodeProperty::Prev;if(n=="next")return amigaguide::NodeProperty::Next;if(n=="help")return amigaguide::NodeProperty::Help;if(n=="toc")return amigaguide::NodeProperty::Toc;if(n=="index")return amigaguide::NodeProperty::Index;if(n=="font")return amigaguide::NodeProperty::Font;return amigaguide::NodeProperty::TabWidth;}
amigaguide::NodeFlag flagFor(QObject* o){const auto n=o->objectName();if(n=="smartwrap")return amigaguide::NodeFlag::SmartWrap;if(n=="proportional")return amigaguide::NodeFlag::Proportional;return amigaguide::NodeFlag::WordWrap;}
}

EditorUi::EditorUi(MainWindow* window):window_(window){}
EditorUi::~EditorUi()=default;

void EditorUi::setup(){
    window_->setWindowTitle("AmigaGuide Editor");
    window_->resize(1100,700);

    auto* splitter=new QSplitter(window_);
    auto* left=new QWidget(splitter);
    auto* leftLayout=new QVBoxLayout(left);
    leftLayout->addWidget(new QLabel("Nodes",left));
    nodes=new QListWidget(left);
    leftLayout->addWidget(nodes);

    auto* right=new QWidget(splitter);
    auto* rightLayout=new QVBoxLayout(right);
    auto* props=new QGroupBox("Node Properties",right);
    auto* form=new QFormLayout(props);
    titleEdit=new QLineEdit(props);
    titleEdit->setPlaceholderText("Node title");
    form->addRow("Title",titleEdit);

    auto addText=[&](QLineEdit*& e,const char* name,const QString& label){
        e=new QLineEdit(props);
        e->setObjectName(name);
        form->addRow(label,e);
        QObject::connect(e,&QLineEdit::editingFinished,window_,&MainWindow::propertyEdited);
    };
    addText(keywordsEdit,"keywords","Keywords");
    addText(prevEdit,"prev","Prev");
    addText(nextEdit,"next","Next");
    addText(helpEdit,"help","Help");
    addText(tocEdit,"toc","TOC");
    addText(indexEdit,"index","Index");
    addText(fontEdit,"font","Font");

    tabWidthEdit=new QSpinBox(props);
    tabWidthEdit->setRange(0,1000);
    tabWidthEdit->setSpecialValueText("Default");
    form->addRow("Tab width",tabWidthEdit);
    QObject::connect(tabWidthEdit,&QSpinBox::editingFinished,window_,&MainWindow::propertyEdited);

    wordWrapEdit=new QCheckBox("Word wrap",props);
    wordWrapEdit->setObjectName("wordwrap");
    smartWrapEdit=new QCheckBox("Smart wrap",props);
    smartWrapEdit->setObjectName("smartwrap");
    proportionalEdit=new QCheckBox("Proportional",props);
    proportionalEdit->setObjectName("proportional");
    form->addRow("Options",wordWrapEdit);
    form->addRow("",smartWrapEdit);
    form->addRow("",proportionalEdit);
    QObject::connect(wordWrapEdit,&QCheckBox::toggled,window_,&MainWindow::flagEdited);
    QObject::connect(smartWrapEdit,&QCheckBox::toggled,window_,&MainWindow::flagEdited);
    QObject::connect(proportionalEdit,&QCheckBox::toggled,window_,&MainWindow::flagEdited);

    auto* scroll=new QScrollArea(right);
    scroll->setWidget(props);
    scroll->setWidgetResizable(true);
    scroll->setMaximumHeight(250);
    rightLayout->addWidget(scroll);
    editor=new QPlainTextEdit(right);
    editor->setLineWrapMode(QPlainTextEdit::NoWrap);
    rightLayout->addWidget(editor,1);

    splitter->addWidget(left);
    splitter->addWidget(right);
    splitter->setStretchFactor(1,1);
    splitter->setSizes({300,800});
    window_->setCentralWidget(splitter);

    auto* fileMenu=window_->menuBar()->addMenu("&File");
    auto* a=new QAction("&New",window_);a->setShortcut(QKeySequence::New);QObject::connect(a,&QAction::triggered,window_,&MainWindow::newFile);fileMenu->addAction(a);
    a=new QAction("&Open...",window_);a->setShortcut(QKeySequence::Open);QObject::connect(a,&QAction::triggered,window_,&MainWindow::open);fileMenu->addAction(a);
    a=new QAction("&Save",window_);a->setShortcut(QKeySequence::Save);QObject::connect(a,&QAction::triggered,window_,&MainWindow::save);fileMenu->addAction(a);
    a=new QAction("Save &As...",window_);a->setShortcut(QKeySequence::SaveAs);QObject::connect(a,&QAction::triggered,window_,&MainWindow::saveAs);fileMenu->addAction(a);
    a=new QAction("Document &Properties...",window_);QObject::connect(a,&QAction::triggered,window_,&MainWindow::documentProperties);fileMenu->addAction(a);
    fileMenu->addSeparator();
    a=new QAction("E&xit",window_);a->setShortcut(QKeySequence::Quit);QObject::connect(a,&QAction::triggered,window_,&QWidget::close);fileMenu->addAction(a);

    auto* editMenu=window_->menuBar()->addMenu("&Edit");
    a=new QAction("&Undo",window_);a->setShortcut(QKeySequence::Undo);QObject::connect(a,&QAction::triggered,editor,&QPlainTextEdit::undo);QObject::connect(editor,&QPlainTextEdit::undoAvailable,a,&QAction::setEnabled);a->setEnabled(false);editMenu->addAction(a);
    a=new QAction("&Redo",window_);a->setShortcut(QKeySequence::Redo);QObject::connect(a,&QAction::triggered,editor,&QPlainTextEdit::redo);QObject::connect(editor,&QPlainTextEdit::redoAvailable,a,&QAction::setEnabled);a->setEnabled(false);editMenu->addAction(a);
    editMenu->addSeparator();
    a=new QAction("&Cut",window_);a->setShortcut(QKeySequence::Cut);QObject::connect(a,&QAction::triggered,editor,&QPlainTextEdit::cut);editMenu->addAction(a);
    a=new QAction("&Copy",window_);a->setShortcut(QKeySequence::Copy);QObject::connect(a,&QAction::triggered,editor,&QPlainTextEdit::copy);editMenu->addAction(a);
    a=new QAction("&Paste",window_);a->setShortcut(QKeySequence::Paste);QObject::connect(a,&QAction::triggered,editor,&QPlainTextEdit::paste);editMenu->addAction(a);

    auto* nodeMenu=window_->menuBar()->addMenu("&Node");
    a=new QAction("&New Node...",window_);a->setShortcut(QKeySequence(Qt::CTRL|Qt::SHIFT|Qt::Key_N));QObject::connect(a,&QAction::triggered,window_,&MainWindow::newNode);nodeMenu->addAction(a);
    a=new QAction("&Rename Node...",window_);QObject::connect(a,&QAction::triggered,window_,&MainWindow::renameNode);nodeMenu->addAction(a);
    a=new QAction("&Delete Node",window_);a->setShortcut(QKeySequence::Delete);QObject::connect(a,&QAction::triggered,window_,&MainWindow::deleteNode);nodeMenu->addAction(a);

    QObject::connect(editor,&QPlainTextEdit::textChanged,window_,&MainWindow::updateNodes);
    QObject::connect(nodes,&QListWidget::currentRowChanged,window_,&MainWindow::nodeActivated);
    QObject::connect(titleEdit,&QLineEdit::editingFinished,window_,&MainWindow::titleEdited);
    window_->statusBar()->showMessage("Ready");
}
