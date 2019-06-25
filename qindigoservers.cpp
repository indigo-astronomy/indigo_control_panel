#include "qindigoservers.h"

QIndigoServers::QIndigoServers(QWidget *parent): QDialog(parent)
{
	setWindowTitle("Checkable list in Qt");

	createListWidget();
	createOtherWidgets();
	createLayout();
	createConnections();
}

void QIndigoServers::createListWidget(){
	widget = new QListWidget;
	QStringList strList;
	strList << "monitor" << "mouse" << "keyboard" << "hard disk drive"
			<< "graphic card" << "sound card" << "memory" << "motherboard";

	widget->addItems(strList);

	QListWidgetItem* item = 0;
	for(int i = 0; i < widget->count(); ++i){
		item = widget->item(i);
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setCheckState(Qt::Unchecked);
	}
}

void QIndigoServers::createOtherWidgets(){
	viewBox = new QGroupBox(tr("Required components"));
	buttonBox = new QDialogButtonBox;
	saveButton = buttonBox->addButton(QDialogButtonBox::Save);
	closeButton = buttonBox->addButton(QDialogButtonBox::Close);
}

void QIndigoServers::createLayout(){
	QVBoxLayout* viewLayout = new QVBoxLayout;
	viewLayout->addWidget(widget);
	viewBox->setLayout(viewLayout);

	QHBoxLayout* horizontalLayout = new QHBoxLayout;
	horizontalLayout->addWidget(buttonBox);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(viewBox);
	mainLayout->addLayout(horizontalLayout);

	setLayout(mainLayout);
}

void QIndigoServers::createConnections(){
	QObject::connect(widget, SIGNAL(itemChanged(QListWidgetItem*)),
					 this, SLOT(highlightChecked(QListWidgetItem*)));
	QObject::connect(saveButton, SIGNAL(clicked()), this, SLOT(save()));
	QObject::connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
}

void QIndigoServers::highlightChecked(QListWidgetItem *item){
	if(item->checkState() == Qt::Checked)
		item->setBackgroundColor(QColor("#ffffb2"));
	else
		item->setBackgroundColor(QColor("#ffffff"));
}

void QIndigoServers::save(){

	QFile file("required_components.txt");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;

	QTextStream out(&file);
	out << "Required components:" << "\n";

	QListWidgetItem* item = 0;
	for(int i = 0; i < widget->count(); ++i){
		item = widget->item(i);
		if(item->checkState() == Qt::Checked)
			out << item->text() << "\n";
	}

	QMessageBox::information(this, tr("Checkable list in Qt"),
								   tr("Required components were saved."),
								   QMessageBox::Ok);
}
