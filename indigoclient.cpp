#include <indigo_client.h>
#include "indigoclient.h"


static indigo_result client_attach(indigo_client *client) {
	indigo_enumerate_properties(client, &INDIGO_ALL_PROPERTIES);
	return INDIGO_OK;
}


static indigo_result client_define_property(indigo_client *client, indigo_device *device, indigo_property *property, const char *message) {
	//  Deep copy the property so it won't disappear on us later
	indigo_property* p = nullptr;
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
		p = indigo_init_blob_property(nullptr, property->device, property->name, property->group, property->label, property->state,property->count);
		break;
	}
	memcpy(p, property, sizeof(indigo_property) + property->count * sizeof(indigo_item));

	emit(IndigoClient::instance().property_defined(p, message));
	return INDIGO_OK;
}


static indigo_result client_update_property(indigo_client *client, indigo_device *device, indigo_property *property, const char *message) {
	emit(IndigoClient::instance().property_changed(property, message));
	return INDIGO_OK;
}


static indigo_result client_delete_property(indigo_client *client, indigo_device *device, indigo_property *property, const char *message) {
	fprintf(stderr, "** Deleting property [%s] on device [%s]\n", property->name, property->device);

	indigo_property* p = new indigo_property;
	strcpy(p->device, property->device);
	strcpy(p->group, property->group);
	strcpy(p->name, property->name);
	emit(IndigoClient::instance().property_deleted(p, message));
}


static indigo_result client_send_message(indigo_client *client, indigo_device *device, const char *message) {
	//qDebug() << "BUS MSG: " << message;
}


static indigo_result client_detach(indigo_client *client) {
	//qDebug() << ("client detach\n");
	//exit(0);
	return INDIGO_OK;
}


static indigo_client client = {
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
