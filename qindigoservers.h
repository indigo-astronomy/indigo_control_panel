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

class QIndigoServers : public QDialog
{
	Q_OBJECT
public:
	QIndigoServers(QWidget *parent = 0);
public slots:
	void highlightChecked(QListWidgetItem* item);
	void save();
private:
	QListWidget* widget;
	QDialogButtonBox* buttonBox;
	QGroupBox* viewBox;
	QPushButton* saveButton;
	QPushButton* closeButton;

	void createListWidget();
	void createOtherWidgets();
	void createLayout();
	void createConnections();
};

#endif // QINDIGO_SERVERS_H
