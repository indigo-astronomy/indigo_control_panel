// Copyright (c) 2019 Rumen G.Bogdanovski
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

#if !defined(INDIGO_WINDOWS)
#define USE_LIBJPEG
#endif

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <QUrl>
#include <QDir>
#include <QDesktopServices>
#include <QStandardPaths>
#include <QHBoxLayout>
#include "qindigoblob.h"
#include "conf.h"
#include <QPixmap>
#include <QBitmap>

#if defined(USE_LIBJPEG)
#include <jpeglib.h>
#endif

QIndigoBLOB::QIndigoBLOB(QIndigoProperty* p, indigo_property* property, indigo_item* item, QWidget *parent)
	: QWidget(parent), QIndigoItem(p, property, item), m_dirty(false) {

	m_logger = &Logger::instance();
	label = new QLabel(m_item->label);
	label->setObjectName("INDIGO_property");

	image = new QLabel();
	image->setObjectName("INDIGO_property");

	text = new QLineEdit();
	char tooltip[1600];
	snprintf(tooltip, sizeof(tooltip), "%s URL: read only (in legacy mode is empty)", m_item->label);
	text->setToolTip(tooltip);
	text->setObjectName("INDIGO_property");
	if (m_property->perm == INDIGO_RO_PERM) {
		text->setReadOnly(true);
	}
	update();

	//  Lay the labels out somehow in the widget
	QVBoxLayout* vbox = new QVBoxLayout();
	setLayout(vbox);
	QHBoxLayout* hbox = new QHBoxLayout();
	QWidget* base = new QWidget();
	base->setObjectName("INDIGO_property");
	base->setLayout(hbox);
	hbox->setAlignment(Qt::AlignLeft);
	hbox->setMargin(0);
	hbox->setSpacing(0);
	hbox->addWidget(label, 20);
	hbox->addWidget(text, 80);

	vbox->setAlignment(Qt::AlignHCenter);
	vbox->setMargin(0);
	vbox->setSpacing(10);
	vbox->addWidget(image);
	vbox->setAlignment(image, Qt::AlignHCenter);
	vbox->addWidget(base);

	connect(text, &QLineEdit::textEdited, this, &QIndigoBLOB::dirty);
}


QImage* QIndigoBLOB::decompress_jpeg(unsigned char *jpg_buffer, unsigned long jpg_size) {
#if !defined(USE_LIBJPEG)

	QImage* img = new QImage();
	img->loadFromData((const uchar*)jpg_buffer, jpg_size, "JPG");
	return img;

#else // INDIGO Mac and Linux

	int rc;

	unsigned char *bmp_buffer;
	unsigned long bmp_size;

	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	int row_stride, width, height, pixel_size, color_space;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	jpeg_mem_src(&cinfo, jpg_buffer, jpg_size);

	rc = jpeg_read_header(&cinfo, TRUE);

	if (rc != 1) {
		indigo_error("File does not seem to be a normal JPEG");
		return nullptr;
	}

	jpeg_start_decompress(&cinfo);

	width = cinfo.output_width;
	height = cinfo.output_height;
	pixel_size = cinfo.output_components;
	color_space = cinfo.out_color_space;

	bmp_size = width * height * pixel_size;
	bmp_buffer = (unsigned char*)malloc(bmp_size);

	row_stride = width * pixel_size;

	indigo_debug("Proc: Image is %d by %d with %d components (CS: %d)", width, height, pixel_size, color_space);

	while (cinfo.output_scanline < cinfo.output_height) {
		unsigned char *buffer_array[1];
		buffer_array[0] = bmp_buffer + \
						   (cinfo.output_scanline) * row_stride;

		jpeg_read_scanlines(&cinfo, buffer_array, 1);

	}
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	QImage* img;
	if (color_space == JCS_GRAYSCALE) {
		img = new QImage(width, height, QImage::Format_Indexed8);
	} else if (color_space == JCS_RGB) {
		img = new QImage(width, height, QImage::Format_RGB888);
	} else {
		return nullptr;
	}

	for (int y = 0; y < img->height(); y++) {
		memcpy(img->scanLine(y), bmp_buffer + y * img->bytesPerLine(), img->bytesPerLine());
	}

	free(bmp_buffer);
	return img;
#endif
}


QIndigoBLOB::~QIndigoBLOB() {
	//delete label;
	//delete text;
}


void QIndigoBLOB::update() {
	//  Apply update from indigo bus only if not being edited
	if (*m_item->blob.url) {
		text->setText(m_item->blob.url);
		if ((m_property->state == INDIGO_OK_STATE) && (m_item->blob.value != NULL)) {
			QImage* img;
			if (!strcmp(m_item->blob.format, ".jpeg") || !strcmp(m_item->blob.format, ".jpg")) {
				img = decompress_jpeg((unsigned char*)m_item->blob.value, m_item->blob.size);
			} else {
				img = nullptr;
			}
			if (img == nullptr) return;
			QPixmap pixmap = QPixmap::fromImage(*img);
			image->setPixmap(pixmap.scaledToWidth(PREVIEW_WIDTH, Qt::SmoothTransformation));
		}
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


void QIndigoBLOB::save_blob_item() {
	if ((m_property->state == INDIGO_OK_STATE) && (m_item->blob.value != NULL)) {
		char file_name[PATH_LEN];
		char message[PATH_LEN+100];
		char location[PATH_LEN];

		if (QStandardPaths::displayName(QStandardPaths::PicturesLocation).length() > 0) {
			QString qlocation = QDir::toNativeSeparators(QDir::homePath() + tr("/") + QStandardPaths::displayName(QStandardPaths::PicturesLocation));
			strncpy(location, qlocation.toUtf8().constData(), PATH_LEN);
		} else {
			if (!getcwd(location, sizeof(location))) {
				location[0] = '\0';
			}
		}

		if (save_blob_item_with_prefix(location, file_name)) {
			snprintf(message, sizeof(message), "Image saved to '%s'", file_name);
			m_logger->log(NULL, message);
		} else {
			snprintf(message, sizeof(message), "Can not save '%s'", file_name);
			m_logger->log(NULL, message);
		}
	}
}


void QIndigoBLOB::preview_blob_item() {
	if ((m_property->state == INDIGO_OK_STATE) && (m_item->blob.value != nullptr)) {
		char file_name[PATH_LEN];
		char url[PATH_LEN+100];
		char prefix[PATH_LEN] = "/tmp";

#if defined(INDIGO_WINDOWS)
		// Get temp with url dir separators instead of windows native
		strcpy(prefix, qgetenv("TEMP").constData());
#endif
		indigo_debug("PREFIX: %s\n", prefix);
		if (save_blob_item_with_prefix(prefix, file_name)) {
			snprintf(url, sizeof(url), "file:///%s", file_name);
			// change to url separators instead of windows native (for windows)
			if(QDesktopServices::openUrl(QUrl(QDir::fromNativeSeparators(url)))) {
				return;
			}
		}
		indigo_error("Can not display image '%s'", file_name);
	}
}


bool QIndigoBLOB::save_blob_item_with_prefix(const char *prefix, char *file_name) {
	int fd;
	int file_no = 0;

	do {

#if defined(INDIGO_WINDOWS)
		sprintf(file_name, "%s\\blob_%03d%s", prefix, file_no++, m_item->blob.format);
		fd = open(file_name, O_CREAT | O_WRONLY | O_EXCL | O_BINARY, 0);
#else
		sprintf(file_name, "%s/blob_%03d%s", prefix, file_no++, m_item->blob.format);
		fd = open(file_name, O_CREAT | O_WRONLY | O_EXCL, S_IRUSR | S_IWUSR);
#endif
	} while ((fd < 0) && (errno == EEXIST));

	if (fd < 0) {
		return false;
	} else {
		write(fd, m_item->blob.value, m_item->blob.size);
		close_fd(fd);
	}
	return true;
}
