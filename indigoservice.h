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
    virtual ~IndigoService();

    IndigoService &operator=(const IndigoService &other);

    bool operator==(const IndigoService &other) const;
    bool operator!=(const IndigoService &other) const;

    QByteArray name() const;

public:
    QZeroConfService service;
    indigo_server_entry* server_entry;
};

#endif // INDIGOSERVICE_H
