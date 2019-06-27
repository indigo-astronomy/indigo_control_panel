#include "qindigoservers.h"

QIndigoServers::QIndigoServers(QWidget *parent): QDialog(parent)
{
	setWindowTitle("Available Services");

	m_server_list = new QListWidget;
	m_view_box = new QWidget();
	m_button_box = new QDialogButtonBox;
	m_service_line = new QLineEdit;
	m_service_line->setMinimumWidth(300);
	//m_add_button = m_button_box->addButton(tr("Add service"), QDialogButtonBox::ActionRole);
	m_add_button = new QPushButton(" &Add ");
	m_add_button->setDefault(true);
	m_remove_button = m_button_box->addButton(tr("Remove selected"), QDialogButtonBox::ActionRole);
	m_close_button = m_button_box->addButton(tr("Close"), QDialogButtonBox::ActionRole);

	QVBoxLayout* viewLayout = new QVBoxLayout;
	viewLayout->setContentsMargins(0, 0, 0, 0);
	viewLayout->addWidget(m_server_list);

	m_add_service_box = new QWidget();
	QHBoxLayout* addLayout = new QHBoxLayout;
	addLayout->setContentsMargins(0, 0, 0, 0);
	addLayout->addWidget(m_service_line);
	addLayout->addWidget(m_add_button);
	m_add_service_box->setLayout(addLayout);

	viewLayout->addWidget(m_add_service_box);
	m_view_box->setLayout(viewLayout);

	QHBoxLayout* horizontalLayout = new QHBoxLayout;
	horizontalLayout->addWidget(m_button_box);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(m_view_box);
	mainLayout->addLayout(horizontalLayout);

	setLayout(mainLayout);

	QObject::connect(m_server_list, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(highlightChecked(QListWidgetItem*)));
	QObject::connect(m_add_button, SIGNAL(clicked()), this, SLOT(onAddManualService()));
	QObject::connect(m_remove_button, SIGNAL(clicked()), this, SLOT(onRemoveManualService()));
	QObject::connect(m_close_button, SIGNAL(clicked()), this, SLOT(close()));
}


void QIndigoServers::onConnectionChange(IndigoService &indigo_service) {
	QString service_name = indigo_service.name();
	printf ("Connection State Change [%s] connected = %d\n", service_name.toUtf8().constData(), indigo_service.connected());
	QListWidgetItem* item = 0;
	for(int i = 0; i < m_server_list->count(); ++i){
		item = m_server_list->item(i);
		QString service = getServiceName(item);
		if (service == service_name) {
			if (indigo_service.connected())
				item->setCheckState(Qt::Checked);
			else
				item->setCheckState(Qt::Unchecked);
			break;
		}
	}
}


void QIndigoServers::onAddService(IndigoService &indigo_service) {
	QListWidgetItem* item = new QListWidgetItem(
		indigo_service.name() +
		tr(" @ ") +
		indigo_service.host() +
		tr(":") +
		QString::number(indigo_service.port())
	);

	item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
	if (indigo_service.connected())
		item->setCheckState(Qt::Checked);
	else
		item->setCheckState(Qt::Unchecked);

	if (indigo_service.isQZeroConfService) {
		item->setForeground(QBrush(QColor("#99FF00")));
		item->setData(Qt::DecorationRole,QIcon("resource/bonjour_service.png"));
	} else {
		item->setForeground(QBrush(QColor("#FFFFFF")));
		item->setData(Qt::DecorationRole,QIcon("resource/manual_service.png"));
	}
	m_server_list->addItem(item);
}


void QIndigoServers::onAddManualService() {
	QString service_str = m_service_line->text();
	QStringList parts = service_str.split('@', QString::SkipEmptyParts);
	if (parts.size() != 2) {
		printf ("FORMAT ERROR\n");
		return;
	}
	QStringList parts2 = parts.at(1).split(':', QString::SkipEmptyParts);
	if (parts2.size() != 2) {
		printf ("FORMAT 2 ERROR\n");
		return;
	}
	int port = atoi(parts2.at(1).toUtf8().constData());
	IndigoService indigo_service(parts.at(0).toUtf8(), parts2.at(0).toUtf8(), port);
	requestAddManualService(indigo_service);
	m_service_line->setText("");
	printf ("Service '%s' host '%s' port = %d\n", parts.at(0).toUtf8().constData(), parts2.at(0).toUtf8().constData(), atoi(parts2.at(1).toUtf8().constData()));
}


void QIndigoServers::onRemoveService(IndigoService &indigo_service) {
	QString service_name = indigo_service.name();
	QListWidgetItem* item = 0;
	for(int i = 0; i < m_server_list->count(); ++i){
		item = m_server_list->item(i);
		QString service = getServiceName(item);
		if (service == service_name) {
			delete item;
			break;
		}
	}
}


void QIndigoServers::highlightChecked(QListWidgetItem *item){
	QString service = getServiceName(item);
	if(item->checkState() == Qt::Checked)
		emit(requestConnect(service));
	else
		emit(requestDisconnect(service));
}


void QIndigoServers::onRemoveManualService() {
	printf ("TO BE REMOVED: [....]\n");
	QModelIndex index = m_server_list->currentIndex();
	QString service = index.data(Qt::DisplayRole).toString();
	int pos = service.indexOf('@');
	if (pos > 0) service.truncate(pos);
	service = service.trimmed();
	printf ("TO BE REMOVED: [%s]\n", service.toUtf8().constData());
	emit(requestRemoveManualService(service));
}


QString QIndigoServers::getServiceName(QListWidgetItem* item) {
	QString service = item->text();
	int pos = service.indexOf('@');
	if (pos > 0) service.truncate(pos);
	service = service.trimmed();
	return service;
}
