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
#include "qindigotext.h"


QIndigoText::QIndigoText(QIndigoProperty* p, indigo_property* property, indigo_item* item, QWidget *parent)
	: QWidget(parent), QIndigoItem(p, property, item), m_dirty(false) {

	label = new QLabel(m_item->label);
	label->setObjectName("INDIGO_property");

	if (strncmp(m_item->name, "SCRIPT", INDIGO_NAME_SIZE)) {
		text = new QLineEdit();
		char tooltip[1600];
		text->setObjectName("INDIGO_property");
		if (m_property->perm == INDIGO_RO_PERM) {
			snprintf(tooltip, sizeof(tooltip), "%s: read only", m_item->label);
			text->setToolTip(tooltip);
			text->setReadOnly(true);
		} else {
			snprintf(tooltip, sizeof(tooltip), "%s: editable", m_item->label);
			text->setToolTip(tooltip);
		}
		QHBoxLayout* hbox = new QHBoxLayout();
		setLayout(hbox);
		hbox->setAlignment(Qt::AlignLeft);
		hbox->setMargin(0);
		hbox->setSpacing(0);
		hbox->addWidget(label, 35);
		hbox->addWidget(text, 65);
		connect(text, &QLineEdit::textEdited, this, &QIndigoText::dirty);
	} else {
		text_edit = new QPlainTextEdit();
		text = NULL;
		char tooltip[1600];
		//text_edit->setObjectName("INDIGO_property");
		if (m_property->perm == INDIGO_RO_PERM) {
			snprintf(tooltip, sizeof(tooltip), "%s: read only", m_item->label);
			text_edit->setToolTip(tooltip);
			text_edit->setReadOnly(true);
		} else {
			snprintf(tooltip, sizeof(tooltip), "%s: editable", m_item->label);
			text_edit->setToolTip(tooltip);
		}
		QHBoxLayout* hbox = new QHBoxLayout();
		setLayout(hbox);
		hbox->setAlignment(Qt::AlignLeft);
		hbox->setMargin(0);
		hbox->setSpacing(0);
		hbox->addWidget(label, 15);
		hbox->addWidget(text_edit, 85);
		text_edit->setFixedHeight(350);
		connect(text_edit, &QPlainTextEdit::textChanged, this, &QIndigoText::dirty);
	}
	update();
}

QIndigoText::~QIndigoText() {
	//delete label;
	//delete text;
}

void QIndigoText::update() {
	//  Apply update from indigo bus only if not being edited
	if (!m_dirty) {
		if (text)
			text->setText(m_item->text.value);
		else
			text_edit->setPlainText(m_item->text.value);
	}
}

void QIndigoText::reset() {
	if (m_dirty) {
		m_dirty = false;
		update();
		if (text) {
			text->setStyleSheet("color: #FFFFFF");
			text->clearFocus();
		} else {
			text_edit->setStyleSheet("color: #FFFFFF");
			text_edit->clearFocus();
		}
	}
}

void QIndigoText::apply() {
	if (m_dirty) {
		if (text) {
			strncpy(m_item->text.value, text->text().trimmed().toUtf8().constData(), sizeof(m_item->text.value));
		} else {
			strncpy(m_item->text.value, text_edit->toPlainText().trimmed().toUtf8().constData(), sizeof(m_item->text.value));
		}
		reset();
	}
}

void QIndigoText::dirty() {
	//  Set dirty flag
	m_dirty = true;
	//  Colour text orange
	if (text) {
		text->setStyleSheet("color: #CCCC00");
	} else {
		text_edit->setStyleSheet("color: #CCCC00");
	}
}
