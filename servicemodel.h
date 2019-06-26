#ifndef SERVICEMODEL_H
#define SERVICEMODEL_H

#include <QAbstractListModel>
#include <QList>

#include <qzeroconf.h>
#include "logger.h"


class IndigoService;


class ServiceModel : public QAbstractListModel
{
    Q_OBJECT

public:

    ServiceModel(const QByteArray &type);

    virtual int rowCount(const QModelIndex &parent) const;
	bool addService(QByteArray name, QByteArray host,  int port);
	bool connectService(QByteArray name);
	bool disconnectService(QByteArray name);
	bool removeService(QByteArray name);
    virtual QVariant data(const QModelIndex &index, int role) const;

signals:
	void serviceAdded(IndigoService &indigo_service);
	void serviceRemoved(IndigoService &indigo_service);

private Q_SLOTS:
    void onServiceError(QZeroConf::error_t);
    void onServiceAdded(QZeroConfService s);
    void onServiceUpdated(QZeroConfService s);
    void onServiceRemoved(QZeroConfService s);
public Q_SLOTS:
	void onRequestConnect(const QString &service);
	void onRequestAddManualService(IndigoService &indigo_service);
	void onRequestRemoveManualService(const QString &service);
	void onRequestDisconnect(const QString &service);

private:
    int findService(const QByteArray &name);

	Logger* m_logger;
    QList<IndigoService*> mServices;

    QZeroConf m_zeroConf;
};

#endif // SERVICEMODEL_H
