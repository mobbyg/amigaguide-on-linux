#include "mainwindow.h"

#include <QAction>
#include <QActionGroup>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QSettings>
#include <QStatusBar>
#include <QStyle>
#include <QTabWidget>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTextDocument>
#include <QToolBar>
#include <QUrl>
#include <QWidget>

#include "amigaguide/destination_resolver.h"
#include "amigaguide/parser.h"
#include "amigaguide/renderer.h"
#include "amigaguide/search.h"

namespace {
constexpr auto kCrossDocumentLinksKey = "crossDocumentLinks";
constexpr auto kNewTabValue = "newTab";
constexpr auto kNewWindowValue = "newWindow";
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), tabs_(new QTabWidget(this))
{
    setWindowTitle(QStringLiteral("AmigaGuide on Linux"));
    resize(900, 650);
    setCentralWidget(tabs_);
    tabs_->setTabsClosable(false);
    connect(tabs_, &QTabWidget::currentChanged, this, &MainWindow::tabChanged);

    auto* file_menu = menuBar()->addMenu(tr("&File"));
    auto* open_action = file_menu->addAction(tr("&Open..."));
    open_action->setShortcut(QKeySequence::Open);
    connect(open_action, &QAction::triggered, this, &MainWindow::openFile);

    new_tab_action_ = file_menu->addAction(tr("New &Tab"));
    new_tab_action_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
    connect(new_tab_action_, &QAction::triggered, this, [this] { createTab(true); });

    new_window_action_ = file_menu->addAction(tr("New &Window"));
    new_window_action_->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N));
    connect(new_window_action_, &QAction::triggered, this, [this] {
        auto* window = new MainWindow;
        window->setAttribute(Qt::WA_DeleteOnClose);
        window->show();
    });

    file_menu->addSeparator();
    auto* exit_action = new QAction(tr("E&xit"), this);
    exit_action->setShortcut(QKeySequence::Quit);
    connect(exit_action, &QAction::triggered, this, &QWidget::close);
    file_menu->addAction(exit_action);

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

    auto* settings_menu = menuBar()->addMenu(tr("&Settings"));
    auto* cross_document_menu = settings_menu->addMenu(tr("Cross-document links"));
    cross_document_group_ = new QActionGroup(this);
    cross_document_group_->setExclusive(true);

    auto* tab_action = cross_document_menu->addAction(tr("New tab"));
    tab_action->setCheckable(true);
    cross_document_group_->addAction(tab_action);
    connect(tab_action, &QAction::triggered, this, &MainWindow::setCrossDocumentLinksInTab);

    auto* window_action = cross_document_menu->addAction(tr("New window"));
    window_action->setCheckable(true);
    cross_document_group_->addAction(window_action);
    connect(window_action, &QAction::triggered, this, &MainWindow::setCrossDocumentLinksInWindow);

    const QSettings settings;
    const QString saved_mode = settings.value(kCrossDocumentLinksKey, kNewTabValue).toString();
    if (saved_mode == QLatin1String(kNewWindowValue)) window_action->setChecked(true);
    else tab_action->setChecked(true);

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

    createTab(true);
    updateNavigationActions();
}

MainWindow::ViewState* MainWindow::currentView()
{
    const int index = tabs_ ? tabs_->currentIndex() : -1;
    if (index < 0 || index >= static_cast<int>(views_.size())) return nullptr;
    return &views_[static_cast<std::size_t>(index)];
}

const MainWindow::ViewState* MainWindow::currentView() const
{
    const int index = tabs_ ? tabs_->currentIndex() : -1;
    if (index < 0 || index >= static_cast<int>(views_.size())) return nullptr;
    return &views_[static_cast<std::size_t>(index)];
}

MainWindow::ViewState* MainWindow::createTab(bool switch_to)
{
    ViewState view;
    view.viewer = new QTextBrowser(tabs_);
    view.viewer->setOpenExternalLinks(false);
    view.viewer->setOpenLinks(false);
    view.viewer->setPlaceholderText(tr("Open an AmigaGuide file to begin."));
    connect(view.viewer, &QTextBrowser::anchorClicked, this, &MainWindow::openLink);

    views_.push_back(std::move(view));
    const int index = tabs_->addTab(views_.back().viewer, tr("New Tab"));
    tabs_->setTabToolTip(index, tr("No document loaded"));
    if (switch_to) tabs_->setCurrentIndex(index);
    return &views_.back();
}

void MainWindow::openFile()
{
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Open AmigaGuide"), QString(),
        tr("AmigaGuide files (*.guide *.Guide);;Text files (*.txt);;All files (*)"));
    if (path.isEmpty()) return;

    auto* view = currentView();
    if (!view) view = createTab(true);
    loadDocumentFile(*view, path, QString(), true);
}

bool MainWindow::loadDocumentFile(ViewState& view, const QString& path, const QString& requested_node, bool add_history)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        statusBar()->showMessage(tr("Open failed: %1").arg(file.errorString()), 5000);
        return false;
    }

    const QByteArray bytes = file.readAll();
    amigaguide::Document parsed_document;
    amigaguide::ParseError error;
    amigaguide::Parser parser;
    if (!parser.parse(bytes.toStdString(), parsed_document, &error)) {
        statusBar()->showMessage(
            tr("Parse failed — line %1: %2")
                .arg(error.line)
                .arg(QString::fromStdString(error.message)),
            5000);
        return false;
    }

    view.document = std::move(parsed_document);
    view.current_document_path = QFileInfo(path).absoluteFilePath();
    renderDocument(view);
    search_box_->clear();

    if (!view.document.metadata().toc.empty() && view.document.find_node(view.document.metadata().toc)) {
        view.home_node = QString::fromStdString(view.document.metadata().toc);
    } else if (!view.document.nodes().empty()) {
        view.home_node = QString::fromStdString(view.document.nodes().front().name);
    } else {
        view.home_node.clear();
    }

    QString target_node = requested_node;
    if (target_node.isEmpty() || !view.document.find_node(target_node.toStdString())) {
        target_node = view.home_node;
    }

    if (!target_node.isEmpty()) {
        view.viewer->scrollToAnchor(target_node);
        if (add_history) view.navigation_history.visit(currentNodeDestination(view, target_node).uri());
    } else if (add_history) {
        view.navigation_history.visit(QUrl::fromLocalFile(view.current_document_path).toString(QUrl::FullyEncoded).toStdString());
    }

    const int index = static_cast<int>(&view - views_.data());
    updateTabTitle(index);
    updateSearchStatus();
    updateNavigationActions();
    if (index == tabs_->currentIndex()) {
        setWindowTitle(QStringLiteral("%1 — AmigaGuide on Linux").arg(QFileInfo(path).fileName()));
    }
    return true;
}

void MainWindow::renderDocument(ViewState& view)
{
    QString html;
    html += QStringLiteral("<html><head><style>");
    html += QStringLiteral("body{font-family:sans-serif;margin:18px;}h1{margin-bottom:12px;}a{font-weight:600;}p.meta{color:#666;}");
    html += QStringLiteral("</style></head><body>");

    for (const auto& node : view.document.nodes()) {
        html += QString::fromStdString(amigaguide::render_node_html(view.document, node));
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

    view.viewer->setHtml(html);
}

amigaguide::Destination MainWindow::currentNodeDestination(const ViewState& view, const QString& node) const
{
    if (view.current_document_path.isEmpty()) {
        return amigaguide::Destination::node(node.toStdString());
    }

    QUrl url = QUrl::fromLocalFile(view.current_document_path);
    url.setFragment(node);
    return amigaguide::Destination::parse(url.toString(QUrl::FullyEncoded).toStdString());
}

void MainWindow::openLink(const QUrl& url)
{
    const QString raw_link = url.toString();
    auto destination = amigaguide::Destination::parse(raw_link.toStdString());

    if (raw_link.startsWith(QStringLiteral("node:"), Qt::CaseInsensitive)) {
        const QString nested_link = raw_link.mid(5);
        const auto nested = amigaguide::Destination::parse(nested_link.toStdString());
        if (nested.valid() && nested.type() != amigaguide::DestinationType::Node) destination = nested;
    }

    if (!destination.valid()) {
        statusBar()->showMessage(tr("Unsupported navigation target"), 3000);
        return;
    }
    navigateToDestination(destination);
}

void MainWindow::navigateToNode(const QString& node, bool add_history)
{
    auto* view = currentView();
    if (!view || node.isEmpty()) return;
    navigateToDestination(currentNodeDestination(*view, node), add_history);
}

void MainWindow::navigateToDestination(const amigaguide::Destination& destination, bool add_history)
{
    auto* view = currentView();
    if (!view || !destination.valid()) return;

    amigaguide::LocalDestinationResolver resolver;
    const auto resolution = resolver.resolve(destination);

    switch (resolution.kind) {
    case amigaguide::ResolutionKind::InternalNode: {
        const QString target = QString::fromStdString(resolution.value);
        if (target.isEmpty() || !view->document.find_node(resolution.value)) return;
        if (add_history) view->navigation_history.visit(currentNodeDestination(*view, target).uri());
        view->viewer->scrollToAnchor(target);
        updateNavigationActions();
        return;
    }
    case amigaguide::ResolutionKind::LocalFile: {
        QUrl target_url(QString::fromStdString(resolution.value));
        if (!target_url.isValid() || target_url.scheme().compare(QStringLiteral("file"), Qt::CaseInsensitive) != 0) {
            statusBar()->showMessage(tr("Invalid local file target"), 3000);
            return;
        }
        if (target_url.isRelative() && !view->current_document_path.isEmpty()) {
            target_url = QUrl::fromLocalFile(view->current_document_path).resolved(target_url);
        }

        const QString path = target_url.toLocalFile();
        if (path.isEmpty()) {
            statusBar()->showMessage(tr("Invalid local file target"), 3000);
            return;
        }
        const QString node = target_url.fragment(QUrl::FullyDecoded);
        const QString current_path = QFileInfo(view->current_document_path).absoluteFilePath();
        const QString target_path = QFileInfo(path).absoluteFilePath();
        const Qt::CaseSensitivity sensitivity =
#ifdef Q_OS_WIN
            Qt::CaseInsensitive;
#else
            Qt::CaseSensitive;
#endif
        const bool different_document = current_path.isEmpty() || current_path.compare(target_path, sensitivity) != 0;

        if (add_history && different_document) {
            if (crossDocumentMode() == CrossDocumentMode::NewWindow) {
                auto* window = new MainWindow;
                window->setAttribute(Qt::WA_DeleteOnClose);
                window->show();
                if (auto* new_view = window->currentView()) {
                    window->loadDocumentFile(*new_view, path, node, true);
                }
            } else {
                auto* new_view = createTab(true);
                loadDocumentFile(*new_view, path, node, true);
            }
            return;
        }

        loadDocumentFile(*view, path, node, add_history);
        return;
    }
    case amigaguide::ResolutionKind::RemoteHttp:
        statusBar()->showMessage(tr("Remote document navigation is not implemented yet"), 3000);
        return;
    case amigaguide::ResolutionKind::LibraryDocument:
        statusBar()->showMessage(tr("AmigaGuide Library navigation is not implemented yet"), 3000);
        return;
    case amigaguide::ResolutionKind::Invalid:
        statusBar()->showMessage(tr("Unsupported navigation target"), 3000);
        return;
    }
}

void MainWindow::navigateBack()
{
    auto* view = currentView();
    if (!view || !view->navigation_history.back()) return;
    navigateToDestination(amigaguide::Destination::parse(view->navigation_history.current()), false);
}

void MainWindow::navigateForward()
{
    auto* view = currentView();
    if (!view || !view->navigation_history.forward()) return;
    navigateToDestination(amigaguide::Destination::parse(view->navigation_history.current()), false);
}

void MainWindow::navigateHome()
{
    auto* view = currentView();
    if (view) navigateToNode(view->home_node);
}

void MainWindow::searchText()
{
    auto* view = currentView();
    if (!view) return;
    if (search_box_->text().isEmpty()) {
        view->viewer->moveCursor(QTextCursor::Start);
        updateSearchStatus();
        return;
    }
    view->viewer->moveCursor(QTextCursor::Start);
    view->viewer->find(search_box_->text());
    updateSearchStatus();
}

void MainWindow::searchNext()
{
    auto* view = currentView();
    if (!view || search_box_->text().isEmpty()) return;
    if (!view->viewer->find(search_box_->text())) {
        view->viewer->moveCursor(QTextCursor::Start);
        view->viewer->find(search_box_->text());
    }
    updateSearchStatus();
}

void MainWindow::searchPrevious()
{
    auto* view = currentView();
    if (!view || search_box_->text().isEmpty()) return;
    if (!view->viewer->find(search_box_->text(), QTextDocument::FindBackward)) {
        view->viewer->moveCursor(QTextCursor::End);
        view->viewer->find(search_box_->text(), QTextDocument::FindBackward);
    }
    updateSearchStatus();
}

void MainWindow::updateSearchStatus()
{
    const auto* view = currentView();
    if (!view || search_box_->text().isEmpty()) {
        search_status_->clear();
        return;
    }

    amigaguide::SearchEngine engine;
    const auto matches = engine.find(view->document, search_box_->text().toStdString());
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
    const auto* view = currentView();
    const bool can_back = view && view->navigation_history.can_back();
    const bool can_forward = view && view->navigation_history.can_forward();
    if (back_action_) back_action_->setEnabled(can_back);
    if (forward_action_) forward_action_->setEnabled(can_forward);
    if (home_action_) home_action_->setEnabled(view && !view->home_node.isEmpty());
}

void MainWindow::updateTabTitle(int index)
{
    if (index < 0 || index >= static_cast<int>(views_.size())) return;
    const auto& view = views_[static_cast<std::size_t>(index)];
    const QString title = view.current_document_path.isEmpty()
        ? tr("New Tab")
        : QFileInfo(view.current_document_path).fileName();
    tabs_->setTabText(index, title);
    tabs_->setTabToolTip(index, view.current_document_path.isEmpty()
                                      ? tr("No document loaded")
                                      : view.current_document_path);
}

void MainWindow::tabChanged(int index)
{
    if (index < 0 || index >= static_cast<int>(views_.size())) return;
    search_box_->clear();
    updateSearchStatus();
    updateNavigationActions();
    const auto& view = views_[static_cast<std::size_t>(index)];
    setWindowTitle(view.current_document_path.isEmpty()
                       ? QStringLiteral("AmigaGuide on Linux")
                       : QStringLiteral("%1 — AmigaGuide on Linux").arg(QFileInfo(view.current_document_path).fileName()));
}

void MainWindow::setCrossDocumentLinksInTab()
{
    setCrossDocumentMode(CrossDocumentMode::NewTab);
}

void MainWindow::setCrossDocumentLinksInWindow()
{
    setCrossDocumentMode(CrossDocumentMode::NewWindow);
}

void MainWindow::setCrossDocumentMode(CrossDocumentMode mode)
{
    QSettings settings;
    settings.setValue(kCrossDocumentLinksKey,
                      mode == CrossDocumentMode::NewTab ? kNewTabValue : kNewWindowValue);
}

MainWindow::CrossDocumentMode MainWindow::crossDocumentMode() const
{
    const auto* action = cross_document_group_->checkedAction();
    if (action && action->text() == tr("New window")) return CrossDocumentMode::NewWindow;
    return CrossDocumentMode::NewTab;
}
