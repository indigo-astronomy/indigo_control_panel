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
#include "qindigoblob.h"


QIndigoProperty::QIndigoProperty(indigo_property* property, QWidget *parent) : QWidget(parent), m_property(property) {
	//  Set widget layout
	QVBoxLayout* formLayout = new QVBoxLayout;
	formLayout->setAlignment(Qt::AlignTop);
	setLayout(formLayout);
	//setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
	formLayout->setContentsMargins(0, 0, 0, 0);
	// formLayout->setSpacing(0);

	//  Allocate storage for the controls
	m_controls = reinterpret_cast<QIndigoItem**>(malloc(sizeof(QIndigoItem*) * property->count));

	//  Add the control grid
	build_property_form(formLayout);
}


QIndigoProperty::~QIndigoProperty() {
	free(m_controls);
}


void QIndigoProperty::update_controls() {
	for (int i = 0; i < m_property->count; i++) {
		m_controls[i]->update();
	}
}


void QIndigoProperty::update_property_view() {
	switch (m_property->state) {
	case INDIGO_IDLE_STATE:
		m_led->setPixmap(QPixmap(":resource/led-grey.png"));
		this->setStyleSheet("background-color: #202020");
		break;
	case INDIGO_BUSY_STATE:
		m_led->setPixmap(QPixmap(":resource/led-orange.png"));
		this->setStyleSheet("background-color: #353520");
		break;
	case INDIGO_ALERT_STATE:
		m_led->setPixmap(QPixmap(":resource/led-red.png"));
		this->setStyleSheet("background-color: #352020");
		break;
	case INDIGO_OK_STATE:
		m_led->setPixmap(QPixmap(":resource/led-green.png"));
		this->setStyleSheet("background-color: #203220");
		break;
	}
	m_led->update();
}


void QIndigoProperty::update() {
	//  Update all the controls to the current state
	update_controls();
	update_property_view();

	//  Update the property on the bus
	indigo_change_property(nullptr, m_property);
}


void QIndigoProperty::property_update(indigo_property* property) {
	//  Ignore updates to properties that this widget doesn't represent
	if (m_property != property)
		return;

	//  It's for us, so update!!
	//fprintf(stderr, "UPDATE TO [%s]	WE WANT [%s]\n", property->name, m_property->name);
	update_controls();
	update_property_view();
}


void QIndigoProperty::set_clicked() {
	//  Apply all dirty controls
	for (int i = 0; i < m_property->count; i++)
		m_controls[i]->apply();

	//  Update property on indigo bus
	indigo_change_property(nullptr, m_property);
}


void QIndigoProperty::reset_clicked() {
	//  Revert all dirty controls to clean
	for (int i = 0; i < m_property->count; i++)
		m_controls[i]->reset();
}


void QIndigoProperty::build_property_form(QVBoxLayout* layout) {
	//  Build the header

	QFrame *frame = new QFrame();
	frame->setFrameShadow(QFrame::Raised);
	frame->setFrameShape(QFrame::StyledPanel);

	QVBoxLayout* vbox = new QVBoxLayout();
	vbox->addWidget(frame);
	layout->addLayout(vbox);

	QVBoxLayout* frame_layout = new QVBoxLayout();
	frame->setLayout(frame_layout);

	QWidget *head = new QWidget();
	frame_layout->addWidget(head);
	QHBoxLayout* head_layout = new QHBoxLayout();
	head->setLayout(head_layout);

	head_layout->setAlignment(Qt::AlignLeft);
	head_layout->setMargin(3);
	head_layout->setSpacing(5);

	char device_string[1025];
	sprintf(device_string, "@ %s", m_property->device);
	m_led = new QLabel();
	QLabel* property_label = new QLabel(m_property->label);
	property_label->setStyleSheet("font-weight: bold;");
	QLabel* device_label = new QLabel(device_string);

	head_layout->addWidget(m_led);
	head_layout->addWidget(property_label);
	//head_layout->addStretch();
	head_layout->addWidget(device_label);

	QWidget *property_setings = new QWidget();
	head_layout->addWidget(property_setings);
	QVBoxLayout* property_layout = new QVBoxLayout();
	property_setings->setLayout(property_layout);

	update_property_view();

	//  Build the item fields
	switch (m_property->type) {
	case INDIGO_TEXT_VECTOR:
		build_text_property_form(frame_layout);
		break;
	case INDIGO_NUMBER_VECTOR:
		build_number_property_form(frame_layout);
		break;
	case INDIGO_SWITCH_VECTOR:
		build_switch_property_form(frame_layout);
		break;
	case INDIGO_LIGHT_VECTOR:
		build_light_property_form(frame_layout);
		break;
	case INDIGO_BLOB_VECTOR:
		build_blob_property_form(frame_layout);
		break;
	}

	//  Build buttons (if appropriate)
	build_buttons(frame_layout);
}


void QIndigoProperty::build_text_property_form(QVBoxLayout* layout) {
	//  Build each item
	for (int row = 0; row < m_property->count; row++) {
		indigo_item& i = m_property->items[row];

		QIndigoText* data = new QIndigoText(this, m_property, &i);
		m_controls[row] = data;

		layout->addWidget(data);
	}

	//  We want to button press signal to cause the form fields to update to the property and send on bus
	//  To do this, we have to know which property is selected, and then we find the PropertyNode for it.
	//  We then run through the ItemNodes and copy any modified fields to the Items.
	//  Finally we send a property update on the bus.
}


void QIndigoProperty::build_number_property_form(QVBoxLayout* layout) {
	//  Build each item
	for (int row = 0; row < m_property->count; row++) {
		indigo_item& i = m_property->items[row];

		QIndigoNumber* data = new QIndigoNumber(this, m_property, &i);
		m_controls[row] = data;

		layout->addWidget(data);
	}
}


void QIndigoProperty::build_switch_property_form(QVBoxLayout* layout) {
	//  Build each item
	for (int row = 0; row < m_property->count; row++) {
		indigo_item& i = m_property->items[row];

		QIndigoSwitch* data = new QIndigoSwitch(this, m_property, &i);
		m_controls[row] = data;

		layout->addWidget(data);

		connect(data, &QIndigoSwitch::item_changed, this, &QIndigoProperty::update);
	}
}


void QIndigoProperty::build_light_property_form(QVBoxLayout* layout) {
	//  Build each item
	for (int row = 0; row < m_property->count; row++) {
		indigo_item& i = m_property->items[row];

		QIndigoLight* light = new QIndigoLight(this, m_property, &i);
		m_controls[row] = light;

		layout->addWidget(light);
	}
}

void QIndigoProperty::build_blob_property_form(QVBoxLayout* layout) {
	for (int row = 0; row < m_property->count; row++) {
		indigo_item& i = m_property->items[row];

		QIndigoBLOB* blob = new QIndigoBLOB(this, m_property, &i);
		m_controls[row] = blob;

		layout->addWidget(blob);
		//  Create a button box
		QHBoxLayout* hbox = new QHBoxLayout();
		layout->addLayout(hbox);
		hbox->setAlignment(Qt::AlignRight);
		hbox->setSpacing(10);
		hbox->setMargin(0);

		//  Add Display button
		QPushButton *displayb = new QPushButton("Preview...");
		displayb->setDefault(true);
		displayb->setMinimumWidth(75);
		hbox->addWidget(displayb);

		//  Add Save button
		QPushButton* saveb = new QPushButton("Save BLOB");
		saveb->setDefault(true);
		saveb->setMinimumWidth(75);
		hbox->addWidget(saveb);

		//  Connect signals
		connect(saveb, &QPushButton::clicked, blob, &QIndigoBLOB::save_blob_item);
		connect(displayb, &QPushButton::clicked, blob, &QIndigoBLOB::preview_blob_item);
	}

	//  We want to button press signal to cause the form fields to update to the property and send on bus
	//  To do this, we have to know which property is selected, and then we find the PropertyNode for it.
	//  We then run through the ItemNodes and copy any modified fields to the Items.
	//  Finally we send a property update on the bus.
}

void QIndigoProperty::build_buttons(QVBoxLayout* layout) {
	//  Buttons only for TEXT and NUMBER
	if (m_property->type != INDIGO_TEXT_VECTOR && m_property->type != INDIGO_NUMBER_VECTOR)
		return;

	//  No buttons if RO
	if (m_property->perm == INDIGO_RO_PERM)
		return;

	//  Create a button box
	QHBoxLayout* hbox = new QHBoxLayout();
	layout->addLayout(hbox);
	hbox->setAlignment(Qt::AlignRight);
	hbox->setSpacing(10);
	hbox->setMargin(0);

	//  Add buttons
	QPushButton* set = new QPushButton("Set");
	QPushButton* reset = new QPushButton("Reset");
	set->setDefault(true);
	set->setMinimumWidth(75);
	reset->setMinimumWidth(75);
	hbox->addWidget(reset);
	hbox->addWidget(set);

	//  Connect signals
	connect(set, &QPushButton::clicked, this, &QIndigoProperty::set_clicked);
	connect(reset, &QPushButton::clicked, this, &QIndigoProperty::reset_clicked);
}
