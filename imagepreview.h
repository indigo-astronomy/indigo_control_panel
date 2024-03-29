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
// NEGLIGENCEIMAGEOTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, IMAGE IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef _IMAGEPREVIEW_H
#define _IMAGEPREVIEW_H

#include <QImage>
#include <QHash>
#include <indigo/indigo_client.h>
#include <pixelformat.h>
#include <image_preview_lut.h>
#include <stretcher.h>

#if !defined(INDIGO_WINDOWS)
#define USE_LIBJPEG
#endif
#if defined(USE_LIBJPEG)
#include <jpeglib.h>
#endif

class preview_image: public QImage {
public:
	preview_image():
		m_raw_data(nullptr),
		m_histogram(nullptr),
		m_width(0),
		m_height(0),
		m_pix_format(0)
	{};

	//preview_image(preview_image &&other) = delete;
	//preview_image(const uchar *data, int width, int height, int bytesPerLine, QImage::Format format, QImageCleanupFunction cleanupFunction = nullptr, void *cleanupInfo = nullptr) = delete;
	//preview_image(const char *const xpm[]) = delete;
	//preview_image(uchar *data, int width, int height, int bytesPerLine, QImage::Format format, QImageCleanupFunction cleanupFunction = nullptr, void *cleanupInfo = nullptr)= delete;
	//preview_image(const uchar *data, int width, int height, QImage::Format format, QImageCleanupFunction cleanupFunction = nullptr, void *cleanupInfo = nullptr)= delete;
	//preview_image(uchar *data, int width, int height, QImage::Format format, QImageCleanupFunction cleanupFunction = nullptr, void *cleanupInfo = nullptr)= delete;
	//preview_image(const QSize &size, QImage::Format format)= delete;

	preview_image(int width, int height, QImage::Format format):
		QImage(width, height, format),
		m_raw_data(nullptr),
		m_histogram(nullptr),
		m_width(0),
		m_height(0),
		m_pix_format(0)
	{};

	preview_image(preview_image &image): QImage(image) {
		int size = 0;
		int hist_size = 0;
		m_width = image.m_width;
		m_height = image.m_height;
		m_pix_format = image.m_pix_format;

		if (image.m_raw_data == nullptr) {
			m_raw_data = nullptr;
			m_histogram = nullptr;
			return;
		}

		if (m_pix_format == PIX_FMT_Y8) {
			size = m_width * m_height;
			hist_size = 256;
		} else if (m_pix_format == PIX_FMT_Y16) {
			size = m_width * m_height * 2;
			hist_size = 65536;
		} else if (m_pix_format == PIX_FMT_RGB24) {
			size = m_width * m_height * 3;
			hist_size = 256;
		} else if (m_pix_format == PIX_FMT_RGB48) {
			size = m_width * m_height * 6;
			hist_size = 65536;
		}

		if (size == 0) {
			m_raw_data = nullptr;
			m_histogram = nullptr;
			return;
		}

		m_raw_data = (char*)malloc(size);
		memcpy(m_raw_data, image.m_raw_data, size);

		if (image.m_histogram) {
			m_histogram = (int*)malloc(hist_size * sizeof(int));
			memcpy(m_histogram, image.m_histogram, hist_size * sizeof(int));
		} else {
			m_histogram = nullptr;
			return;
		}
	};

	preview_image& operator=(preview_image &image) {
		QImage::operator=(image);
		int size = 0;
		int hist_size = 0;
		m_width = image.m_width;
		m_height = image.m_height;
		m_pix_format = image.m_pix_format;

		if (image.m_raw_data == nullptr) {
			if (m_raw_data) free(m_raw_data);
			m_raw_data = nullptr;
			if (m_histogram) free(m_histogram);
			m_histogram = nullptr;
			return *this;
		}

		if (m_pix_format == PIX_FMT_Y8) {
			size = m_width * m_height;
			hist_size = 256;
		} else if (m_pix_format == PIX_FMT_Y16) {
			size = m_width * m_height * 2;
			hist_size = 65536;
		} else if (m_pix_format == PIX_FMT_RGB24) {
			size = m_width * m_height * 3;
			hist_size = 256;
		} else if (m_pix_format == PIX_FMT_RGB48) {
			size = m_width * m_height * 6;
			hist_size = 65536;
		}

		if (m_raw_data) free(m_raw_data);
		if (m_histogram) free(m_histogram);

		if (size == 0) {
			m_raw_data = nullptr;
			m_histogram = nullptr;
			return *this;
		}

		m_raw_data = (char*)malloc(size);
		memcpy(m_raw_data, image.m_raw_data, size);
		if (image.m_histogram) {
			m_histogram = (int*)malloc(hist_size * sizeof(int));
			memcpy(m_histogram, image.m_histogram, hist_size * sizeof(int));
		} else {
			m_histogram = nullptr;
		}
		return *this;
	}

	~preview_image() {
		if (m_raw_data != nullptr) {
			free(m_raw_data);
			m_raw_data = nullptr;
		}
		if (m_histogram != nullptr) {
			free(m_histogram);
			m_histogram = nullptr;
		}
	};

	int pixel_value(int x, int y, int &r, int &g, int &b) const {
		if (m_raw_data == nullptr) {
			if (pixelFormat().colorModel() == 2) {
				r = g = b = -1;
				return PIX_FMT_INDEX;
			}
			QRgb rgb = pixel(x,y);
			r = qRed(rgb);
			g = qGreen(rgb);
			b = qBlue(rgb);
			return PIX_FMT_RGB24;
		}

		if (x < 0 || x >= m_width || y < 0 || y >= m_height) return 0;
		if (m_pix_format == PIX_FMT_Y8) {
			uint8_t* pixels = (uint8_t*) m_raw_data;
			r = pixels[y * m_width + x];
			g = -1;
			b = -1;
		} else if (m_pix_format == PIX_FMT_Y16) {
			uint16_t* pixels = (uint16_t*) m_raw_data;
			r = pixels[y * m_width + x];
			g = -1;
			b = -1;
		} else if (m_pix_format == PIX_FMT_RGB24) {
			uint8_t* pixels = (uint8_t*) m_raw_data;
			r = pixels[3 * (y * m_width + x)];
			g = pixels[3 * (y * m_width + x) + 1];
			b = pixels[3 * (y * m_width + x) + 2];
		} else if (m_pix_format == PIX_FMT_RGB48) {
			uint16_t* pixels = (uint16_t*) m_raw_data;
			r = pixels[3 * (y * m_width + x)];
			g = pixels[3 * (y * m_width + x) + 1];
			b = pixels[3 * (y * m_width + x) + 2];
		}
		return m_pix_format;
	};

	char *m_raw_data;
	int *m_histogram;
	int m_width;
	int m_height;
	int m_pix_format;
	StretchParams m_strech_params;
};

int get_bayer_offsets(uint32_t pix_format);
template <typename T> void parallel_debayer(T *input_buffer, int width, int height, int offsets, T *output_buffer);

preview_image* create_jpeg_preview(unsigned char *jpg_buffer, unsigned long jpg_size);
preview_image* create_fits_preview(unsigned char *fits_buffer, unsigned long fits_size, const stretch_config_t sconfig);
preview_image* create_raw_preview(unsigned char *raw_image_buffer, unsigned long raw_size, const stretch_config_t sconfig);
preview_image* create_preview(unsigned char *data, size_t size, const char* format, const stretch_config_t sconfig);
preview_image* create_preview(int width, int height, int pixel_format, char *image_data, const stretch_config_t sconfig);
preview_image* create_preview(indigo_property *property, indigo_item *item, const stretch_config_t sconfig);
preview_image* create_preview(indigo_item *item, const stretch_config_t sconfig);
void stretch_preview(preview_image *img, const stretch_config_t sconfig);

#endif /* _IMAGEPREVIEW_H */
