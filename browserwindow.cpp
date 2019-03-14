#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QListView>
#include <QTreeView>
#include "browserwindow.h"
#include "servicemodel.h"
#include "propertymodel.h"
#include "indigoclient.h"
#include "qindigoproperty.h"


BrowserWindow::BrowserWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("INDIGO Control Panel"));
    resize(1024, 768);

    mLog = new QListView;
    mServices = new QListView;
    mProperties = new QTreeView;

    QVBoxLayout *rootLayout = new QVBoxLayout;
    QWidget *widget = new QWidget;
    widget->setLayout(rootLayout);
    setCentralWidget(widget);

    //  Add basic vertical layout for form panel
    form_panel = new QWidget();
    QVBoxLayout* formLayout = new QVBoxLayout;
    form_panel->setLayout(formLayout);

    hSplitter = new QSplitter;
    hSplitter->addWidget(mProperties);
    hSplitter->addWidget(form_panel);
    hSplitter->setOpaqueResize(false);
    rootLayout->addWidget(hSplitter);

    mServiceModel = new ServiceModel("_indigo._tcp");
    mServices->setModel(mServiceModel);

    mPropertyModel = new PropertyModel();
    mProperties->setHeaderHidden(true);
    mProperties->setSortingEnabled(true);
    mProperties->setModel(mPropertyModel);
    mProperties->resize(400, mProperties->height());
    mProperties->setMaximumWidth(400);

    connect(&IndigoClient::instance(), &IndigoClient::property_defined, mPropertyModel, &PropertyModel::define_property);
    connect(&IndigoClient::instance(), &IndigoClient::property_changed, mPropertyModel, &PropertyModel::update_property);
    connect(&IndigoClient::instance(), &IndigoClient::property_deleted, mPropertyModel, &PropertyModel::delete_property);

    connect(mProperties->selectionModel(), &QItemSelectionModel::selectionChanged, this, &BrowserWindow::on_selection_changed);

    current_node = nullptr;

    //  Start up the client
    IndigoClient::instance().start();
}

void
BrowserWindow::on_selection_changed(const QItemSelection &selected, const QItemSelection &) {
    fprintf(stderr, "SELECTION CHANGED\n");

    //  Deal with the outgoing selection
    if (current_node != nullptr) {
		fprintf(stderr, "SELECTION CHANGED no current node\n");
    }

    if (selected.indexes().empty()) {
		fprintf(stderr, "SELECTION CHANGED selected.indexes().empty()\n");
		delete form_panel;
		form_panel = nullptr;
        current_node = nullptr;
        return;
    }

    QModelIndex s = selected.indexes().front();
    TreeNode* n = static_cast<TreeNode*>(s.internalPointer());
	if (n != nullptr) fprintf(stderr, "SELECTION CHANGED n->node_type == %d\n", n->node_type);
    if (n != nullptr && n->node_type == TREE_NODE_PROPERTY) {
		fprintf(stderr, "SELECTION CHANGED n->node_type == TREE_NODE_PROPERTY\n");
        PropertyNode* p = reinterpret_cast<PropertyNode*>(n);
		delete form_panel;
		form_panel = nullptr;
        QIndigoProperty* ip = new QIndigoProperty(p->property);
        current_node = p;
        form_panel = ip;

        hSplitter->addWidget(form_panel);

        //  Connect to update signals coming from indigo bus
        connect(mPropertyModel, &PropertyModel::property_updated, ip, &QIndigoProperty::property_update);
    }
}
