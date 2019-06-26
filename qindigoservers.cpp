#include "qindigoservers.h"

QIndigoServers::QIndigoServers(QWidget *parent): QDialog(parent)
{
	setWindowTitle("Available Servers");

	m_server_list = new QListWidget;
	m_view_box = new QGroupBox(tr("Servers"));
	m_button_box = new QDialogButtonBox;
	m_add_button = m_button_box->addButton(tr("Add service"), QDialogButtonBox::ActionRole);
	m_remove_button = m_button_box->addButton(tr("Remove service"), QDialogButtonBox::ActionRole);
	m_close_button = m_button_box->addButton(tr("Close"), QDialogButtonBox::ActionRole);

	QVBoxLayout* viewLayout = new QVBoxLayout;
	viewLayout->addWidget(m_server_list);
	m_view_box->setLayout(viewLayout);

	QHBoxLayout* horizontalLayout = new QHBoxLayout;
	horizontalLayout->addWidget(m_button_box);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(m_view_box);
	mainLayout->addLayout(horizontalLayout);

	setLayout(mainLayout);

	QObject::connect(m_server_list, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(highlightChecked(QListWidgetItem*)));
	QObject::connect(m_add_button, SIGNAL(clicked()), this, SLOT(save()));
	QObject::connect(m_close_button, SIGNAL(clicked()), this, SLOT(close()));
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

	if (indigo_service.isQZeroConfService)
		item->setForeground(QBrush(QColor("#ffff00")));
	else
		item->setForeground(QBrush(QColor("#00ff00")));
	m_server_list->addItem(item);
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


QString QIndigoServers::getServiceName(QListWidgetItem* item) {
	QString service = item->text();
	int pos = service.indexOf('@');
	if (pos > 0) service.truncate(pos);
	service = service.trimmed();
	return service;
}

void QIndigoServers::save(){

	QFile file("required_components.txt");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;

	QTextStream out(&file);
	out << "Required components:" << "\n";

	QListWidgetItem* item = 0;
	for(int i = 0; i < m_server_list->count(); ++i){
		item = m_server_list->item(i);
		if(item->checkState() == Qt::Checked)
			out << item->text() << "\n";
	}

	QMessageBox::information(this, tr("Checkable list in Qt"),
								   tr("Required components were saved."),
								   QMessageBox::Ok);
}
