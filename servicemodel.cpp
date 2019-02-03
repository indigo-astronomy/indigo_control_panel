#include "indigoservice.h"
#include "servicemodel.h"
#include <indigo_client.h>

Q_DECLARE_METATYPE(QMdnsEngine::Service)

ServiceModel::ServiceModel(const QByteArray &type)
    : mServer(),
      mBrowser(&mServer, type, &cache)
{
    connect(&mBrowser, &QMdnsEngine::Browser::serviceAdded, this, &ServiceModel::onServiceAdded);
    connect(&mBrowser, &QMdnsEngine::Browser::serviceUpdated, this, &ServiceModel::onServiceUpdated);
    connect(&mBrowser, &QMdnsEngine::Browser::serviceRemoved, this, &ServiceModel::onServiceRemoved);
}

int
ServiceModel::rowCount(const QModelIndex &) const
{
    return mServices.count();
}

QVariant
ServiceModel::data(const QModelIndex &index, int role) const
{
    // Ensure the index points to a valid row
    if (!index.isValid() || index.row() < 0 || index.row() >= mServices.count()) {
        return QVariant();
    }

    IndigoService* service = mServices.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        return QString("%1:%2")
            .arg(QString(service->service.hostname()))
            .arg(service->service.port());
    case Qt::UserRole:
        return QVariant();
    }

    return QVariant();
}


void ServiceModel::onServiceAdded(const QMdnsEngine::Service &service)
{
    qDebug() << service.name() << "discovered on port" << service.port() << "!";
    beginInsertRows(QModelIndex(), mServices.count(), mServices.count());
    IndigoService* indigo_service = new IndigoService(service);
    mServices.append(indigo_service);
    endInsertRows();

    indigo_connect_server(service.name(), service.hostname(), service.port(), &indigo_service->server_entry);
}

void ServiceModel::onServiceUpdated(const QMdnsEngine::Service &service)
{
    qDebug() << "Service Updated " << service.name();
//    int i = findService(service.name());
//    if (i != -1) {
//        IndigoService s(service);
//        mServices.replace(i, s);
//        emit dataChanged(index(i), index(i));
//    }
}

void ServiceModel::onServiceRemoved(const QMdnsEngine::Service &service)
{
qDebug() << "Service Removed " << service.name();
    int i = findService(service.name());
    if (i != -1) {
        IndigoService* indigo_service = mServices.at(i);

        beginRemoveRows(QModelIndex(), i, i);
        mServices.removeAt(i);
        endRemoveRows();

        indigo_disconnect_server(indigo_service->server_entry);
        delete indigo_service;
    }
}

int ServiceModel::findService(const QByteArray &name)
{
    for (auto i = mServices.constBegin(); i != mServices.constEnd(); ++i) {
        if ((*i)->service.name() == name) {
            return i - mServices.constBegin();
        }
    }
    return -1;
}
