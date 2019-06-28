#include <indigo_client.h>
#include "indigoclient.h"
#include "conf.h"


static indigo_result client_attach(indigo_client *client) {
	indigo_enumerate_properties(client, &INDIGO_ALL_PROPERTIES);
	return INDIGO_OK;
}


static indigo_result client_define_property(indigo_client *client, indigo_device *device, indigo_property *property, const char *message) {
	Q_UNUSED(device);
	//  Deep copy the property so it won't disappear on us later
	static indigo_property* p = nullptr;
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
		if (conf.blobs_enabled) {
			if (device->version >= INDIGO_VERSION_2_0)
				indigo_enable_blob(client, property, INDIGO_ENABLE_BLOB_URL);
			else
				indigo_enable_blob(client, property, INDIGO_ENABLE_BLOB_ALSO);
		}
		if (property->state == INDIGO_OK_STATE) {
			for (int row = 0; row < property->count; row++) {
				if (*property->items[row].blob.url && indigo_populate_http_blob_item(&property->items[row])) {
				}
			}
		}
		p = indigo_init_blob_property(nullptr, property->device, property->name, property->group, property->label, property->state,property->count);
		break;
	}
	memcpy(p, property, sizeof(indigo_property) + property->count * sizeof(indigo_item));

	if (message) {
		static char msg[INDIGO_VALUE_SIZE];
		strncpy(msg, message, INDIGO_VALUE_SIZE);
		emit(IndigoClient::instance().property_defined(p, msg));
	} else {
		emit(IndigoClient::instance().property_defined(p, NULL));
	}
	return INDIGO_OK;
}


static indigo_result client_update_property(indigo_client *client, indigo_device *device, indigo_property *property, const char *message) {
	Q_UNUSED(client);
	Q_UNUSED(device);
	static indigo_property* p = nullptr;
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
		if (property->state == INDIGO_OK_STATE) {
			for (int row = 0; row < property->count; row++) {
				if (*property->items[row].blob.url && indigo_populate_http_blob_item(&property->items[row])) {
					indigo_log("Image URL received (%s, %ld bytes)...\n", property->items[0].blob.url, property->items[0].blob.size);
				}
			}
		}
		p = indigo_init_blob_property(nullptr, property->device, property->name, property->group, property->label, property->state,property->count);
		break;
	}

	memcpy(p, property, sizeof(indigo_property) + property->count * sizeof(indigo_item));
	if (message) {
		static char msg[INDIGO_VALUE_SIZE];
		strncpy(msg, message, INDIGO_VALUE_SIZE);
		emit(IndigoClient::instance().property_changed(p, msg));
	} else {
		emit(IndigoClient::instance().property_changed(p, NULL));
	}
	return INDIGO_OK;
}


static indigo_result client_delete_property(indigo_client *client, indigo_device *device, indigo_property *property, const char *message) {
	Q_UNUSED(client);
	Q_UNUSED(device);
	indigo_debug("Deleting property [%s] on device [%s]\n", property->name, property->device);

	indigo_property* p = new indigo_property;
	strcpy(p->device, property->device);
	strcpy(p->group, property->group);
	strcpy(p->name, property->name);

	if (message) {
		static char msg[INDIGO_VALUE_SIZE];
		strncpy(msg, message, INDIGO_VALUE_SIZE);
		emit(IndigoClient::instance().property_deleted(p, msg));
	} else {
		emit(IndigoClient::instance().property_deleted(p, NULL));
	}
	return INDIGO_OK;
}


static indigo_result client_send_message(indigo_client *client, indigo_device *device, const char *message) {
	Q_UNUSED(client);
	Q_UNUSED(device);
	Q_UNUSED(message);

	return INDIGO_OK;
}


static indigo_result client_detach(indigo_client *client) {
	Q_UNUSED(client);

	return INDIGO_OK;
}


indigo_client client = {
	"Indigo Control Panel", false, NULL, INDIGO_OK, INDIGO_VERSION_CURRENT, NULL,
	client_attach,
	client_define_property,
	client_update_property,
	client_delete_property,
	client_send_message,
	client_detach
};


IndigoClient::IndigoClient() {
	indigo_set_log_level(INDIGO_LOG_DEBUG);
}

void IndigoClient::start() {
	indigo_start();
	indigo_attach_client(&client);
}
