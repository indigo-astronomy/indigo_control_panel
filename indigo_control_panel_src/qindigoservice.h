// Copyright (c) 2019 Rumen G.Bogdanovski
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


#ifndef INDIGOSERVICE_H
#define INDIGOSERVICE_H

#include <QObject>
#include <indigo/indigo_client.h>


class QIndigoService {

public:
	QIndigoService(QByteArray name, QByteArray host, int port);
	QIndigoService(QByteArray name, QByteArray host, int port, bool connect, bool is_manual_service = true);
	virtual ~QIndigoService();

	bool connect();
	bool connected() const;
	bool disconnect();
	QByteArray name() const { return m_name; }
	QByteArray host() const { return m_host; }
	int port() const { return m_port; }
	void setHost(QByteArray host) { m_host = host; }

	QByteArray m_name;
	QByteArray m_host;
	int m_port;
	indigo_server_entry* m_server_entry;

public:
	bool is_auto_service;
	bool auto_connect;
#ifdef INDIGO_VERSION_3
	indigo_uni_handle *prev_handle;
#else
	int prev_socket;
#endif

};

#endif // INDIGOSERVICE_H
