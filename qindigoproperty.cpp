#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include "qindigoproperty.h"
#include "qindigotext.h"
#include "qindigonumber.h"
#include "qindigoswitch.h"
#include "qindigolight.h"


QIndigoProperty::QIndigoProperty(indigo_property* property, QWidget *parent)
    : QFrame(parent), m_property(property)
{
    //  Set widget layout
    QVBoxLayout* formLayout = new QVBoxLayout;
    formLayout->setAlignment(Qt::AlignTop);
    setLayout(formLayout);
    setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
//    formLayout->setMargin(0);
//    formLayout->setSpacing(0);

    //  Allocate storage for the controls
    m_controls = reinterpret_cast<QIndigoItem**>(malloc(sizeof(QIndigoItem*) * property->count));

    //  Add the control grid
    build_property_form(formLayout);
}

QIndigoProperty::~QIndigoProperty() {
    //free(controls);
}

void
QIndigoProperty::update_controls() {
    for (int i = 0; i < m_property->count; i++) {
        m_controls[i]->update();
    }
}

void
QIndigoProperty::update() {
    //  Update all the controls to the current state
    update_controls();

    //  Update the property on the bus
    indigo_change_property(nullptr, m_property);
}

void
QIndigoProperty::property_update(indigo_property* property) {
    //  Ignore updates to properties that this widget doesn't represent
    if (m_property != property)
        return;

    //  It's for us, so update!!
    //fprintf(stderr, "UPDATE TO [%s]    WE WANT [%s]\n", property->name, m_property->name);
    update_controls();
}


void
QIndigoProperty::build_property_form(QVBoxLayout* layout) {
    //  Build the header
    QHBoxLayout* hbox = new QHBoxLayout();
    layout->addLayout(hbox);
    hbox->setAlignment(Qt::AlignLeft);
    hbox->setMargin(10);
    hbox->setSpacing(5);

    char title[1025];
    sprintf(title, "%s -> %s", m_property->device, m_property->label);
    QLabel* led = new QLabel();
    QLabel* header = new QLabel(title);
    hbox->addWidget(led);
    hbox->addWidget(header);

    switch (m_property->state) {
    case INDIGO_IDLE_STATE:
        led->setPixmap(QPixmap(":led-grey.png"));
        break;
    case INDIGO_BUSY_STATE:
        led->setPixmap(QPixmap(":led-orange.png"));
        break;
    case INDIGO_ALERT_STATE:
        led->setPixmap(QPixmap(":led-red.png"));
        break;
    case INDIGO_OK_STATE:
        led->setPixmap(QPixmap(":led-green.png"));
        break;
    }

    //  Build the item fields
    switch (m_property->type) {
    case INDIGO_TEXT_VECTOR:
        build_text_property_form(layout);
        break;
    case INDIGO_NUMBER_VECTOR:
        build_number_property_form(layout);
        break;
    case INDIGO_SWITCH_VECTOR:
        build_switch_property_form(layout);
        break;
    case INDIGO_LIGHT_VECTOR:
        build_light_property_form(layout);
        break;
    case INDIGO_BLOB_VECTOR:
        fprintf(stderr, "BUILD BLOB FORM\n");
        break;
    }
}

void
QIndigoProperty::build_text_property_form(QVBoxLayout* layout) {
    //  Create grid for the items
    QGridLayout* form_grid = new QGridLayout;
    form_grid->setColumnStretch(0, 35);
    form_grid->setColumnStretch(1, 65);
    layout->addLayout(form_grid);

    //  Build each item
    for (int row = 0; row < m_property->count; row++) {
        indigo_item& i = m_property->items[row];

        QIndigoText* data = new QIndigoText(this, m_property, &i);
        m_controls[row] = data;

        layout->addWidget(data);
    }

    if (m_property->perm != INDIGO_RO_PERM) {
        //  Add buttons
        QPushButton* set = new QPushButton("Set");
        QPushButton* reset = new QPushButton("Reset");
        layout->addWidget(reset);
        layout->addWidget(set);
    }

    //  We want to button press signal to cause the form fields to update to the property and send on bus
    //  To do this, we have to know which property is selected, and then we find the PropertyNode for it.
    //  We then run through the ItemNodes and copy any modified fields to the Items.
    //  Finally we send a property update on the bus.
}

void
QIndigoProperty::build_number_property_form(QVBoxLayout* layout) {
    //  Create grid for the items
//    QGridLayout* form_grid = new QGridLayout;
//    form_grid->setColumnStretch(0, 35);
//    form_grid->setColumnStretch(1, 65);
//    layout->addLayout(form_grid);

    //  Build each item
    char buffer[50];
    for (int row = 0; row < m_property->count; row++) {
        indigo_item& i = m_property->items[row];

        QIndigoNumber* data = new QIndigoNumber(this, m_property, &i);
        m_controls[row] = data;

        layout->addWidget(data);
    }

    if (m_property->perm != INDIGO_RO_PERM) {
        //  Add buttons
        QPushButton* button = new QPushButton("Set");
        layout->addWidget(button);
    }
}

void
QIndigoProperty::build_switch_property_form(QVBoxLayout* layout) {
    //  Create grid for the items
//    QGridLayout* form_grid = new QGridLayout;
//    form_grid->setColumnStretch(0, 35);
//    form_grid->setColumnStretch(1, 65);
//    layout->addLayout(form_grid);

    //  Build each item
    for (int row = 0; row < m_property->count; row++) {
        indigo_item& i = m_property->items[row];

        QIndigoSwitch* data = new QIndigoSwitch(this, m_property, &i);
        m_controls[row] = data;

        layout->addWidget(data);

        connect(data, &QIndigoSwitch::item_changed, this, &QIndigoProperty::update);
    }
}

void
QIndigoProperty::build_light_property_form(QVBoxLayout* layout) {
    //  Create grid for the items
//    QGridLayout* form_grid = new QGridLayout;
//    form_grid->setColumnStretch(0, 5);
//    form_grid->setColumnStretch(1, 95);
//    layout->addLayout(form_grid);

    //  Build each item
    for (int row = 0; row < m_property->count; row++) {
        indigo_item& i = m_property->items[row];

        QIndigoLight* light = new QIndigoLight(this, m_property, &i);
        m_controls[row] = light;

        layout->addWidget(light);
    }
}
