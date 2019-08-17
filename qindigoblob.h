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


#ifndef QINDIGOBLOB_H
#define QINDIGOBLOB_H

#include <QLabel>
#include <QLineEdit>
#include <QWidget>
#include "qindigoswitch.h"
#include "logger.h"


class QIndigoBLOB : public QWidget, public QIndigoItem {
	Q_OBJECT
public:
	explicit QIndigoBLOB(QIndigoProperty* p, indigo_property* property, indigo_item* item, QWidget *parent = nullptr);
	virtual ~QIndigoBLOB();

signals:

public slots:
	void dirty();
	void save_blob_item();
	void preview_blob_item();

private:
	Logger* m_logger;
	QLabel* label;
	QLabel* image;
	QLineEdit* text;
	QImage* preview;
	bool m_dirty;

	virtual void update();
	virtual void reset();
	virtual void apply();
	bool save_blob_item_with_prefix(const char *prefix, char *file_name);
	QImage* process_jpeg(unsigned char *jpg_buffer, unsigned long jpg_size);
	QImage* process_fits(unsigned char *fits_buffer, unsigned long fits_size);
	QImage* process_raw(unsigned char *raw_image_buffer, unsigned long raw_size);
	QImage* generate_preview(int width, int height, int pixel_format, char *image_data, int *hist, double white_threshold);
};

#endif // QINDIGOBLOB_H
