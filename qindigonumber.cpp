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


#include <QHBoxLayout>
#include <indigo/indigo_bus.h>
#include "qindigonumber.h"


QIndigoNumber::QIndigoNumber(QIndigoProperty* p, indigo_property* property, indigo_item* item, QWidget *parent)
	: QWidget(parent), QIndigoItem(p, property, item) {

	label = new QLabel(m_item->label);
	label->setObjectName("INDIGO_property");
	text_value = new QLineEdit();
	char tooltip[2100];
	snprintf(tooltip, sizeof(tooltip), "%s value, read only", m_item->label);
	text_value->setToolTip(tooltip);
	text_value->setObjectName("INDIGO_property");
	text_value->setReadOnly(true);
	text_target = nullptr;
	m_dirty = false;
	if (m_property->perm != INDIGO_RO_PERM) {
		text_target = new QLineEdit();
		if (m_item->number.format[strlen(m_item->number.format) - 1] == 'm') {
			snprintf (
				tooltip,
				sizeof(tooltip),
				"%s, range: [%s, %s] step: %s",
				m_item->label, indigo_dtos(m_item->number.min, NULL),
				indigo_dtos(m_item->number.max, NULL),
				indigo_dtos(m_item->number.step, NULL)
			);
		} else {
			char format[1600];
			snprintf (
				format,
				sizeof(format),
				"%s, range: [%s, %s] step: %s",
				m_item->label, m_item->number.format,
				m_item->number.format,
				m_item->number.format
			);
			snprintf (
				tooltip,
				sizeof(tooltip),
				format, m_item->number.min,
				m_item->number.max,
				m_item->number.step
			);
		}
		text_target->setToolTip(tooltip);
		text_target->setObjectName("INDIGO_property");
	}
	update();

	//  Lay the labels out somehow in the widget
	QHBoxLayout* hbox = new QHBoxLayout();
	setLayout(hbox);
	hbox->setAlignment(Qt::AlignLeft);
	hbox->setMargin(0);
	hbox->setSpacing(0);
	hbox->addWidget(label, 35);
	if (m_property->perm == INDIGO_RO_PERM)
		hbox->addWidget(text_value, 33);
	else {
		hbox->addWidget(text_value, 16);
		hbox->addSpacing(5);
		hbox->addWidget(text_target, 16);
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
		snprintf(buffer, sizeof(buffer), m_item->number.format, m_item->number.value);
	}
	text_value->setText(buffer);
	if (text_target && !m_dirty && !text_target->hasFocus()) {
		if (m_item->number.format[strlen(m_item->number.format) - 1] == 'm')
			strncpy(buffer, indigo_dtos(m_item->number.target, NULL), sizeof(buffer));
		else
			snprintf(buffer, sizeof(buffer), m_item->number.format, m_item->number.target);
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
	if (text_target) {
		m_item->number.value = m_item->number.target = indigo_stod((char*)text_target->text().trimmed().toStdString().c_str());
		reset();
	}
}

void QIndigoNumber::dirty() {
	if (text_target) {
		//  Set dirty flag
		m_dirty = true;

		//  Colour text orange
		text_target->setStyleSheet("color: #CCCC00");
	}
}
