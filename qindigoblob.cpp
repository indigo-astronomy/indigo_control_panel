#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
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

/* C++ looks for method close - maybe name collision so... */
void close_fd(int fd) {
	close(fd);
}

void QIndigoBLOB::save_blob_item(){
	if ((m_property->state == INDIGO_OK_STATE) && (m_item->blob.value != NULL)) {
		char file_name[255];
		int fd;
		int file_no = 0;

		do {
			sprintf(file_name, "blob_%03d.%s", file_no++, m_item->blob.format);
			fd = open(file_name, O_CREAT | O_WRONLY | O_EXCL, S_IRUSR | S_IWUSR);
		} while ((fd < 0) && (errno == EEXIST));

		if (fd < 0) {
			indigo_log("Can not save %s...", file_name);
		} else {
			write(fd, m_item->blob.value, m_item->blob.size);
			close_fd(fd);
			indigo_log("ITEM image saved to %s...", file_name);
		}
	}
}
