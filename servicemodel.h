#ifndef SERVICEMODEL_H
#define SERVICEMODEL_H

#include <QAbstractListModel>
#include <QList>

#include <qmdnsengine/browser.h>
#include <qmdnsengine/cache.h>
#include <qmdnsengine/service.h>
#include <qmdnsengine/server.h>
#include <qmdnsengine/resolver.h>


class IndigoService;


class ServiceModel : public QAbstractListModel
{
    Q_OBJECT

public:

    ServiceModel(const QByteArray &type);

    virtual int rowCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

private Q_SLOTS:

    void onServiceAdded(const QMdnsEngine::Service &service);
    void onServiceUpdated(const QMdnsEngine::Service &service);
    void onServiceRemoved(const QMdnsEngine::Service &service);

private:

    int findService(const QByteArray &name);

    QMdnsEngine::Server mServer;
    QMdnsEngine::Cache cache;
    QMdnsEngine::Browser mBrowser;
    QList<IndigoService*> mServices;
};

#endif // SERVICEMODEL_H
