#include "mainwindow.h"

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenuBar>
#include <QMessageBox>
#include <QTextBrowser>
#include <QTextStream>
#include <QUrl>

#include "amigaguide/parser.h"

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
    html += QStringLiteral("<html><body>");
    html += QStringLiteral("<h1>%1</h1>").arg(QFileInfo(path).fileName().toHtmlEscaped());
    html += QStringLiteral("<p><b>Nodes:</b> %1</p>").arg(document.nodes().size());

    for (const auto& node : document.nodes()) {
        html += QStringLiteral("<hr><h2 id=\"%1\">%2</h2>")
                    .arg(QString::fromStdString(node.name).toHtmlEscaped(),
                         QString::fromStdString(node.title).toHtmlEscaped());
        if (!node.next.empty()) {
            html += QStringLiteral("<p><a href=\"node:%1\">Next: %1</a></p>")
                        .arg(QString::fromStdString(node.next).toHtmlEscaped());
        }
        if (!node.prev.empty()) {
            html += QStringLiteral("<p><a href=\"node:%1\">Previous: %1</a></p>")
                        .arg(QString::fromStdString(node.prev).toHtmlEscaped());
        }
    }
    html += QStringLiteral("</body></html>");

    viewer_->setHtml(html);
    setWindowTitle(QStringLiteral("%1 — AmigaGuide on Linux").arg(QFileInfo(path).fileName()));
}

void MainWindow::openLink(const QUrl& url)
{
    if (url.scheme() == QStringLiteral("node")) {
        viewer_->scrollToAnchor(url.path());
    }
}
