// Copyright (c) 2019 Rumen G.Bogdanovski & David Hulse
// All rights reserved.
//
// You can use this software under the terms of 'INDIGO Astronomy
// open-source license' (see LICENSE.md).
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS 'AS IS' AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


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
#include "conf.h"


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
		this->setStyleSheet(
			"#INDIGO_property { background-color: #272727; border: 0px}"
			"QLineEdit#INDIGO_property { background-color: #222222}"
			"QPushButton#INDIGO_property { background-color: #323232 }"
			"QPushButton#INDIGO_property:focus { background-color: #393939 }"
		);
		break;
	case INDIGO_BUSY_STATE:
		if (conf.use_state_icons)
			m_led->setPixmap(QPixmap(":resource/led-orange-cb.png"));
		else
			m_led->setPixmap(QPixmap(":resource/led-orange.png"));
		this->setStyleSheet(
			"#INDIGO_property { background-color: #353520; border: 0px}"
			"QLineEdit#INDIGO_property { background-color: #252520}"
			"QPushButton#INDIGO_property { background-color: #454522 }"
			"QPushButton#INDIGO_property:focus { background-color: #505022 }"
		);
		break;
	case INDIGO_ALERT_STATE:
		if (conf.use_state_icons)
			m_led->setPixmap(QPixmap(":resource/led-red-cb.png"));
		else
			m_led->setPixmap(QPixmap(":resource/led-red.png"));
		this->setStyleSheet(
			"#INDIGO_property { background-color: #352222; border: 0px}"
			"QLineEdit#INDIGO_property { background-color: #252222}"
			"QPushButton#INDIGO_property { background-color: #452222 }"
			"QPushButton#INDIGO_property:focus { background-color: #502222 }"
		);
		break;
	case INDIGO_OK_STATE:
		if (conf.use_state_icons)
			m_led->setPixmap(QPixmap(":resource/led-green-cb.png"));
		else
			m_led->setPixmap(QPixmap(":resource/led-green.png"));
		this->setStyleSheet(
			"#INDIGO_property { background-color: #203220; border: 0px}"
			"QLineEdit#INDIGO_property { background-color: #202520}"
			"QPushButton#INDIGO_property { background-color: #224322 }"
			"QPushButton#INDIGO_property:focus { background-color: #225022 }"
		);
		break;
	}
	m_led->update();
}


void QIndigoProperty::update() {
	//  Update all the controls to the current state
	update_controls();
	update_property_view();

	//  Update the property on the bus
	m_property->access_token = indigo_get_device_or_master_token(m_property->device);
	indigo_change_property(nullptr, m_property);
}


void QIndigoProperty::property_update(indigo_property* property) {
	//  Ignore updates to properties that this widget doesn't represent
	if (m_property != property)
		return;

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
	frame->setObjectName("INDIGO_property");

	QVBoxLayout* vbox = new QVBoxLayout();
	vbox->addWidget(frame);
	layout->addLayout(vbox);

	QVBoxLayout* frame_layout = new QVBoxLayout();
	frame->setLayout(frame_layout);

	QWidget *head = new QWidget();
	head->setObjectName("INDIGO_property");
	frame_layout->addWidget(head);
	QHBoxLayout* head_layout = new QHBoxLayout();
	head->setLayout(head_layout);

	head_layout->setAlignment(Qt::AlignLeft);
	head_layout->setMargin(3);
	head_layout->setSpacing(5);

	char device_string[1025];
	sprintf(device_string, "@ %s", m_property->device);
	m_led = new QLabel();
	m_led->setObjectName("INDIGO_property");
	QLabel* property_label = new QLabel(m_property->label);
	property_label->setStyleSheet("font-weight: bold;");
	property_label->setObjectName("INDIGO_property");
	QLabel* device_label = new QLabel(device_string);
	device_label->setObjectName("INDIGO_property");

	head_layout->addWidget(m_led);
	head_layout->addWidget(property_label);
	//head_layout->addStretch();
	head_layout->addWidget(device_label);

	QWidget *property_setings = new QWidget();
	property_setings->setObjectName("INDIGO_property");
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
		QPushButton *displayb = new QPushButton("&View...");
		displayb->setToolTip("View BLOB with the default viewer");
		displayb->setObjectName("INDIGO_property");
		displayb->setDefault(true);
		displayb->setMinimumWidth(75);
		hbox->addWidget(displayb);

		//  Add Save button
		QPushButton* saveb = new QPushButton("&Save BLOB");
		saveb->setToolTip("Save BLOB to the default Pictures folder");
		saveb->setObjectName("INDIGO_property");
		saveb->setDefault(true);
		saveb->setMinimumWidth(75);
		hbox->addWidget(saveb);

		//  Connect signals
		connect(saveb, &QPushButton::clicked, blob, &QIndigoBLOB::save_blob_item);
		connect(displayb, &QPushButton::clicked, blob, &QIndigoBLOB::view_blob_item);
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
	QPushButton* set = new QPushButton("&Set");
	set->setObjectName("INDIGO_property");
	QPushButton* reset = new QPushButton("&Reset");
	reset->setObjectName("INDIGO_property");
	set->setDefault(true);
	set->setMinimumWidth(75);
	reset->setMinimumWidth(75);
	hbox->addWidget(reset);
	hbox->addWidget(set);

	//  Connect signals
	connect(set, &QPushButton::clicked, this, &QIndigoProperty::set_clicked);
	connect(reset, &QPushButton::clicked, this, &QIndigoProperty::reset_clicked);
}
