#include "indigoservice.h"

IndigoService::IndigoService(const QZeroConfService& _service) : service(_service), server_entry(nullptr) {
}


IndigoService::IndigoService(const IndigoService &other) : service(other.service) {
}


IndigoService::~IndigoService() {
}


IndigoService &IndigoService::operator=(const IndigoService &other) {
	service = other.service;
	return *this;
}


bool IndigoService::operator==(const IndigoService &other) const {
	return service == other.service;
}


bool IndigoService::operator!=(const IndigoService &other) const {
	return !(*this == other);
}


QByteArray IndigoService::name() const {
	return service.name().toUtf8();
}
