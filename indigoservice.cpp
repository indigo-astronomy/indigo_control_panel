#include "indigoservice.h"

IndigoService::IndigoService(const QZeroConfService& _service) :
	m_name(_service.name().toUtf8().constData()),
	m_host(_service.host().toUtf8().constData()),
	m_port(_service.port()),
	m_service(_service),
	m_server_entry(nullptr),
	isQZeroConfService(true),
	prevSocket(0) {
}


IndigoService::IndigoService(const IndigoService &other) : m_service(other.m_service) {
}


IndigoService::IndigoService(QByteArray name, QByteArray host, int port) :
	m_name(name),
	m_host(host),
	m_port(port),
	m_server_entry(nullptr),
	isQZeroConfService(false),
	prevSocket(0) {
}


IndigoService::~IndigoService() {
}


bool IndigoService::connect() {
	int i = 5; /* 0.5 seconds */
	prevSocket = -100;
	printf ("CALL: +++++ CONNECT %s %s %d\n", m_name.constData(), m_host.constData(), m_port);
	indigo_result res = indigo_connect_server(m_name.constData(), m_host.constData(), m_port, &m_server_entry);
	if (res != INDIGO_OK) return false;
	while (!connected() && i--) {
		indigo_usleep(100000);
	}
	return connected();
}


bool IndigoService::connected() const {
	if (m_server_entry) {
		return (m_server_entry->socket > 0);
	}
	printf ("Connected socket null\n");
	return false;
}


bool IndigoService::disconnect() {

	if (m_server_entry) {
		 printf ("CALL: ----- DISCONNECT %s %s %d\n", m_name.constData(), m_host.constData(), m_port);
		return (indigo_disconnect_server(m_server_entry) == INDIGO_OK);
	}
	return false;
}


IndigoService &IndigoService::operator=(const IndigoService &other) {
	m_service = other.m_service;
	return *this;
}


bool IndigoService::operator==(const IndigoService &other) const {
	return m_service == other.m_service;
}


bool IndigoService::operator!=(const IndigoService &other) const {
	return !(*this == other);
}
