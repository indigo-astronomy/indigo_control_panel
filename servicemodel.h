// Copyright (c) 2019 Rumen G.Bogdanovski & David Hulse
// All rights reserved.
//
// You can use this software under the terms of 'INDIGO Astronomy
// open-source license' (see LICENSE.md).
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS 'AS IS' AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef SERVICEMODEL_H
#define SERVICEMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QTimer>

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
	void serviceConnectionChange(IndigoService &indigo_service);

private Q_SLOTS:
    void onServiceError(QZeroConf::error_t);
    void onServiceAdded(QZeroConfService s);
    void onServiceUpdated(QZeroConfService s);
    void onServiceRemoved(QZeroConfService s);
	void onTimer();
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
