#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QTreeView>
#include <QPlainTextEdit>
#include <QScrollArea>
#include <sys/time.h>
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

    //  Set central widget of window
    QWidget *widget = new QWidget;
    setCentralWidget(widget);

    //  Set the root layout to be a VBox
    QVBoxLayout *rootLayout = new QVBoxLayout;
    widget->setLayout(rootLayout);

    //  Create properties viewing area
    mProperties = new QTreeView;
    form_panel = new QWidget();
    form_layout = new QVBoxLayout();
    form_layout->setMargin(0);
    form_panel->setLayout(form_layout);

    mScrollArea = new QScrollArea();
    mScrollArea->setWidgetResizable(true);
    mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    //mScrollArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    //mScrollArea->setViewportMargins(0, 0, 60, 0);
    //mScrollArea->setBackgroundRole(QPalette::Dark);
    form_layout->addWidget(mScrollArea);
    mScrollArea->setMinimumWidth(600);

//    QWidget* ppanel = new QWidget();
//    mScrollArea->setWidget(ppanel);
//    QVBoxLayout* playout = new QVBoxLayout;
//    ppanel->setLayout(playout);
//    form_layout = playout;

    QSplitter* hSplitter = new QSplitter;
    hSplitter->addWidget(mProperties);
    //hSplitter->addWidget(form_panel);
    //hSplitter->setOpaqueResize(false);
    hSplitter->addWidget(form_panel);
    hSplitter->setStretchFactor(0, 45);
    hSplitter->setStretchFactor(2, 55);
    rootLayout->addWidget(hSplitter, 85);


    //mScrollArea->setWidget(form_panel);
//    form_panel->show();
//    mScrollArea->show();
//    ppanel->show();

    //  Create log viewer
    mLog = new QPlainTextEdit;
    mLog->setReadOnly(true);
    rootLayout->addWidget(mLog, 15);

    mServiceModel = new ServiceModel("_indigo._tcp");

    mPropertyModel = new PropertyModel();
    mProperties->setHeaderHidden(true);
    mProperties->setModel(mPropertyModel);

    connect(&IndigoClient::instance(), &IndigoClient::property_defined, mPropertyModel, &PropertyModel::define_property);
    connect(&IndigoClient::instance(), &IndigoClient::property_changed, mPropertyModel, &PropertyModel::update_property);
    connect(&IndigoClient::instance(), &IndigoClient::property_deleted, mPropertyModel, &PropertyModel::delete_property);

	connect(&IndigoClient::instance(), &IndigoClient::property_defined, this, &BrowserWindow::on_property_log);
	connect(&IndigoClient::instance(), &IndigoClient::property_changed, this, &BrowserWindow::on_property_log);
	connect(&IndigoClient::instance(), &IndigoClient::property_deleted, this, &BrowserWindow::on_property_log);

    connect(mProperties->selectionModel(), &QItemSelectionModel::selectionChanged, this, &BrowserWindow::on_selection_changed);

    current_node = nullptr;

    //  Start up the client
    IndigoClient::instance().start();
}


void
BrowserWindow::on_property_log(indigo_property* property, const char *message) {
	char timestamp[16];
	char log_line[255];
	struct timeval tmnow;

	if (!message) return;

	gettimeofday(&tmnow, NULL);
        strftime(timestamp, 9, "%H:%M:%S", localtime((const time_t *) &tmnow.tv_sec));
        snprintf(timestamp + 8, sizeof(timestamp) - 8, ".%06ld", tmnow.tv_usec);

	snprintf(log_line, sizeof(log_line), "%s - '%s'.%s : %s", timestamp, property->device, property->name, message);

	mLog->appendPlainText(log_line); // Adds the message to the widget
	//mLog->verticalScrollBar()->setValue(mLog->verticalScrollBar()->maximum());
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
        //delete form_panel;
        //form_panel = nullptr;
//        if (form_layout != nullptr) {
//            delete form_layout;
//            form_layout = nullptr;
//        }
        current_node = nullptr;
        return;
    }

    QModelIndex s = selected.indexes().front();
    TreeNode* n = static_cast<TreeNode*>(s.internalPointer());
	if (n != nullptr) fprintf(stderr, "SELECTION CHANGED n->node_type == %d\n", n->node_type);
    if (n != nullptr && n->node_type == TREE_NODE_PROPERTY) {
		fprintf(stderr, "SELECTION CHANGED n->node_type == TREE_NODE_PROPERTY\n");
        PropertyNode* p = reinterpret_cast<PropertyNode*>(n);
        //delete form_panel;
        //form_panel = nullptr;

//        if (form_layout != nullptr)
//            delete form_layout;

//        form_layout = new QVBoxLayout();
//        form_layout->setAlignment(Qt::AlignTop);
//        form_layout->setMargin(10);

        QIndigoProperty* ip = new QIndigoProperty(p->property);
        //form_layout->addWidget(ip);
        current_node = p;

//        form_panel = new QFrame();
//        form_panel->setLayout(form_layout);
//        mScrollArea->setWidget(form_panel);

//        QScrollArea* scroll = new QScrollArea();
//        scroll->setWidgetResizable(true);
//        QWidget* f = new QWidget();
//        QVBoxLayout* l = new QVBoxLayout();
//        l->setAlignment(Qt::AlignTop);
//        l->setContentsMargins(10, 10, 32, 10);
//        f->setLayout(l);
//        l->addWidget(ip);

//        scroll->setWidget(f);
//        form_layout->addWidget(scroll);

        //mScrollArea->setStyleSheet("background-color: green");


        QWidget* ppanel = new QWidget();
        //ppanel->setStyleSheet("background-color: red");
        QVBoxLayout* playout = new QVBoxLayout;
        playout->setSpacing(10);
        playout->setContentsMargins(10, 10, 25, 10);
        playout->setSizeConstraint(QLayout::SetMinimumSize);
        ppanel->setLayout(playout);
        playout->addWidget(ip);
        mScrollArea->setWidget(ppanel);
        ppanel->show();

        //  Connect to update signals coming from indigo bus
        connect(mPropertyModel, &PropertyModel::property_updated, ip, &QIndigoProperty::property_update);
    }
    else if (n != nullptr && n->node_type == TREE_NODE_GROUP) {
        fprintf(stderr, "SELECTION CHANGED n->node_type == TREE_NODE_GROUP\n");
        GroupNode* g = reinterpret_cast<GroupNode*>(n);
        //current_node = g;

//        delete form_panel;
//        form_panel = nullptr;

        //mScrollArea->setStyleSheet("background-color: green");

        QWidget* ppanel = new QWidget();
        //ppanel->setStyleSheet("background-color: red");
        QVBoxLayout* playout = new QVBoxLayout;
        playout->setSpacing(10);
        playout->setContentsMargins(10, 10, 25, 10);
        playout->setSizeConstraint(QLayout::SetMinimumSize);
        ppanel->setLayout(playout);

        //  Iterate properties
        for (int i = 0; i < g->children.count; i++) {
        //g->each_child([playout](PropertyNode* p){
            PropertyNode* p = g->children.nodes[i];
            QIndigoProperty* ip = new QIndigoProperty(p->property);
            //ip->setStyleSheet("background-color: blue");
            fprintf(stderr, "POPER\n");
            playout->addWidget(ip);
			connect(mPropertyModel, &PropertyModel::property_updated, ip, &QIndigoProperty::property_update);

        }//);

        mScrollArea->setWidget(ppanel);
        mScrollArea->setWidgetResizable(true);
        ppanel->show();

        //QIndigoProperty* ip = new QIndigoProperty(p->property);

    }
}
