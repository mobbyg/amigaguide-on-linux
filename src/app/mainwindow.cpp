#include "mainwindow.h"

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenuBar>
#include <QMessageBox>
#include <QTextBrowser>
#include <QUrl>

#include "amigaguide/parser.h"
#include "amigaguide/renderer.h"

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

    connect(viewer_, &QTextBrowser::anchorClicked, this, &MainWindow::openLink);
    viewer_->setOpenExternalLinks(false);
    viewer_->setOpenLinks(false);
    viewer_->setPlaceholderText(tr("Open an AmigaGuide file to begin."));
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
    amigaguide::Document document;
    amigaguide::ParseError error;
    amigaguide::Parser parser;
    if (!parser.parse(bytes.toStdString(), document, &error)) {
        QMessageBox::critical(this, tr("Parse failed"),
                              tr("Line %1: %2").arg(error.line).arg(QString::fromStdString(error.message)));
        return;
    }

    QString html;
    html += QStringLiteral("<html><head><style>");
    html += QStringLiteral("body{font-family:sans-serif;margin:18px;}h1{margin-bottom:12px;}a{font-weight:600;}p.meta{color:#666;}");
    html += QStringLiteral("</style></head><body>");
    html += QStringLiteral("<p class=\"meta\"><b>Database:</b> %1 &nbsp; <b>Nodes:</b> %2</p>")
                .arg(QString::fromStdString(document.metadata().name).toHtmlEscaped())
                .arg(document.nodes().size());

    for (const auto& node : document.nodes()) {
        html += QString::fromStdString(amigaguide::render_node_html(document, node));
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
    setWindowTitle(QStringLiteral("%1 — AmigaGuide on Linux").arg(QFileInfo(path).fileName()));
}

void MainWindow::openLink(const QUrl& url)
{
    if (url.scheme() != QStringLiteral("node")) return;

    const QString target = url.path().isEmpty() ? url.host() : url.path();
    viewer_->scrollToAnchor(target);
}
