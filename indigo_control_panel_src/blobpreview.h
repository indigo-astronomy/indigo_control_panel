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

#ifndef _BLOBPREVIEW_H
#define _BLOBPREVIEW_H

#include <QImage>
#include <QHash>
#include <indigo/indigo_client.h>
#include <imagepreview.h>

class blob_preview_cache: QHash<QString, QImage*> {
public:
	blob_preview_cache(): preview_mutex(PTHREAD_MUTEX_INITIALIZER) {
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&preview_mutex, &attr);
	};

	~blob_preview_cache() {
		indigo_debug("preview: %s()\n", __FUNCTION__);
		blob_preview_cache::iterator i = begin();
		while (i != end()) {
			QImage *preview = i.value();
			if (preview != nullptr) delete(preview);
			i = erase(i);
		}
	};

private:
	pthread_mutex_t preview_mutex;
	QString create_key(indigo_property *property, indigo_item *item);
	bool _remove(indigo_property *property, indigo_item *item);

public:
	void set_stretch_level(preview_stretch level);
	bool create(indigo_property *property, indigo_item *item);
	bool obsolete(indigo_property *property, indigo_item *item);
	QImage* get(indigo_property *property, indigo_item *item);
	bool remove(indigo_property *property, indigo_item *item);
};

extern blob_preview_cache preview_cache;

#endif /* _BLOBPREVIEW_H */
