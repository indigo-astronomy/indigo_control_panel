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
	text = new QLineEdit();
	text->setObjectName("INDIGO_property");
	if (m_property->perm == INDIGO_RO_PERM) text->setReadOnly(true);
	update();

	//  Lay the labels out somehow in the widget
	QHBoxLayout* hbox = new QHBoxLayout();
	setLayout(hbox);
	hbox->setAlignment(Qt::AlignLeft);
	hbox->setMargin(0);
	hbox->setSpacing(0);
	hbox->addWidget(label, 35);
	hbox->addWidget(text, 65);

	connect(text, &QLineEdit::textEdited, this, &QIndigoText::dirty);
}

QIndigoText::~QIndigoText() {
	//delete label;
	//delete text;
}

void QIndigoText::update() {
	//  Apply update from indigo bus only if not being edited
	if (!m_dirty) text->setText(m_item->text.value);
}

void QIndigoText::reset() {
	if (m_dirty) {
		m_dirty = false;
		update();
		text->setStyleSheet("color: #FFFFFF");
		text->clearFocus();
	}
}

void QIndigoText::apply() {
	if (m_dirty) {
		strncpy(m_item->text.value, text->text().toUtf8().constData(), sizeof(m_item->text.value));
		reset();
	}
}

void QIndigoText::dirty() {
	//  Set dirty flag
	m_dirty = true;
	//  Colour text orange
	text->setStyleSheet("color: #CCCC00");
}
