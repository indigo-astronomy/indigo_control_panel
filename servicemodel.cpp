#include "indigoservice.h"
#include "servicemodel.h"
#include <indigo_client.h>


ServiceModel::ServiceModel(const QByteArray &type) {
	connect(&zeroConf, &QZeroConf::error, this, &ServiceModel::onServiceError);
	connect(&zeroConf, &QZeroConf::serviceAdded, this, &ServiceModel::onServiceAdded);
	connect(&zeroConf, &QZeroConf::serviceRemoved, this, &ServiceModel::onServiceRemoved);
	zeroConf.startBrowser(type);
}


int ServiceModel::rowCount(const QModelIndex &) const {
	return mServices.count();
}


QVariant ServiceModel::data(const QModelIndex &index, int role) const {
	// Ensure the index points to a valid row
	if (!index.isValid() || index.row() < 0 || index.row() >= mServices.count()) {
		return QVariant();
	}

	IndigoService* service = mServices.at(index.row());

	switch (role) {
	case Qt::DisplayRole:
		return QString("%1:%2")
			.arg(QString(service->service.host()))
			.arg(service->service.port());
	case Qt::UserRole:
		return QVariant();
	}

	return QVariant();
}


void ServiceModel::onServiceError(QZeroConf::error_t e) {
	fprintf(stderr, "ZEROCONF ERROR %d", e);
}


void ServiceModel::onServiceAdded(QZeroConfService service) {
	int i = findService(service.name().toUtf8());
	if (i != -1) {
		fprintf(stderr, "SERVICE DUPLICATE [%s]\n", service.name().toUtf8().constData());
		return;
	}

	fprintf(stderr, "SERVICE ADDED [%s]\n", service.name().toUtf8().constData());

	qDebug() << service.name() << "discovered on port" << service.port() << "!";
	beginInsertRows(QModelIndex(), mServices.count(), mServices.count());
	IndigoService* indigo_service = new IndigoService(service);
	mServices.append(indigo_service);
	endInsertRows();

	indigo_connect_server(service.name().toUtf8().constData(), service.host().toUtf8().constData(), service.port(), &indigo_service->server_entry);
}


void ServiceModel::onServiceUpdated(QZeroConfService service) {
	qDebug() << "Service Updated " << service.name();
//	int i = findService(service.name());
//	if (i != -1) {
//		IndigoService s(service);
//		mServices.replace(i, s);
//		emit dataChanged(index(i), index(i));
//	}
}


void ServiceModel::onServiceRemoved(QZeroConfService service) {
	fprintf(stderr, "SERVICE REMOVED [%s]\n", service.name().toUtf8().constData());

	//qDebug() << "Service Removed " << service.name();
	int i = findService(service.name().toUtf8());
	if (i != -1) {
		IndigoService* indigo_service = mServices.at(i);

		beginRemoveRows(QModelIndex(), i, i);
		mServices.removeAt(i);
		endRemoveRows();

		indigo_disconnect_server(indigo_service->server_entry);
		delete indigo_service;
		indigo_service = nullptr;
	}
}


int ServiceModel::findService(const QByteArray &name) {
	for (auto i = mServices.constBegin(); i != mServices.constEnd(); ++i) {
		if ((*i)->service.name() == name) {
			return i - mServices.constBegin();
		}
	}
	return -1;
}
