#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QTreeView>
#include <QMenuBar>
#include <QIcon>
#include <QPlainTextEdit>
#include <QScrollArea>
#include <QMessageBox>
#include <QActionGroup>
#include <sys/time.h>
#include "browserwindow.h"
#include "servicemodel.h"
#include "propertymodel.h"
#include "indigoclient.h"
#include "qindigoproperty.h"
#include "qindigoservers.h"
#include "logger.h"
#include "conf.h"

void write_conf();

BrowserWindow::BrowserWindow(QWidget *parent) : QMainWindow(parent) {
	setWindowTitle(tr("INDIGO Control Panel"));
	resize(1024, 768);

	QIcon icon(":resource/appicon.png");
	this->setWindowIcon(icon);

	mIndigoServers = new QIndigoServers(this);

	//  Set central widget of window
	QWidget *central = new QWidget;
	setCentralWidget(central);

	//  Set the root layout to be a VBox
	QVBoxLayout *rootLayout = new QVBoxLayout;
	rootLayout->setSpacing(0);
	rootLayout->setContentsMargins(0, 0, 0, 0);
	rootLayout->setSizeConstraint(QLayout::SetMinimumSize);
	central->setLayout(rootLayout);

	// Create menubar
	QMenuBar *menu = new QMenuBar;
	QMenu *file = new QMenu("&File");
	QAction *act;

	act = file->addAction(tr("&Manage Services"));
	connect(act, &QAction::triggered, this, &BrowserWindow::on_servers_act);

	act = file->addAction(tr("&Exit"));
	connect(act, &QAction::triggered, this, &BrowserWindow::on_exit_act);

	menu->addMenu(file);

	QMenu *settings = new QMenu("&Settings");

	act = settings->addAction(tr("Ebable &BLOBs"));
	act->setCheckable(true);
	act->setChecked(conf.blobs_enabled);
	connect(act, &QAction::toggled, this, &BrowserWindow::on_blobs_changed);

	act = settings->addAction(tr("Enable auto &connect"));
	act->setCheckable(true);
	act->setChecked(conf.auto_connect);
	connect(act, &QAction::toggled, this, &BrowserWindow::on_bonjour_changed);

	act = settings->addAction(tr("&Use host suffix"));
	act->setCheckable(true);
	act->setChecked(conf.indigo_use_host_suffix);
	connect(act, &QAction::toggled, this, &BrowserWindow::on_use_suffix_changed);

	settings->addSeparator();
	QActionGroup *log_group = new QActionGroup(this);
	log_group->setExclusive(true);

	act = settings->addAction("Log Error");
	act->setCheckable(true);
	if (conf.indigo_log_level == INDIGO_LOG_ERROR) act->setChecked(true);
	connect(act, &QAction::triggered, this, &BrowserWindow::on_log_error);
	log_group->addAction(act);

	act = settings->addAction("Log Info");
	act->setCheckable(true);
	if (conf.indigo_log_level == INDIGO_LOG_INFO) act->setChecked(true);
	connect(act, &QAction::triggered, this, &BrowserWindow::on_log_info);
	log_group->addAction(act);

	act = settings->addAction("Log Debug");
	act->setCheckable(true);
	if (conf.indigo_log_level == INDIGO_LOG_DEBUG) act->setChecked(true);
	connect(act, &QAction::triggered, this, &BrowserWindow::on_log_debug);
	log_group->addAction(act);

	act = settings->addAction("Log Trace");
	act->setCheckable(true);
	if (conf.indigo_log_level == INDIGO_LOG_TRACE) act->setChecked(true);
	connect(act, &QAction::triggered, this, &BrowserWindow::on_log_trace);
	log_group->addAction(act);

	menu->addMenu(settings);
	QMenu *help = new QMenu("&Help");

	act = help->addAction(tr("&About"));
	connect(act, &QAction::triggered, this, &BrowserWindow::on_about_act);
	menu->addMenu(help);

	rootLayout->addWidget(menu);

	// Create properties viewing area
	QWidget *view = new QWidget;
	QVBoxLayout *propertyLayout = new QVBoxLayout;
	propertyLayout->setSpacing(5);
	propertyLayout->setContentsMargins(5, 5, 5, 5);
	propertyLayout->setSizeConstraint(QLayout::SetMinimumSize);
	view->setLayout(propertyLayout);
	rootLayout->addWidget(view);

	mProperties = new QTreeView;
	form_panel = new QWidget();
	form_layout = new QVBoxLayout();
	form_layout->setMargin(0);
	form_panel->setLayout(form_layout);

	mScrollArea = new QScrollArea();
	mScrollArea->setWidgetResizable(true);
	mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	mScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	form_layout->addWidget(mScrollArea);
	mScrollArea->setMinimumWidth(600);

	QSplitter* hSplitter = new QSplitter;
	hSplitter->addWidget(mProperties);
	hSplitter->addWidget(form_panel);
	hSplitter->setStretchFactor(0, 45);
	hSplitter->setStretchFactor(2, 55);
	propertyLayout->addWidget(hSplitter, 85);

	//  Create log viewer
	mLog = new QPlainTextEdit;
	mLog->setReadOnly(true);
	propertyLayout->addWidget(mLog, 15);

	mServiceModel = new ServiceModel("_indigo._tcp");

	mPropertyModel = new PropertyModel();
	mProperties->setHeaderHidden(true);
	mProperties->setModel(mPropertyModel);

	connect(mServiceModel, &ServiceModel::serviceAdded, mIndigoServers, &QIndigoServers::onAddService);
	connect(mServiceModel, &ServiceModel::serviceRemoved, mIndigoServers, &QIndigoServers::onRemoveService);
	connect(mServiceModel, &ServiceModel::serviceConnectionChange, mIndigoServers, &QIndigoServers::onConnectionChange);

	connect(mIndigoServers, &QIndigoServers::requestConnect, mServiceModel, &ServiceModel::onRequestConnect);
	connect(mIndigoServers, &QIndigoServers::requestDisconnect, mServiceModel, &ServiceModel::onRequestDisconnect);
	connect(mIndigoServers, &QIndigoServers::requestAddManualService, mServiceModel, &ServiceModel::onRequestAddManualService);
	connect(mIndigoServers, &QIndigoServers::requestRemoveManualService, mServiceModel, &ServiceModel::onRequestRemoveManualService);

	connect(&IndigoClient::instance(), &IndigoClient::property_defined, mPropertyModel, &PropertyModel::define_property);
	connect(&IndigoClient::instance(), &IndigoClient::property_changed, mPropertyModel, &PropertyModel::update_property);
	connect(&IndigoClient::instance(), &IndigoClient::property_deleted, mPropertyModel, &PropertyModel::delete_property);

	connect(&IndigoClient::instance(), &IndigoClient::property_defined, this, &BrowserWindow::on_property_log);
	connect(&IndigoClient::instance(), &IndigoClient::property_changed, this, &BrowserWindow::on_property_log);
	connect(&IndigoClient::instance(), &IndigoClient::property_deleted, this, &BrowserWindow::on_property_log);

	connect(mPropertyModel, &PropertyModel::property_defined, this, &BrowserWindow::on_property_define_delete);
	connect(mPropertyModel, &PropertyModel::property_deleted, this, &BrowserWindow::on_property_define_delete);

	connect(&Logger::instance(), &Logger::log_in_window, this, &BrowserWindow::on_property_log);

	connect(mProperties->selectionModel(), &QItemSelectionModel::selectionChanged, this, &BrowserWindow::on_selection_changed);
	connect(this, &BrowserWindow::enable_blobs, mPropertyModel, &PropertyModel::enable_blobs);

	current_node = nullptr;

	//  Start up the client
	IndigoClient::instance().start();
}


void BrowserWindow::on_property_log(indigo_property* property, const char *message) {
	char timestamp[16];
	char log_line[512];
	struct timeval tmnow;

	if (!message) return;

	//indigo_debug("CCCCC-> %s\n", message);

	gettimeofday(&tmnow, NULL);
	strftime(timestamp, sizeof(log_line), "%H:%M:%S", localtime((const time_t *) &tmnow.tv_sec));
	snprintf(timestamp + 8, sizeof(timestamp) - 8, ".%03ld", tmnow.tv_usec/1000);
	if (property)
		snprintf(log_line, 512, "%s '%s'.%s - %s", timestamp, property->device, property->name, message);
	else
		snprintf(log_line, 512, "%s %s", timestamp, message);

	mLog->appendPlainText(log_line); // Adds the message to the widget
	indigo_debug("Log window: %s\n", message);
}

void BrowserWindow::on_property_define_delete(indigo_property* property, const char *message) {
	Q_UNUSED(message);
	QItemSelection selected = mProperties->selectionModel()->selection();
	if (!selected.isEmpty()) {
		QModelIndex s = selected.indexes().front();
		TreeNode* node = static_cast<TreeNode*>(s.internalPointer());
		if (node->node_type == TREE_NODE_PROPERTY) {
			/* We can not have selected property to be defined -> so just clean window */
			PropertyNode* p = reinterpret_cast<PropertyNode*>(node);
			if (!strcmp(p->property->name, property->name) &&
			    !strcmp(p->property->device, property->device) &&
			    !strcmp(p->property->group, property->group)) {
				//indigo_debug("SELECTED PROPERTY deleted\n");
				clear_window();
				return;
			}
		} else if (node->node_type == TREE_NODE_GROUP) {
			/* If we have deleted or defined property update group window */
			GroupNode* g = reinterpret_cast<GroupNode*>(node);
			if (g->size() > 0) {
				PropertyNode* p = g->children.nodes[0];
				if (!strcmp(p->property->device, property->device) &&
				    !strcmp(p->property->group, property->group)) {
						indigo_debug("PROPERTY IN GROUP defined/deleted\n");
						clear_window();
						on_selection_changed(selected, selected);
						return;
				}
			}
		}
		if (!strcmp(property->name,"")) {
			if (node->node_type == TREE_NODE_PROPERTY) {
				PropertyNode* p = reinterpret_cast<PropertyNode*>(node);
				if (!strcmp(p->property->device, property->device)) {
					indigo_debug("SELECTED DEVICE deleted (Peoperty selected)\n");
					clear_window();
					return;
				}
			} else if (node->node_type == TREE_NODE_GROUP) {
				GroupNode* g = reinterpret_cast<GroupNode*>(node);
				PropertyNode* p = g->children.nodes[0];
				if (!strcmp(p->property->device, property->device)) {
					indigo_debug("SELECTED DEVICE deleted (Group selected)\n");
					clear_window();
					return;
				}
			}
		}
	} else {
		//indigo_debug("SELECTION does not contain the created/deleted PROPERTY\n");
	}
}


void BrowserWindow::clear_window() {
	QWidget* ppanel = new QWidget();
	QVBoxLayout* playout = new QVBoxLayout;
	playout->setSizeConstraint(QLayout::SetMinimumSize);
	ppanel->setLayout(playout);
	mScrollArea->setWidget(ppanel);
	mScrollArea->setWidgetResizable(true);
	ppanel->show();
}

void BrowserWindow::on_selection_changed(const QItemSelection &selected, const QItemSelection &) {
	indigo_debug( "SELECTION CHANGED\n");
	//current_selection = (QItemSelection*)&selected;

	//  Deal with the outgoing selection
	if (current_node != nullptr) {
		indigo_debug("SELECTION CHANGED no current node\n");
		clear_window();
	}

	if (selected.indexes().empty()) {
		indigo_debug("SELECTION CHANGED selected.indexes().empty()\n");
		clear_window();
		current_node = nullptr;
		return;
	}

	QModelIndex s = selected.indexes().front();
	TreeNode* n = static_cast<TreeNode*>(s.internalPointer());
	if (n != nullptr) {
		indigo_debug("SELECTION CHANGED n->node_type == %d\n", n->node_type);
		clear_window();
	}

	if (n != nullptr && n->node_type == TREE_NODE_PROPERTY) {
		indigo_debug( "SELECTION CHANGED n->node_type == TREE_NODE_PROPERTY\n");

		PropertyNode* p = reinterpret_cast<PropertyNode*>(n);
		QIndigoProperty* ip = new QIndigoProperty(p->property);
		current_node = p;

		QWidget* ppanel = new QWidget();
		QVBoxLayout* playout = new QVBoxLayout;
		playout->setSpacing(10);
		playout->setContentsMargins(10, 10, 10, 10);
		playout->setSizeConstraint(QLayout::SetMinimumSize);
		ppanel->setLayout(playout);
		playout->addWidget(ip);
		mScrollArea->setWidget(ppanel);
		ppanel->show();

		//  Connect to update signals coming from indigo bus
		connect(mPropertyModel, &PropertyModel::property_updated, ip, &QIndigoProperty::property_update);
	} else if (n != nullptr && n->node_type == TREE_NODE_GROUP) {
		indigo_debug("SELECTION CHANGED n->node_type == TREE_NODE_GROUP\n");
		GroupNode* g = reinterpret_cast<GroupNode*>(n);
		QWidget* ppanel = new QWidget();
		QVBoxLayout* playout = new QVBoxLayout;
		playout->setSpacing(10);
		playout->setContentsMargins(10, 10, 10, 10);
		playout->setSizeConstraint(QLayout::SetMinimumSize);
		ppanel->setLayout(playout);

        	//  Iterate properties
		for (int i = 0; i < g->children.count; i++) {
			PropertyNode* p = g->children.nodes[i];
			QIndigoProperty* ip = new QIndigoProperty(p->property);
			playout->addWidget(ip);
			connect(mPropertyModel, &PropertyModel::property_updated, ip, &QIndigoProperty::property_update);
		}
		playout->addStretch(); // Fill the vertical space available

		mScrollArea->setWidget(ppanel);
		mScrollArea->setWidgetResizable(true);
		ppanel->show();
	} else if (n != nullptr && n->node_type == TREE_NODE_DEVICE) {
		indigo_debug("SELECTION CHANGED n->node_type == TREE_NODE_DEVICE\n");
		clear_window();
	}
}


void BrowserWindow::on_servers_act() {
	mIndigoServers->show();
}


void BrowserWindow::on_exit_act() {
	QApplication::quit();
}


void BrowserWindow::on_blobs_changed(bool status) {
	conf.blobs_enabled = status;
	emit(enable_blobs(status));
	if(status) on_property_log(NULL, "BLOBs enabled");
	else on_property_log(NULL, "BLOBs disabled");
	write_conf();
	indigo_debug("%s\n", __FUNCTION__);
}


void BrowserWindow::on_bonjour_changed(bool status) {
	conf.auto_connect = status;
	write_conf();
	indigo_debug("%s\n", __FUNCTION__);
}


void BrowserWindow::on_use_suffix_changed(bool status) {
	conf.indigo_use_host_suffix = status;
	write_conf();
	indigo_debug("%s\n", __FUNCTION__);
}


void BrowserWindow::on_log_error() {
	conf.indigo_log_level = INDIGO_LOG_ERROR;
	indigo_set_log_level(conf.indigo_log_level);
	write_conf();
	indigo_error("%s\n", __FUNCTION__);
}


void BrowserWindow::on_log_info() {
	indigo_debug("%s\n", __FUNCTION__);
	conf.indigo_log_level = INDIGO_LOG_INFO;
	indigo_set_log_level(conf.indigo_log_level);
	write_conf();
}


void BrowserWindow::on_log_debug() {
	indigo_debug("%s\n", __FUNCTION__);
	conf.indigo_log_level = INDIGO_LOG_DEBUG;
	indigo_set_log_level(conf.indigo_log_level);
	write_conf();
}


void BrowserWindow::on_log_trace() {
	indigo_debug("%s\n", __FUNCTION__);
	conf.indigo_log_level = INDIGO_LOG_TRACE;
	indigo_set_log_level(conf.indigo_log_level);
	write_conf();
}


void BrowserWindow::on_about_act() {
	QMessageBox msgBox(this);
	QPixmap pixmap(":resource/appicon.png");
	msgBox.setWindowTitle("About INDIGO Panel");
	msgBox.setTextFormat(Qt::RichText);
	msgBox.setIconPixmap(pixmap.scaledToWidth(96, Qt::SmoothTransformation));
	msgBox.setText(
		"<b>INDIGO Control Panel</b><br>"
		"Version 0.1-b1</b><br><br>"
		"Authors:<br>"
		"David Hulse<br>"
		"Rumen G.Bogdanovski<br><br>"
		"Copyright ©2019, The INDIGO Initiative.<br>"
		"<a href='http://www.indigo-astronomy.org'>http://www.indigo-astronomy.org</a>"
	);
	msgBox.exec();
	indigo_debug ("%s\n", __FUNCTION__);
}
