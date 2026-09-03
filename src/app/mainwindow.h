#pragma once

#include <QMainWindow>
#include <QUrl>

#include <QString>
#include <vector>

#include "amigaguide/destination.h"
#include "amigaguide/document.h"
#include "amigaguide/navigation.h"

class QAction;
class QActionGroup;
class QLabel;
class QLineEdit;
class QTabWidget;
class QTextBrowser;

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
    void tabChanged(int index);
    void setCrossDocumentLinksInTab();
    void setCrossDocumentLinksInWindow();

private:
    enum class CrossDocumentMode {
        NewTab,
        NewWindow
    };

    struct ViewState {
        QTextBrowser* viewer = nullptr;
        amigaguide::Document document;
        amigaguide::NavigationHistory navigation_history;
        QString current_document_path;
        QString home_node;
    };

    ViewState* currentView();
    const ViewState* currentView() const;
    ViewState* createTab(bool switch_to);
    bool loadDocumentFile(ViewState& view, const QString& path, const QString& requested_node, bool add_history);
    void renderDocument(ViewState& view);
    amigaguide::Destination currentNodeDestination(const ViewState& view, const QString& node) const;
    void navigateToNode(const QString& node, bool add_history = true);
    void navigateToDestination(const amigaguide::Destination& destination, bool add_history = true);
    void updateNavigationActions();
    void updateSearchStatus();
    void updateTabTitle(int index);
    void setCrossDocumentMode(CrossDocumentMode mode);
    CrossDocumentMode crossDocumentMode() const;

    QTabWidget* tabs_ = nullptr;
    QLineEdit* search_box_ = nullptr;
    QLabel* search_status_ = nullptr;
    QAction* back_action_ = nullptr;
    QAction* forward_action_ = nullptr;
    QAction* home_action_ = nullptr;
    QAction* new_tab_action_ = nullptr;
    QAction* new_window_action_ = nullptr;
    QActionGroup* cross_document_group_ = nullptr;

    std::vector<ViewState> views_;
};
