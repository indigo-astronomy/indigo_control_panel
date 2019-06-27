#ifndef QINDIGOSERVERS_H
#define QINDIGOSERVERS_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QLineEdit>
#include "indigoservice.h"

class QIndigoServers : public QDialog
{
	Q_OBJECT
public:
	QIndigoServers(QWidget *parent = 0);
	QString getServiceName(QListWidgetItem* item);

signals:
	void requestConnect(const QString &service);
	void requestDisconnect(const QString &service);
	void requestAddManualService(IndigoService &indigo_service);
	void requestRemoveManualService(const QString &service);

public slots:
	void onAddService(IndigoService &indigo_service);
	void onRemoveService(IndigoService &indigo_service);
	void highlightChecked(QListWidgetItem* item);
	void onConnectionChange(IndigoService &indigo_service);
	void onAddManualService();
	void onRemoveManualService();

private:
	QListWidget* m_server_list;
	QDialogButtonBox* m_button_box;
	QWidget* m_view_box;
	QWidget* m_add_service_box;
	QLineEdit* m_service_line;
	QPushButton* m_add_button;
	QPushButton* m_remove_button;
	QPushButton* m_close_button;
};

#endif // QINDIGO_SERVERS_H
