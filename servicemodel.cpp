#include "indigoservice.h"
#include "servicemodel.h"
#include <indigo_client.h>
#include "conf.h"


ServiceModel::ServiceModel(const QByteArray &type) {
	m_logger = &Logger::instance();
	QTimer *m_timer = new QTimer(this);
	connect(m_timer, &QTimer::timeout, this, QOverload<>::of(&ServiceModel::onTimer));
	m_timer->start(3000);
	connect(&m_zeroConf, &QZeroConf::error, this, &ServiceModel::onServiceError);
	connect(&m_zeroConf, &QZeroConf::serviceAdded, this, &ServiceModel::onServiceAdded);
	connect(&m_zeroConf, &QZeroConf::serviceRemoved, this, &ServiceModel::onServiceRemoved);
	m_zeroConf.startBrowser(type);
}


void ServiceModel::onTimer() {
	for (auto i = mServices.constBegin(); i != mServices.constEnd(); ++i) {
		if (i == nullptr) continue;
		if ((*i)->m_server_entry == nullptr) continue;

		int socket = (*i)->m_server_entry->socket;
		if (socket != (*i)->prevSocket) {
			indigo_debug("SERVICE Sockets '%s' '%s' [%d] %d\n",(*i)->m_server_entry->name, (*i)->m_server_entry->host, socket, (*i)->prevSocket);
			(*i)->prevSocket = socket;
			emit(serviceConnectionChange(**i));
		}
	}
}


int ServiceModel::rowCount(const QModelIndex &) const {
	return mServices.count();
}


bool ServiceModel::addService(QByteArray name, QByteArray host, int port) {
	int i = findService(name);
	if (i != -1) {
		indigo_debug("SERVICE DUPLICATE [%s]\n", name.constData());
		return false;
	}

	indigo_debug("SERVICE ADDED Manually [%s]\n", name.constData());

	beginInsertRows(QModelIndex(), mServices.count(), mServices.count());
	IndigoService* indigo_service = new IndigoService(name, host, port);
	mServices.append(indigo_service);
	endInsertRows();

	if (conf.auto_connect) indigo_service->connect();
	emit(serviceAdded(*indigo_service));
	return true;
}


bool ServiceModel::removeService(QByteArray name) {
	int i = findService(name);
	if (i != -1) {
		IndigoService* indigo_service = mServices.at(i);
		if (indigo_service->isQZeroConfService) return false;
		beginRemoveRows(QModelIndex(), i, i);
		mServices.removeAt(i);
		endRemoveRows();
		indigo_service->disconnect();
		emit(serviceRemoved(*indigo_service));
		delete indigo_service;
		indigo_debug("SERVICE REMOVED [%s]\n", name.constData());
		return true;
	}
	indigo_debug("SERVICE DOES NOT EXIST [%s]\n", name.constData());
	return false;
}


bool ServiceModel::connectService(QByteArray name) {
	int i = findService(name);
	if (i == -1) {
		indigo_debug("SERVICE NOT FOUND [%s]\n", name.constData());
		return false;
	}
	IndigoService* indigo_service = mServices.at(i);
	indigo_debug("CONNECTING TO SERVICE [%s] on %s:%d\n", name.constData(), indigo_service->host().constData(), indigo_service->port());
	return indigo_service->connect();
}


bool ServiceModel::disconnectService(QByteArray name) {
	int i = findService(name);
	if (i == -1) {
		indigo_debug("SERVICE NOT FOUND [%s]\n", name.constData());
		return false;
	}
	IndigoService* indigo_service = mServices.at(i);
	indigo_debug("CONNECTING TO SERVICE [%s] on %s:%d\n", name.constData(), indigo_service->host().constData(), indigo_service->port());

	return indigo_service->disconnect();
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
			.arg(QString(service->host()))
			.arg(service->port());
	case Qt::UserRole:
		return QVariant();
	}

	return QVariant();
}


void ServiceModel::onServiceError(QZeroConf::error_t e) {
	indigo_error("ZEROCONF ERROR %d", e);
}


void ServiceModel::onServiceAdded(QZeroConfService service) {
	int i = findService(service.name().toUtf8());
	if (i != -1) {
		indigo_debug("SERVICE DUPLICATE [%s]\n", service.name().toUtf8().constData());
		return;
	}

	indigo_debug("SERVICE ADDED [%s] on %s:%d\n", service.name().constData(), service.host().constData(), service.port());

	beginInsertRows(QModelIndex(), mServices.count(), mServices.count());
	IndigoService* indigo_service = new IndigoService(service);
	mServices.append(indigo_service);
	endInsertRows();

	if (conf.auto_connect) indigo_service->connect();
	emit(serviceAdded(*indigo_service));
}


void ServiceModel::onServiceUpdated(QZeroConfService service) {
	indigo_debug("SERVICE ADDED [%s] on %s:%d\n", service.name().constData(), service.host().constData(), service.port());
//	int i = findService(service.name());
//	if (i != -1) {
//		IndigoService s(service);
//		mServices.replace(i, s);
//		emit dataChanged(index(i), index(i));
//	}
}


void ServiceModel::onServiceRemoved(QZeroConfService service) {
	indigo_debug("SERVICE REMOVED [%s]\n", service.name().toUtf8().constData());

	//qDebug() << "Service Removed " << service.name();
	int i = findService(service.name().toUtf8());
	if (i != -1) {
		IndigoService* indigo_service = mServices.at(i);
		indigo_debug("SERVICE REMOVED WWWWW [%s]\n", service.name().toUtf8().constData());
		beginRemoveRows(QModelIndex(), i, i);
		mServices.removeAt(i);
		endRemoveRows();
		indigo_service->disconnect();
		emit(serviceRemoved(*indigo_service));
		delete indigo_service;
		indigo_service = nullptr;
	}
}


void ServiceModel::onRequestConnect(const QString &service) {
	connectService(service.toUtf8());
}


void ServiceModel::onRequestDisconnect(const QString &service) {
	disconnectService(service.toUtf8());
}

void ServiceModel::onRequestAddManualService(IndigoService &indigo_service) {
	addService(indigo_service.name(), indigo_service.host(), indigo_service.port());
}


void ServiceModel::onRequestRemoveManualService(const QString &service) {
	removeService(service.toUtf8());
}


int ServiceModel::findService(const QByteArray &name) {
	for (auto i = mServices.constBegin(); i != mServices.constEnd(); ++i) {
		if ((*i)->name() == name) {
			return i - mServices.constBegin();
		}
	}
	return -1;
}
