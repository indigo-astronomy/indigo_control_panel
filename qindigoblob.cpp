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
#include <fits/fits.h>
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

	preview = nullptr;

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


QIndigoBLOB::~QIndigoBLOB() {
	if (preview) delete preview;
	//delete label;
	//delete text;
}


void QIndigoBLOB::update() {
	//  Apply update from indigo bus only if not being edited
	if (*m_item->blob.url) {
		text->setText(m_item->blob.url);
		if ((m_property->state == INDIGO_OK_STATE) && (m_item->blob.value != NULL) && (preview == nullptr)) {
			if (!strcmp(m_item->blob.format, ".jpeg") ||
			    !strcmp(m_item->blob.format, ".jpg")) {
				preview = process_jpeg((unsigned char*)m_item->blob.value, m_item->blob.size);
			}else if (!strcmp(m_item->blob.format, ".fits") ||
			          !strcmp(m_item->blob.format, ".fit")) {
				preview = process_fits((unsigned char*)m_item->blob.value, m_item->blob.size);
			}else if (!strcmp(m_item->blob.format, ".raw")) {
				preview = process_raw((unsigned char*)m_item->blob.value, m_item->blob.size);
			} else {
				QPixmap pixmap(":resource/no-preview.png");
				image->setPixmap(pixmap.scaledToWidth(PREVIEW_WIDTH, Qt::SmoothTransformation));
				return;
			}
			if (preview == nullptr) return;
			QPixmap pixmap = QPixmap::fromImage(*preview);
			image->setPixmap(pixmap.scaledToWidth(PREVIEW_WIDTH, Qt::SmoothTransformation));
			return;
		}
	}
	delete(preview);
	preview = nullptr;
	QPixmap pixmap(":resource/no-preview.png");
	image->setPixmap(pixmap.scaledToWidth(PREVIEW_WIDTH, Qt::SmoothTransformation));
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


QImage* QIndigoBLOB::process_jpeg(unsigned char *jpg_buffer, unsigned long jpg_size) {
#if !defined(USE_LIBJPEG)

	QImage* img = new QImage();
	img->loadFromData((const uchar*)jpg_buffer, jpg_size, "JPG");
	return img;

#else // INDIGO Mac and Linux

	unsigned char *bmp_buffer;
	unsigned long bmp_size;

	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	int row_stride, width, height, pixel_size, color_space;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	jpeg_mem_src(&cinfo, jpg_buffer, jpg_size);

	int rc = jpeg_read_header(&cinfo, TRUE);
	if (rc != 1) {
		indigo_error("JPEG: Data does not seem to be JPEG");
		return nullptr;
	}
	jpeg_start_decompress(&cinfo);

	width = cinfo.output_width;
	height = cinfo.output_height;
	pixel_size = cinfo.output_components;
	color_space = cinfo.out_color_space;

	bmp_size = width * height * pixel_size;
	bmp_buffer = (unsigned char*)malloc(bmp_size);

	indigo_debug("JPEG: Image is %d x %d (BPP: %d CS: %d)", width, height, pixel_size*8, color_space);

	row_stride = width * pixel_size;
	while (cinfo.output_scanline < cinfo.output_height) {
		unsigned char *buffer_array[1];
		buffer_array[0] = bmp_buffer + (cinfo.output_scanline) * row_stride;
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
		indigo_error("JPEG: Unsupported colour space (CS: %d)", color_space);
		return nullptr;
	}

	for (int y = 0; y < img->height(); y++) {
		memcpy(img->scanLine(y), bmp_buffer + y * img->bytesPerLine(), img->bytesPerLine());
	}

	free(bmp_buffer);
	return img;
#endif
}


QImage* QIndigoBLOB::process_fits(unsigned char *raw_fits_buffer, unsigned long fits_size) {
	fits_header header;
	int *hist;
	pixel_format pix_format;

	int res = fits_read_header(raw_fits_buffer, fits_size, &header);
	if (res != FITS_OK) {
		indigo_error("FITS: Error parsing header");
		return nullptr;
	}

	if ((header.bitpix==16) && (header.naxis == 2)){
		hist = (int*)malloc(65536*sizeof(int));
		pix_format = MONO_16;
	} else if ((header.bitpix==16) && (header.naxis == 3)){
		hist = (int*)malloc(65536*sizeof(int));
		pix_format = FITS_RGB_48;
	} else if ((header.bitpix==8) && (header.naxis == 2)){
		hist = (int*)malloc(256*sizeof(int));
		pix_format = MONO_8;
	} else if ((header.bitpix==8) && (header.naxis == 3)){
		hist = (int*)malloc(256*sizeof(int));
		pix_format = FITS_RGB_24;
	} else {
		indigo_error("FITS: Unsupported bitpix (BITPIX= %d)", header.bitpix);
		return nullptr;
	}

	char *fits_data = (char*)malloc(fits_get_buffer_size(&header));

	res = fits_process_data_with_hist(raw_fits_buffer, fits_size, &header, fits_data, hist);
	if (res != FITS_OK) {
		indigo_error("FITS: Error processing data");
		return nullptr;
	}

	QImage *img = generate_preview(header.naxisn[0], header.naxisn[1], pix_format, fits_data, hist, 0.005);

	free(hist);
	free(fits_data);
	return img;
}


QImage* QIndigoBLOB::process_raw(unsigned char *raw_image_buffer, unsigned long raw_size) {
	int *hist;
	pixel_format pix_format;
	int bitpix;

	if (sizeof(indigo_raw_header) > raw_size) {
		indigo_error("RAW: Image buffer is too short: can not fit the header (%dB)", raw_size);
		return nullptr;
	}

	indigo_raw_header *header = (indigo_raw_header*)raw_image_buffer;
	char *raw_data = (char*)raw_image_buffer + sizeof(indigo_raw_header);

	int pixel_count = header->height * header->width;

	switch (header->signature) {
	case INDIGO_RAW_MONO16:
		pix_format = MONO_16;
		bitpix = 16;
		break;
	case INDIGO_RAW_MONO8:
		pix_format = MONO_8;
		bitpix = 8;
		break;
	case INDIGO_RAW_RGB24:
		pix_format = RAW_RGB_24;
		bitpix = 8;
		break;
	case INDIGO_RAW_RGB48:
		pix_format = RAW_RGB_48;
		bitpix = 16;
		break;
	default:
		indigo_error("RAW: Unsupported image format (%d)", header->signature);
		return nullptr;
	}

	if ((pixel_count * bitpix / 8 + sizeof(indigo_raw_header)) > raw_size) {
		indigo_error("RAW: Image buffer is too short: can not fit the image (%dB)", raw_size);
		return nullptr;
	}

	if ((header->signature == INDIGO_RAW_MONO16) ||
	    (header->signature == INDIGO_RAW_RGB48)) {
		hist = (int*)malloc(65536*sizeof(int));
		memset(hist, 0, 65536*sizeof(int));
		uint16_t* buf = (uint16_t*)raw_data;
		for (int pix = 0; pix < pixel_count; ++pix) {
			hist[*buf++]++;
		}
	} else if ((header->signature == INDIGO_RAW_MONO8) ||
	           (header->signature == INDIGO_RAW_RGB24)) {
		hist = (int*)malloc(256*sizeof(int));
		memset(hist, 0, 256*sizeof(int));
		uint8_t* buf = (uint8_t*)raw_data;
		for (int pix = 0; pix < pixel_count; ++pix) {
			hist[*buf++]++;
		}
	} else {
		// should not happen - handled above
		return nullptr;
	}

	QImage *img = generate_preview(header->width, header->height, pix_format, raw_data, hist, 0.005);

	free(hist);
	return img;
}


QImage* QIndigoBLOB::generate_preview(int width, int height, int pix_format, char *image_data, int *hist, double white_threshold) {
	int range, max, min = 0, sum;
	int pix_cnt = width * height;
	int thresh = white_threshold * pix_cnt;

	switch (pix_format) {
	case MONO_8:
	case RAW_RGB_24:
	case FITS_RGB_24:
		max = 255;
		break;
	case MONO_16:
	case RAW_RGB_48:
	case FITS_RGB_48:
		max = 65535;
		break;
	default:
		indigo_error("PREVIEW: Unsupported pixel format (%d)", pix_format);
		return nullptr;
	}

	sum = hist[max];
	while (sum < thresh) {
		sum += hist[--max];
	}
	min = 0;
	while (hist[min] == 0) {
		min++;
	};

	range = max - min;
	double scale = 256.0 / range;

	indigo_debug("PREVIEW: pix_format = %d sum = %d thresh = %d max = %d min = %d", pix_format, sum, thresh, max, min);

	QImage* img = new QImage(width, height, QImage::Format_RGB888);
	if (pix_format == MONO_8) {
		uint8_t* buf = (uint8_t*)image_data;
		int index = 0;
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				int value = buf[index++] - min;
				if (value >= range) value = 255;
				else value *= scale;
				img->setPixel(x, y, qRgb(value, value, value));
			}
		}
	} else if (pix_format == MONO_16) {
		uint16_t* buf = (uint16_t*)image_data;
		int index = 0;
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				int value = buf[index++] - min;
				if (value >= range) value = 255;
				else value *= scale;
				img->setPixel(x, y, qRgb(value, value, value));
			}
		}
	} else if (pix_format == FITS_RGB_24) {
		int channel_offest = width * height;
		uint8_t* buf = (uint8_t*)image_data;
		int index = 0;
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				int value_r = buf[index] - min;
				int value_g = buf[index + channel_offest] - min;
				int value_b = buf[index + 2 * channel_offest] - min;
				index++;
				if (value_r >= range) value_r = 255;
				else value_r *= scale;
				if (value_g >= range) value_g = 255;
				else value_g *= scale;
				if (value_b >= range) value_b = 255;
				else value_b *= scale;
				img->setPixel(x, y, qRgb(value_r, value_g, value_b));
			}
		}
	} else if (pix_format == FITS_RGB_48) {
		int channel_offest = width * height;
		uint16_t* buf = (uint16_t*)image_data;
		int index = 0;
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				int value_r = buf[index] - min;
				int value_g = buf[index + channel_offest] - min;
				int value_b = buf[index + 2 * channel_offest] - min;
				index++;
				if (value_r >= range) value_r = 255;
				else value_r *= scale;
				if (value_g >= range) value_g = 255;
				else value_g *= scale;
				if (value_b >= range) value_b = 255;
				else value_b *= scale;
				img->setPixel(x, y, qRgb(value_r, value_g, value_b));
			}
		}
	} else if (pix_format == RAW_RGB_24) {
		uint8_t* buf = (uint8_t*)image_data;
		int index = 0;
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				int value_r = buf[index++] - min;
				int value_g = buf[index++] - min;
				int value_b = buf[index++] - min;

				if (value_r >= range) value_r = 255;
				else value_r *= scale;
				if (value_g >= range) value_g = 255;
				else value_g *= scale;
				if (value_b >= range) value_b = 255;
				else value_b *= scale;
				img->setPixel(x, y, qRgb(value_r, value_g, value_b));
			}
		}
	} else if (pix_format == RAW_RGB_48) {
		uint16_t* buf = (uint16_t*)image_data;
		int index = 0;
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				int value_r = buf[index++] - min;
				int value_g = buf[index++] - min;
				int value_b = buf[index++] - min;

				if (value_r >= range) value_r = 255;
				else value_r *= scale;
				if (value_g >= range) value_g = 255;
				else value_g *= scale;
				if (value_b >= range) value_b = 255;
				else value_b *= scale;
				img->setPixel(x, y, qRgb(value_r, value_g, value_b));
			}
		}
	} else {
		indigo_error("PREVIEW: Unsupported pixel format (%d)", pix_format);
		return nullptr;
	}
	return img;
}
