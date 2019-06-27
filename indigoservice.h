#ifndef INDIGOSERVICE_H
#define INDIGOSERVICE_H

#include <QObject>
#include <qzeroconf.h>
#include <indigo_client.h>


class IndigoService
{
public:
    IndigoService(const QZeroConfService& _service);
    IndigoService(const IndigoService &other);
	IndigoService(QByteArray name, QByteArray host, int port);

    virtual ~IndigoService();

    IndigoService &operator=(const IndigoService &other);

    bool operator==(const IndigoService &other) const;
    bool operator!=(const IndigoService &other) const;

	bool connect();
	bool connected() const;
	bool disconnect();
	QByteArray name() const { return m_name; }
	QByteArray host() const { return m_host; }
	int port() const { return m_port; }

	QByteArray m_name;
	QByteArray m_host;
	int m_port;
	QZeroConfService m_service;
	indigo_server_entry* m_server_entry;

public:
	bool isQZeroConfService;
	int prevSocket;
};

#endif // INDIGOSERVICE_H
