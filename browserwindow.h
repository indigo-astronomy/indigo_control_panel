#ifndef BROWSERWINDOW_H
#define BROWSERWINDOW_H

#include <QMainWindow>
#include <qmdnsengine/server.h>
#include <indigo_bus.h>


class QListView;
class QTreeView;
class ServiceModel;
class PropertyModel;
class QItemSelection;
class QVBoxLayout;
class QGridLayout;
class QSplitter;
class PropertyNode;
class TreeNode;


class BrowserWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit BrowserWindow(QWidget *parent = nullptr);

    QGridLayout* build_property_form(PropertyNode* p);
    QGridLayout* build_text_property_form(PropertyNode* p);
    QGridLayout* build_number_property_form(PropertyNode* p);
    QGridLayout* build_switch_property_form(PropertyNode* p);

public slots:
    void on_selection_changed(const QItemSelection &selected, const QItemSelection &deselected);

private:
    QListView* mLog;
    QListView* mServices;
    QTreeView* mProperties;
    QVBoxLayout* formLayout;
    QGridLayout* form_grid;
    QWidget* form_panel;
    QSplitter* hSplitter;

    ServiceModel* mServiceModel;
    PropertyModel* mPropertyModel;
    TreeNode* current_node;
};

#endif // BROWSERWINDOW_H
