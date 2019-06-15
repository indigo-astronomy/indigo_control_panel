#include <QHBoxLayout>
#include "qindigoblob.h"


QIndigoBLOB::QIndigoBLOB(QIndigoProperty* p, indigo_property* property, indigo_item* item, QWidget *parent)
	: QWidget(parent), QIndigoItem(p, property, item), m_dirty(false) {
	label = new QLabel(m_item->label);
	text = new QLineEdit();
	if (m_property->perm == INDIGO_RO_PERM)
		text->setReadOnly(true);
	update();

	//  Lay the labels out somehow in the widget
	QHBoxLayout* hbox = new QHBoxLayout();
	setLayout(hbox);
	hbox->setAlignment(Qt::AlignLeft);
	hbox->setMargin(0);
	hbox->setSpacing(0);
	hbox->addWidget(label, 20);
	hbox->addWidget(text, 80);

	connect(text, &QLineEdit::textEdited, this, &QIndigoBLOB::dirty);
}


QIndigoBLOB::~QIndigoBLOB() {
	//delete label;
	//delete text;
}


void QIndigoBLOB::update() {
	//  Apply update from indigo bus only if not being edited
	if (*m_item->blob.url) {
		text->setText(m_item->blob.url);
	}
}


void QIndigoBLOB::reset() {
	if (m_dirty) {
		m_dirty = false;
		update();
		text->setStyleSheet("color: #FFFFFF");
		text->clearFocus();
	}
}


void QIndigoBLOB::apply() {
	if (m_dirty) {
		strncpy(m_item->text.value, text->text().toUtf8().constData(), sizeof(m_item->text.value));
		reset();
	}
}


void QIndigoBLOB::dirty() {
	//  Set dirty flag
	m_dirty = true;

	//  Colour text red
	text->setStyleSheet("color: #CC0000");
}


void QIndigoBLOB::save_blob_item(){
	if ((m_property->state == INDIGO_OK_STATE) && (m_item->blob.value != NULL)) {
		char name[32];

		if(*m_item->blob.url) {
			strncpy(name, basename(m_item->blob.url), 32);
		} else {
			sprintf(name, "last.fits");
		}
		FILE *f = fopen(name, "wb");
		fwrite(m_item->blob.value, m_item->blob.size, 1, f);
		fclose(f);
		indigo_log("ITEM image saved to %s...", name);
	}
}
