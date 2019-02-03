#ifndef INDIGOCLIENT_H
#define INDIGOCLIENT_H

#include <QObject>
#include <indigo_bus.h>


class IndigoClient : public QObject
{
    Q_OBJECT
public:
    static IndigoClient& instance();

public:
    IndigoClient();

    void start();

signals:
    void property_defined(indigo_property* property);
    void property_changed(indigo_property* property);
    void property_deleted(indigo_property* property);
};

inline IndigoClient&
IndigoClient::instance()
{
   static IndigoClient* me = nullptr;
   if (!me)
       me = new IndigoClient();
   return *me;
}

#endif // INDIGOCLIENT_H
