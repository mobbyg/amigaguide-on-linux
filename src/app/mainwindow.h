#pragma once

#include <QMainWindow>
#include <QUrl>

#include <QString>
#include <vector>

#include "amigaguide/document.h"

class QLabel;
class QLineEdit;
class QTextBrowser;
class QAction;

class MainWindow final : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void openFile();
    void openLink(const QUrl& url);
    void searchText();
    void searchNext();
    void searchPrevious();
    void navigateBack();
    void navigateForward();
    void navigateHome();

private:
    void navigateToNode(const QString& node, bool add_history = true);
    void updateNavigationActions();
    void updateSearchStatus();

    QTextBrowser* viewer_ = nullptr;
    QLineEdit* search_box_ = nullptr;
    QLabel* search_status_ = nullptr;
    QAction* back_action_ = nullptr;
    QAction* forward_action_ = nullptr;
    QAction* home_action_ = nullptr;

    amigaguide::Document document_;
    std::vector<QString> navigation_history_;
    int history_index_ = -1;
    QString home_node_;
};
