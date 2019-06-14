#include <QHBoxLayout>
#include <indigo_bus.h>
#include "qindigonumber.h"


QIndigoNumber::QIndigoNumber(QIndigoProperty* p, indigo_property* property, indigo_item* item, QWidget *parent)
	: QWidget(parent), QIndigoItem(p, property, item) {

	label = new QLabel(m_item->label);
	text_value = new QLineEdit();
	text_value->setReadOnly(true);
	text_target = nullptr;
	m_dirty = false;
	if (m_property->perm != INDIGO_RO_PERM)
		text_target = new QLineEdit();
	update();

	//  Lay the labels out somehow in the widget
	QHBoxLayout* hbox = new QHBoxLayout();
	setLayout(hbox);
	hbox->setAlignment(Qt::AlignLeft);
	hbox->setMargin(0);
	hbox->setSpacing(0);
	hbox->addWidget(label, 35);
	if (m_property->perm == INDIGO_RO_PERM)
		hbox->addWidget(text_value, 65);
	else {
		hbox->addWidget(text_value, 30);
		hbox->addSpacing(5);
		hbox->addWidget(text_target, 30);
		connect(text_target, &QLineEdit::textEdited, this, &QIndigoNumber::dirty);
	}
}

QIndigoNumber::~QIndigoNumber() {
	//delete label;
	//delete text;
}

void QIndigoNumber::update() {
	char buffer[50];

	if (m_item->number.format[strlen(m_item->number.format) - 1] == 'm') {
		strncpy(buffer, indigo_dtos(m_item->number.value, NULL), sizeof(buffer));
	} else {
		snprintf(buffer,  sizeof(buffer), m_item->number.format, m_item->number.value);
	}
	text_value->setText(buffer);
	if (text_target && !m_dirty) {
		if (m_item->number.format[strlen(m_item->number.format) - 1] == 'm')
			strncpy(buffer, indigo_dtos(m_item->number.target, NULL), sizeof(buffer));
		else
			snprintf(buffer, sizeof(buffer), m_item->number.format, m_item->number.value);
		text_target->setText(buffer);
	}
}

void QIndigoNumber::reset() {
	if (text_target && m_dirty) {
		m_dirty = false;
		update();
		text_target->setStyleSheet("color: #FFFFFF");
		text_target->clearFocus();
	}
}

void QIndigoNumber::apply() {
	if (text_target && m_dirty) {
		m_item->number.value = m_item->number.target = indigo_stod((char*)text_target->text().toStdString().c_str());
		reset();
	}
}

void QIndigoNumber::dirty() {
	if (text_target) {
		//  Set dirty flag
		m_dirty = true;

		//  Colour text red
		text_target->setStyleSheet("color: #CC0000");
	}
}
