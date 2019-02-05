#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QListView>
#include <QTreeView>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include "browserwindow.h"
#include "servicemodel.h"
#include "propertymodel.h"
#include "indigoclient.h"


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
        for (TreeIterator i = current_node->children.begin(); i != current_node->children.end(); i++) {
            ItemNode* item = reinterpret_cast<ItemNode*>(*i);
            item->input_control = nullptr;
        }
    }

    QModelIndex s = selected.indexes().front();
    TreeNode* n = static_cast<TreeNode*>(s.internalPointer());
    if (n != nullptr && n->node_type == TREE_NODE_PROPERTY) {
        PropertyNode* p = reinterpret_cast<PropertyNode*>(n);
        if (form_panel != nullptr)
            delete form_panel;

        form_panel = new QWidget();
        QVBoxLayout* formLayout = new QVBoxLayout;
        form_panel->setLayout(formLayout);
        formLayout->setAlignment(Qt::AlignTop);
        form_grid = build_property_form(p);
        formLayout->addLayout(form_grid);

        hSplitter->addWidget(form_panel);

        current_node = p;
    }
}

QGridLayout*
BrowserWindow::build_property_form(PropertyNode* p) {
    switch (p->property->type) {
    case INDIGO_TEXT_VECTOR:
        fprintf(stderr, "BUILD TEXT FORM\n");
        return build_text_property_form(p);
    case INDIGO_NUMBER_VECTOR:
        fprintf(stderr, "BUILD NUMBER FORM\n");
        return build_number_property_form(p);
    case INDIGO_SWITCH_VECTOR:
        fprintf(stderr, "BUILD SWITCH FORM\n");
        return build_switch_property_form(p);
    case INDIGO_LIGHT_VECTOR:
        fprintf(stderr, "BUILD LIGHT FORM\n");
        break;
    case INDIGO_BLOB_VECTOR:
        fprintf(stderr, "BUILD BLOB FORM\n");
        break;
    }
    fprintf(stderr, "BUILD WIDGET\n");
    return new QGridLayout();
}

QGridLayout*
BrowserWindow::build_text_property_form(PropertyNode* p) {
    //  Create grid for the items
    QGridLayout* form_grid = new QGridLayout;
    form_grid->setColumnStretch(0, 35);
    form_grid->setColumnStretch(1, 65);

    //  Build each item
    for (int row = 0; row < p->property->count; row++) {
//fprintf(stderr, "Item label: [%s]   value: [%s]\n", p->items[row].label, p->items[row].text.value);
        QLabel* label = new QLabel(p->property->items[row].label);
        QLineEdit* data = new QLineEdit(p->property->items[row].text.value);
        data->setReadOnly(true);
        form_grid->addWidget(label, row, 0);
        form_grid->addWidget(data, row, 1);

        ItemNode* item = reinterpret_cast<ItemNode*>(p->children[row]);
        item->input_control = data;
    }

    if (p->property->perm != INDIGO_RO_PERM) {
        //  Add buttons
        QPushButton* button = new QPushButton("Set");
        form_grid->addWidget(button, p->property->count + 1, 1);
    }

    //  We want to button press signal to cause the form fields to update to the property and send on bus
    //  To do this, we have to know which property is selected, and then we find the PropertyNode for it.
    //  We then run through the ItemNodes and copy any modified fields to the Items.
    //  Finally we send a property update on the bus.

    //  Return the form
    return form_grid;
}

QGridLayout*
BrowserWindow::build_number_property_form(PropertyNode* p) {
    //  Create grid for the items
    QGridLayout* form_grid = new QGridLayout;
    form_grid->setColumnStretch(0, 35);
    form_grid->setColumnStretch(1, 65);

    //  Build each item
    char buffer[50];
    for (int row = 0; row < p->property->count; row++) {
//fprintf(stderr, "Item label: [%s]   value: [%s]\n", p->items[row].label, p->items[row].text.value);
        sprintf(buffer, p->property->items[row].number.format, p->property->items[row].number.value);
        QLabel* label = new QLabel(p->property->items[row].label);
        QLineEdit* data = new QLineEdit(buffer);
        data->setReadOnly(true);
        form_grid->addWidget(label, row, 0);
        form_grid->addWidget(data, row, 1);

        ItemNode* item = reinterpret_cast<ItemNode*>(p->children[row]);
        item->input_control = data;
    }

    if (p->property->perm != INDIGO_RO_PERM) {
        //  Add buttons
        QPushButton* button = new QPushButton("Set");
        form_grid->addWidget(button, p->property->count + 1, 1);
    }

    //  Return the form
    return form_grid;
}

QGridLayout*
BrowserWindow::build_switch_property_form(PropertyNode* p) {
    //  Create grid for the items
    QGridLayout* form_grid = new QGridLayout;
    form_grid->setColumnStretch(0, 35);
    form_grid->setColumnStretch(1, 65);

    //  Build each item
    for (int row = 0; row < p->property->count; row++) {
        QCheckBox* data = new QCheckBox(p->property->items[row].label);
        data->setChecked(p->property->items[row].sw.value);
        if (p->property->perm == INDIGO_RO_PERM)
            data->setEnabled(false);
        form_grid->addWidget(data, row, 0);

        ItemNode* item = reinterpret_cast<ItemNode*>(p->children[row]);
        item->input_control = data;

        connect(data, &QCheckBox::clicked, item, &ItemNode::checkbox_clicked);
    }

    //  Return the form
    return form_grid;
}
