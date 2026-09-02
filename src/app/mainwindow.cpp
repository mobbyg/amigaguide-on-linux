#include "mainwindow.h"

#include <QAction>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QStyle>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTextDocument>
#include <QToolBar>
#include <QUrl>
#include <QWidget>

#include "amigaguide/parser.h"
#include "amigaguide/renderer.h"
#include "amigaguide/search.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), viewer_(new QTextBrowser(this))
{
    setWindowTitle(QStringLiteral("AmigaGuide on Linux"));
    resize(900, 650);
    setCentralWidget(viewer_);

    auto* file_menu = menuBar()->addMenu(tr("&File"));
    auto* open_action = file_menu->addAction(tr("&Open..."));
    open_action->setShortcut(QKeySequence::Open);
    connect(open_action, &QAction::triggered, this, &MainWindow::openFile);
    file_menu->addAction(tr("E&xit"), this, &QWidget::close, QKeySequence::Quit);

    auto* edit_menu = menuBar()->addMenu(tr("&Edit"));
    auto* find_action = edit_menu->addAction(tr("&Find..."));
    find_action->setShortcut(QKeySequence::Find);
    connect(find_action, &QAction::triggered, this, [this] {
        search_box_->setFocus();
        search_box_->selectAll();
    });

    auto* navigate_menu = menuBar()->addMenu(tr("&Navigate"));
    auto* menu_back = navigate_menu->addAction(tr("&Back"));
    menu_back->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Left));
    connect(menu_back, &QAction::triggered, this, &MainWindow::navigateBack);
    auto* menu_forward = navigate_menu->addAction(tr("&Forward"));
    menu_forward->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Right));
    connect(menu_forward, &QAction::triggered, this, &MainWindow::navigateForward);
    auto* menu_home = navigate_menu->addAction(tr("&Home"));
    menu_home->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Home));
    connect(menu_home, &QAction::triggered, this, &MainWindow::navigateHome);

    auto* toolbar = addToolBar(tr("Navigation"));
    toolbar->setMovable(false);
    back_action_ = toolbar->addAction(style()->standardIcon(QStyle::SP_ArrowBack), tr("Back"));
    back_action_->setToolTip(tr("Back (Alt+Left)"));
    connect(back_action_, &QAction::triggered, this, &MainWindow::navigateBack);
    forward_action_ = toolbar->addAction(style()->standardIcon(QStyle::SP_ArrowForward), tr("Forward"));
    forward_action_->setToolTip(tr("Forward (Alt+Right)"));
    connect(forward_action_, &QAction::triggered, this, &MainWindow::navigateForward);
    home_action_ = toolbar->addAction(style()->standardIcon(QStyle::SP_DirHomeIcon), tr("Home"));
    home_action_->setToolTip(tr("Home (Alt+Home)"));
    connect(home_action_, &QAction::triggered, this, &MainWindow::navigateHome);
    toolbar->addSeparator();

    auto* search_widget = new QWidget(toolbar);
    auto* search_layout = new QHBoxLayout(search_widget);
    search_layout->setContentsMargins(4, 0, 4, 0);
    search_layout->setSpacing(4);
    auto* search_label = new QLabel(tr("Search:"), search_widget);
    search_box_ = new QLineEdit(search_widget);
    search_box_->setPlaceholderText(tr("Search this guide..."));
    search_box_->setClearButtonEnabled(true);
    search_box_->setMinimumWidth(220);
    search_status_ = new QLabel(search_widget);
    search_status_->setMinimumWidth(65);
    search_layout->addWidget(search_label);
    search_layout->addWidget(search_box_);
    search_layout->addWidget(search_status_);
    toolbar->addWidget(search_widget);

    connect(search_box_, &QLineEdit::textChanged, this, &MainWindow::searchText);
    connect(search_box_, &QLineEdit::returnPressed, this, &MainWindow::searchNext);

    connect(viewer_, &QTextBrowser::anchorClicked, this, &MainWindow::openLink);
    viewer_->setOpenExternalLinks(false);
    viewer_->setOpenLinks(false);
    viewer_->setPlaceholderText(tr("Open an AmigaGuide file to begin."));
    updateNavigationActions();
}

void MainWindow::openFile()
{
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Open AmigaGuide"), QString(),
        tr("AmigaGuide files (*.guide *.Guide);;Text files (*.txt);;All files (*)"));
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Open failed"), file.errorString());
        return;
    }

    const QByteArray bytes = file.readAll();
    amigaguide::Document parsed_document;
    amigaguide::ParseError error;
    amigaguide::Parser parser;
    if (!parser.parse(bytes.toStdString(), parsed_document, &error)) {
        QMessageBox::critical(this, tr("Parse failed"),
                              tr("Line %1: %2").arg(error.line).arg(QString::fromStdString(error.message)));
        return;
    }

    document_ = std::move(parsed_document);

    QString html;
    html += QStringLiteral("<html><head><style>");
    html += QStringLiteral("body{font-family:sans-serif;margin:18px;}h1{margin-bottom:12px;}a{font-weight:600;}p.meta{color:#666;}");
    html += QStringLiteral("</style></head><body>");
    html += QStringLiteral("<p class=\"meta\"><b>Database:</b> %1 &nbsp; <b>Nodes:</b> %2</p>")
                .arg(QString::fromStdString(document_.metadata().name).toHtmlEscaped())
                .arg(document_.nodes().size());

    for (const auto& node : document_.nodes()) {
        html += QString::fromStdString(amigaguide::render_node_html(document_, node));
        html += QStringLiteral("<p>");
        if (!node.prev.empty()) {
            html += QStringLiteral("<a href=\"node:%1\">&lt; Previous</a>")
                        .arg(QString::fromStdString(node.prev).toHtmlEscaped());
            html += QStringLiteral(" &nbsp; ");
        }
        if (!node.next.empty()) {
            html += QStringLiteral("<a href=\"node:%1\">Next &gt;</a>")
                        .arg(QString::fromStdString(node.next).toHtmlEscaped());
        }
        html += QStringLiteral("</p>");
    }
    html += QStringLiteral("</body></html>");

    viewer_->setHtml(html);
    search_box_->clear();
    navigation_history_.clear();

    if (!document_.metadata().toc.empty() && document_.find_node(document_.metadata().toc)) {
        home_node_ = QString::fromStdString(document_.metadata().toc);
    } else if (!document_.nodes().empty()) {
        home_node_ = QString::fromStdString(document_.nodes().front().name);
    } else {
        home_node_.clear();
    }

    if (!home_node_.isEmpty()) navigateToNode(home_node_);
    updateSearchStatus();
    updateNavigationActions();
    setWindowTitle(QStringLiteral("%1 — AmigaGuide on Linux").arg(QFileInfo(path).fileName()));
}

void MainWindow::openLink(const QUrl& url)
{
    if (url.scheme() != QStringLiteral("node")) return;
    const QString target = url.path().isEmpty() ? url.host() : url.path();
    navigateToNode(target);
}

void MainWindow::navigateToNode(const QString& node, bool add_history)
{
    if (node.isEmpty()) return;
    const QString target = node.startsWith(QLatin1Char('/')) ? node.mid(1) : node;
    if (!document_.find_node(target.toStdString())) return;

    if (add_history) navigation_history_.visit(target.toStdString());

    viewer_->scrollToAnchor(target);
    updateNavigationActions();
}

void MainWindow::navigateBack()
{
    if (!navigation_history_.back()) return;
    viewer_->scrollToAnchor(QString::fromStdString(navigation_history_.current()));
    updateNavigationActions();
}

void MainWindow::navigateForward()
{
    if (!navigation_history_.forward()) return;
    viewer_->scrollToAnchor(QString::fromStdString(navigation_history_.current()));
    updateNavigationActions();
}

void MainWindow::navigateHome()
{
    navigateToNode(home_node_);
}

void MainWindow::searchText()
{
    if (search_box_->text().isEmpty()) {
        viewer_->moveCursor(QTextCursor::Start);
        updateSearchStatus();
        return;
    }

    viewer_->moveCursor(QTextCursor::Start);
    viewer_->find(search_box_->text());
    updateSearchStatus();
}

void MainWindow::searchNext()
{
    if (search_box_->text().isEmpty()) return;
    if (!viewer_->find(search_box_->text())) {
        viewer_->moveCursor(QTextCursor::Start);
        viewer_->find(search_box_->text());
    }
    updateSearchStatus();
}

void MainWindow::searchPrevious()
{
    if (search_box_->text().isEmpty()) return;
    if (!viewer_->find(search_box_->text(), QTextDocument::FindBackward)) {
        viewer_->moveCursor(QTextCursor::End);
        viewer_->find(search_box_->text(), QTextDocument::FindBackward);
    }
    updateSearchStatus();
}

void MainWindow::updateSearchStatus()
{
    if (search_box_->text().isEmpty()) {
        search_status_->clear();
        return;
    }

    amigaguide::SearchEngine engine;
    const auto matches = engine.find(document_, search_box_->text().toStdString());
    if (matches.empty()) {
        search_status_->setText(tr("No matches"));
        return;
    }

    search_status_->setText(tr("%1 match%2")
                                .arg(matches.size())
                                .arg(matches.size() == 1 ? QString() : QStringLiteral("es")));
}

void MainWindow::updateNavigationActions()
{
    const bool can_back = navigation_history_.can_back();
    const bool can_forward = navigation_history_.can_forward();
    if (back_action_) back_action_->setEnabled(can_back);
    if (forward_action_) forward_action_->setEnabled(can_forward);
    if (home_action_) home_action_->setEnabled(!home_node_.isEmpty());
}
