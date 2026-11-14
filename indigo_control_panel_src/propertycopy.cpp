// Copyright (c) 2026 Rumen G.Bogdanovski
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


#include <cstring>
#include <cstdlib>
#include "propertycopy.h"


indigo_property *copy_property(indigo_property *property) {
	indigo_property *p = nullptr;
	switch (property->type) {
	case INDIGO_TEXT_VECTOR:
		p = indigo_init_text_property(nullptr, property->device, property->name, property->group, property->label, property->state, property->perm, property->count);
		break;
	case INDIGO_NUMBER_VECTOR:
		p = indigo_init_number_property(nullptr, property->device, property->name, property->group, property->label, property->state, property->perm, property->count);
		break;
	case INDIGO_SWITCH_VECTOR:
		p = indigo_init_switch_property(nullptr, property->device, property->name, property->group, property->label, property->state, property->perm, property->rule, property->count);
		break;
	case INDIGO_LIGHT_VECTOR:
		p = indigo_init_light_property(nullptr, property->device, property->name, property->group, property->label, property->state, property->count);
		break;
	case INDIGO_BLOB_VECTOR:
		p = indigo_init_blob_property(nullptr, property->device, property->name, property->group, property->label, property->state, property->count);
		break;
	default:
		return nullptr;
	}
	if (p == nullptr) {
		return nullptr;
	}

	//  The header memcpy would overwrite allocated_count with the source's
	//  (e.g. INDIGO_PREALLOCATED_COUNT from the XML parser), making
	//  indigo_release_property() walk and free past the end of our copy.
	int allocated_count = p->allocated_count;
	memcpy(p, property, sizeof(indigo_property) + property->count * sizeof(indigo_item));
	p->allocated_count = allocated_count;

	if (property->type == INDIGO_TEXT_VECTOR) {
		for (int i = 0; i < property->count; i++) {
			if (property->items[i].text.long_value) {
				p->items[i].text.long_value = (char *)indigo_safe_malloc(property->items[i].text.length);
				memcpy(p->items[i].text.long_value, property->items[i].text.long_value, property->items[i].text.length);
			}
		}
	}
	return p;
}


void merge_property_items(indigo_property *dst, indigo_property *src) {
	bool is_text = src->type == INDIGO_TEXT_VECTOR && dst->type == INDIGO_TEXT_VECTOR;
	for (int i = 0; i < src->count; i++) {
		indigo_item *s = &src->items[i];
		indigo_item *d = nullptr;
		if (i < dst->count && strcmp(dst->items[i].name, s->name) == 0) {
			d = &dst->items[i];
		} else {
			for (int j = 0; j < dst->count; j++) {
				if (strcmp(dst->items[j].name, s->name) == 0) {
					d = &dst->items[j];
					break;
				}
			}
		}
		if (d == nullptr) {
			continue;  //  stays owned by src, freed when src is released
		}
		if (is_text) {
			//  Long text values live in a separately allocated buffer: free
			//  dst's old one and take ownership of src's.
			free(d->text.long_value);
			memcpy(d, s, sizeof(indigo_item));
			s->text.long_value = nullptr;
		} else {
			memcpy(d, s, sizeof(indigo_item));
		}
	}
}
