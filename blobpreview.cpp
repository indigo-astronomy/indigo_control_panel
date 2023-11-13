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

#include <math.h>
#include <fits/fits.h>
#include <pixelformat.h>
#include "blobpreview.h"
#include <QPainter>

blob_preview_cache preview_cache;
static preview_stretch preview_stretch_level = STRETCH_NORMAL;

QString blob_preview_cache::create_key(indigo_property *property, indigo_item *item) {
	QString key(property->device);
	key.append(".");
	key.append(property->name);
	key.append(".");
	key.append(item->name);
	return key;
}

void blob_preview_cache::set_stretch_level(preview_stretch level) {
	preview_stretch_level = level;
}

bool blob_preview_cache::_remove(indigo_property *property, indigo_item *item) {
	QString key = create_key(property, item);
	if (contains(key)) {
		QImage *preview = value(key);
		indigo_debug("preview: %s(%s) == %p\n", __FUNCTION__, key.toUtf8().constData(), preview);
		if (preview != nullptr)
			delete(preview);
	} else {
		indigo_debug("preview: %s(%s) - no preview\n", __FUNCTION__, key.toUtf8().constData());
	}
	return (bool)QHash::remove(key);
}


bool blob_preview_cache::obsolete(indigo_property *property, indigo_item *item) {
	QString key = create_key(property, item);
	if (contains(key)) {
		QImage *preview = value(key);
		indigo_debug("preview: %s(%s) == %p\n", __FUNCTION__, key.toUtf8().constData(), preview);
		if (preview != nullptr) {
			QPainter painter(preview);
			painter.setPen(QColor(241, 183, 1));
			QFont ft = painter.font();
			ft.setPixelSize(preview->height()/15);
			painter.setFont(ft);
			painter.drawText(preview->width()/20, preview->height()/20, preview->width(), preview->height(), Qt::AlignTop & Qt::AlignLeft, "\u231b Busy...");
			return true;
		}
	} else {
		indigo_debug("preview: %s(%s) - no preview\n", __FUNCTION__, key.toUtf8().constData());
	}
	return false;
}


bool blob_preview_cache::create(indigo_property *property, indigo_item *item) {
	pthread_mutex_lock(&preview_mutex);
	QString key = create_key(property, item);
	_remove(property, item);
	const stretch_config_t sc = {preview_stretch_level, COLOR_BALANCE_NONE };
	QImage *preview = create_preview(property, item, sc);
	indigo_debug("preview: %s(%s) == %p\n", __FUNCTION__, key.toUtf8().constData(), preview);
	if (preview != nullptr) {
		insert(key, preview);
		pthread_mutex_unlock(&preview_mutex);
		return true;
	}
	pthread_mutex_unlock(&preview_mutex);
	return false;
}


QImage* blob_preview_cache::get(indigo_property *property, indigo_item *item) {
	pthread_mutex_lock(&preview_mutex);
	QString key = create_key(property, item);
	if (contains(key)) {
		QImage *preview = value(key);
		indigo_debug("preview: %s(%s) == %p\n", __FUNCTION__, key.toUtf8().constData(), preview);
		pthread_mutex_unlock(&preview_mutex);
		return preview;
	}
	indigo_debug("preview: %s(%s) - no preview\n", __FUNCTION__, key.toUtf8().constData());
	pthread_mutex_unlock(&preview_mutex);
	return nullptr;
}


bool blob_preview_cache::remove(indigo_property *property, indigo_item *item) {
	pthread_mutex_lock(&preview_mutex);
	bool success = _remove(property, item);
	pthread_mutex_unlock(&preview_mutex);
	return success;
}
