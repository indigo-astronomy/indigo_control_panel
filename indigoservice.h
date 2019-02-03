#ifndef INDIGOSERVICE_H
#define INDIGOSERVICE_H

#include <QObject>
#include <qmdnsengine/service.h>
#include <indigo_client.h>


class IndigoService
{
public:
    IndigoService(const QMdnsEngine::Service& _service);
    IndigoService(const IndigoService &other);
    virtual ~IndigoService();

    IndigoService &operator=(const IndigoService &other);

    bool operator==(const IndigoService &other) const;
    bool operator!=(const IndigoService &other) const;

    QByteArray name() const;

public:
    QMdnsEngine::Service service;
    indigo_server_entry* server_entry;
};

#endif // INDIGOSERVICE_H
