#ifndef BROWSERWINDOW_H
#define BROWSERWINDOW_H

#include <QMainWindow>
#include <indigo_bus.h>


class QStringListModel;
class QListView;
class QTreeView;
class ServiceModel;
class PropertyModel;
class QItemSelection;
class QVBoxLayout;
class QScrollArea;
struct PropertyNode;
struct TreeNode;


class BrowserWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit BrowserWindow(QWidget *parent = nullptr);

public slots:
    void on_selection_changed(const QItemSelection &selected, const QItemSelection &deselected);

private:
    QStringListModel* mLogList;
    QListView* mLog;
    QTreeView* mProperties;
    QScrollArea* mScrollArea;
    QWidget* form_panel;
    QVBoxLayout* form_layout;

    ServiceModel* mServiceModel;
    PropertyModel* mPropertyModel;
    PropertyNode* current_node;
};

#endif // BROWSERWINDOW_H
