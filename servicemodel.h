#ifndef SERVICEMODEL_H
#define SERVICEMODEL_H

#include <QAbstractListModel>
#include <QList>

#include <qzeroconf.h>


class IndigoService;


class ServiceModel : public QAbstractListModel
{
    Q_OBJECT

public:

    ServiceModel(const QByteArray &type);

    virtual int rowCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

private Q_SLOTS:
    void onServiceError(QZeroConf::error_t);
    void onServiceAdded(QZeroConfService s);
    void onServiceUpdated(QZeroConfService s);
    void onServiceRemoved(QZeroConfService s);

private:
    int findService(const QByteArray &name);

    QList<IndigoService*> mServices;

    QZeroConf zeroConf;
};

#endif // SERVICEMODEL_H
