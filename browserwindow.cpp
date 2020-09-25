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
#include <QFileDialog>
#include <sys/time.h>
#include "browserwindow.h"
#include "qservicemodel.h"
#include "propertymodel.h"
#include "indigoclient.h"
#include "qindigoproperty.h"
#include "qindigoservers.h"
#include "blobpreview.h"
#include "logger.h"
#include "conf.h"
#include "version.h"

void write_conf();

BrowserWindow::BrowserWindow(QWidget *parent) : QMainWindow(parent) {
	setWindowTitle(tr("INDIGO Control Panel"));
	resize(1024, 768);

	QIcon icon(":resource/appicon.png");
	this->setWindowIcon(icon);

	QFile f(":resource/control_panel.qss");
	f.open(QFile::ReadOnly | QFile::Text);
	QTextStream ts(&f);
	this->setStyleSheet(ts.readAll());
	f.close();

	current_path = new SelectionPath();
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

	//  Create log viewer
	mLog = new QPlainTextEdit;
	mLog->setReadOnly(true);

	// Create menubar
	QMenuBar *menu_bar = new QMenuBar;
	QMenu *menu = new QMenu("&File");
	QAction *act;

	act = menu->addAction(tr("&Manage services"));
	connect(act, &QAction::triggered, this, &BrowserWindow::on_servers_act);

	menu->addSeparator();

	act = menu->addAction(tr("&Load Device ACL..."));
	connect(act, &QAction::triggered, this, &BrowserWindow::on_acl_load_act);

	act = menu->addAction(tr("&Append to Device ACL..."));
	connect(act, &QAction::triggered, this, &BrowserWindow::on_acl_append_act);

	act = menu->addAction(tr("&Save Device ACL As..."));
	connect(act, &QAction::triggered, this, &BrowserWindow::on_acl_save_act);

	act = menu->addAction(tr("&Clear Device ACL"));
	connect(act, &QAction::triggered, this, &BrowserWindow::on_acl_clear_act);

	menu->addSeparator();

	act = menu->addAction(tr("&Exit"));
	connect(act, &QAction::triggered, this, &BrowserWindow::on_exit_act);

	menu_bar->addMenu(menu);

	menu = new QMenu("&Edit");
	act = menu->addAction(tr("Clear &messages"));
	connect(act, &QAction::triggered, mLog, &QPlainTextEdit::clear);
	menu_bar->addMenu(menu);

	menu = new QMenu("&Settings");

	act = menu->addAction(tr("Enable &BLOBs"));
	act->setCheckable(true);
	act->setChecked(conf.blobs_enabled);
	connect(act, &QAction::toggled, this, &BrowserWindow::on_blobs_changed);

	act = menu->addAction(tr("Enable auto &connect"));
	act->setCheckable(true);
	act->setChecked(conf.auto_connect);
	connect(act, &QAction::toggled, this, &BrowserWindow::on_bonjour_changed);

	act = menu->addAction(tr("&Use host suffix"));
	act->setCheckable(true);
	act->setChecked(conf.indigo_use_host_suffix);
	connect(act, &QAction::toggled, this, &BrowserWindow::on_use_suffix_changed);

	act = menu->addAction(tr("Use property state &icons"));
	act->setCheckable(true);
	act->setChecked(conf.use_state_icons);
	connect(act, &QAction::toggled, this, &BrowserWindow::on_use_state_icons_changed);

	act = menu->addAction(tr("Use locale specific &decimal separator"));
	act->setCheckable(true);
	act->setChecked(conf.use_system_locale);
	connect(act, &QAction::toggled, this, &BrowserWindow::on_use_system_locale_changed);

	menu->addSeparator();
	QActionGroup *stretch_group = new QActionGroup(this);
	stretch_group->setExclusive(true);

	act = menu->addAction("Preview Levels Stretch: N&one");
	act->setCheckable(true);
	if (conf.preview_stretch_level == STRETCH_NONE) act->setChecked(true);
	connect(act, &QAction::triggered, this, &BrowserWindow::on_no_stretch);
	stretch_group->addAction(act);

	act = menu->addAction("Preview Levels Stretch: &Normal");
	act->setCheckable(true);
	if (conf.preview_stretch_level == STRETCH_NORMAL) act->setChecked(true);
	connect(act, &QAction::triggered, this, &BrowserWindow::on_normal_stretch);
	stretch_group->addAction(act);

	act = menu->addAction("Preview Levels Stretch: &Hard");
	act->setCheckable(true);
	if (conf.preview_stretch_level == STRETCH_HARD) act->setChecked(true);
	connect(act, &QAction::triggered, this, &BrowserWindow::on_hard_stretch);
	stretch_group->addAction(act);

	menu->addSeparator();
	QActionGroup *log_group = new QActionGroup(this);
	log_group->setExclusive(true);

	act = menu->addAction("Log &Error");
	act->setCheckable(true);
	if (conf.indigo_log_level == INDIGO_LOG_ERROR) act->setChecked(true);
	connect(act, &QAction::triggered, this, &BrowserWindow::on_log_error);
	log_group->addAction(act);

	act = menu->addAction("Log &Info");
	act->setCheckable(true);
	if (conf.indigo_log_level == INDIGO_LOG_INFO) act->setChecked(true);
	connect(act, &QAction::triggered, this, &BrowserWindow::on_log_info);
	log_group->addAction(act);

	act = menu->addAction("Log &Debug");
	act->setCheckable(true);
	if (conf.indigo_log_level == INDIGO_LOG_DEBUG) act->setChecked(true);
	connect(act, &QAction::triggered, this, &BrowserWindow::on_log_debug);
	log_group->addAction(act);

	act = menu->addAction("Log &Trace");
	act->setCheckable(true);
	if (conf.indigo_log_level == INDIGO_LOG_TRACE) act->setChecked(true);
	connect(act, &QAction::triggered, this, &BrowserWindow::on_log_trace);
	log_group->addAction(act);

	menu_bar->addMenu(menu);

	menu = new QMenu("&Help");

	act = menu->addAction(tr("&About"));
	connect(act, &QAction::triggered, this, &BrowserWindow::on_about_act);
	menu_bar->addMenu(menu);

	rootLayout->addWidget(menu_bar);

	// Create properties viewing area
	QWidget *view = new QWidget;
	QVBoxLayout *propertyLayout = new QVBoxLayout;
	propertyLayout->setSpacing(5);
	propertyLayout->setContentsMargins(5, 5, 5, 5);
	propertyLayout->setSizeConstraint(QLayout::SetMinimumSize);
	view->setLayout(propertyLayout);
	rootLayout->addWidget(view);

	mProperties = new QTreeView;
	QWidget *form_panel = new QWidget();
	mFormLayout = new QVBoxLayout();
	mFormLayout->setSpacing(0);
	mFormLayout->setContentsMargins(1, 0, 0, 0);
	//mFormLayout->setMargin(0);
	form_panel->setLayout(mFormLayout);

	QWidget *selection_panel = new QWidget();
	QVBoxLayout *selection_layout = new QVBoxLayout();
	selection_layout->setSpacing(0);
	selection_layout->setContentsMargins(0, 0, 1, 0);
	//selection_layout->setMargin(0);
	selection_panel->setLayout(selection_layout);
	selection_layout->addWidget(mProperties);

	mSelectionLine = new QLabel();
	mSelectionLine->setObjectName("SELECTION_TEXT");

	mScrollArea = new QScrollArea();
	mScrollArea->setObjectName("PROPERTY_AREA");

	mScrollArea->setWidgetResizable(true);
	mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	mScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	mFormLayout->addWidget(mSelectionLine);
	mFormLayout->addWidget(mScrollArea);
	mScrollArea->setMinimumWidth(PROPERTY_AREA_MIN_WIDTH);

	QSplitter* hSplitter = new QSplitter;
	hSplitter->addWidget(selection_panel);
	hSplitter->addWidget(form_panel);
	hSplitter->setStretchFactor(0, 45);
	hSplitter->setStretchFactor(2, 55);
	propertyLayout->addWidget(hSplitter, 85);


	propertyLayout->addWidget(mLog, 15);

	mServiceModel = new QServiceModel("_indigo._tcp");
	mServiceModel->enable_auto_connect(conf.auto_connect);

	mPropertyModel = new PropertyModel();
	mProperties->setHeaderHidden(true);
	mProperties->setModel(mPropertyModel);

	connect(mServiceModel, &QServiceModel::serviceAdded, mIndigoServers, &QIndigoServers::onAddService);
	connect(mServiceModel, &QServiceModel::serviceRemoved, mIndigoServers, &QIndigoServers::onRemoveService);
	connect(mServiceModel, &QServiceModel::serviceConnectionChange, mIndigoServers, &QIndigoServers::onConnectionChange);

	connect(mIndigoServers, &QIndigoServers::requestConnect, mServiceModel, &QServiceModel::onRequestConnect);
	connect(mIndigoServers, &QIndigoServers::requestDisconnect, mServiceModel, &QServiceModel::onRequestDisconnect);
	connect(mIndigoServers, &QIndigoServers::requestAddManualService, mServiceModel, &QServiceModel::onRequestAddManualService);
	connect(mIndigoServers, &QIndigoServers::requestRemoveManualService, mServiceModel, &QServiceModel::onRequestRemoveManualService);

	// NOTE: logging should be before update and delete of properties as they release the copy!!!
	connect(&IndigoClient::instance(), &IndigoClient::property_defined, this, &BrowserWindow::on_message_sent);
	connect(&IndigoClient::instance(), &IndigoClient::property_changed, this, &BrowserWindow::on_message_sent);
	connect(&IndigoClient::instance(), &IndigoClient::property_deleted, this, &BrowserWindow::on_message_sent);
	connect(&IndigoClient::instance(), &IndigoClient::message_sent, this, &BrowserWindow::on_message_sent);

	connect(&IndigoClient::instance(), &IndigoClient::create_preview, this, &BrowserWindow::on_create_preview);
	connect(&IndigoClient::instance(), &IndigoClient::obsolete_preview, this, &BrowserWindow::on_obsolete_preview);
	connect(&IndigoClient::instance(), &IndigoClient::remove_preview, this, &BrowserWindow::on_remove_preview);

	connect(&IndigoClient::instance(), &IndigoClient::property_defined, mPropertyModel, &PropertyModel::define_property);
	connect(&IndigoClient::instance(), &IndigoClient::property_changed, mPropertyModel, &PropertyModel::update_property);
	connect(&IndigoClient::instance(), &IndigoClient::property_deleted, mPropertyModel, &PropertyModel::delete_property);

	connect(mPropertyModel, &PropertyModel::property_defined, this, &BrowserWindow::on_property_define);
	connect(mPropertyModel, &PropertyModel::property_deleted, this, &BrowserWindow::on_property_delete);

	connect(&Logger::instance(), &Logger::do_log, this, &BrowserWindow::on_window_log);

	connect(mProperties->selectionModel(), &QItemSelectionModel::selectionChanged, this, &BrowserWindow::on_selection_changed);
	connect(this, &BrowserWindow::enable_blobs, mPropertyModel, &PropertyModel::enable_blobs);
	connect(this, &BrowserWindow::rebuild_blob_previews, mPropertyModel, &PropertyModel::rebuild_blob_previews);

	preview_cache.set_stretch_level(conf.preview_stretch_level);

	//  Start up the client
	IndigoClient::instance().enable_blobs(conf.blobs_enabled);
	IndigoClient::instance().start("INDIGO Control Panel");

	// load manually configured services
	mServiceModel->loadManualServices();
}


BrowserWindow::~BrowserWindow () {
	indigo_debug("CALLED: %s\n", __FUNCTION__);
	delete mLog;
	delete mProperties;
	delete mSelectionLine;
	delete mFormLayout;
	delete mIndigoServers;
	delete mServiceModel;
	delete mPropertyModel;
	delete current_path;
}

void BrowserWindow::on_create_preview(indigo_property *property, indigo_item *item){
	preview_cache.create(property, item);
}

void BrowserWindow::on_obsolete_preview(indigo_property *property, indigo_item *item){
	preview_cache.obsolete(property, item);
}

void BrowserWindow::on_remove_preview(indigo_property *property, indigo_item *item){
	preview_cache.remove(property, item);
}

void BrowserWindow::on_message_sent(indigo_property* property, char *message) {
	on_window_log(property, message);
	free(message);
}

void BrowserWindow::on_window_log(indigo_property* property, char *message) {
	char timestamp[16];
	char log_line[512];
	char message_line[512];
	struct timeval tmnow;

	if (!message) return;

	gettimeofday(&tmnow, NULL);
#if defined(INDIGO_WINDOWS)
	struct tm *lt;
	time_t rawtime;
	lt = localtime((const time_t *) &(tmnow.tv_sec));
	if (lt == NULL) {
		time(&rawtime);
		lt = localtime(&rawtime);
	}
	strftime(timestamp, sizeof(log_line), "%H:%M:%S", lt);
#else
	strftime(timestamp, sizeof(log_line), "%H:%M:%S", localtime((const time_t *) &tmnow.tv_sec));
#endif
	snprintf(timestamp + 8, sizeof(timestamp) - 8, ".%03ld", tmnow.tv_usec/1000);

	if (property) {
		snprintf(message_line, 512, "%s.%s: %s", property->device, property->name, message);
		switch (property->state) {
		case INDIGO_ALERT_STATE:
			snprintf(log_line, 512, "<font color = \"#E00000\">%s %s<\font>", timestamp, message_line);
			break;
		case INDIGO_BUSY_STATE:
			snprintf(log_line, 512, "<font color = \"orange\">%s %s<\font>", timestamp, message_line);
			break;
		default:
			snprintf(log_line, 512, "%s %s", timestamp, message_line);
			break;
		}
		indigo_debug("[message] %s\n", message_line);
	} else {
		snprintf(log_line, 512, "%s %s", timestamp, message);
		indigo_debug("[message] %s\n", message);
	}
	mLog->appendHtml(log_line); // Adds the message to the widget
}

void BrowserWindow::on_property_define(indigo_property* property, char *message) {
	property_define_delete(property, message, false);
}

void BrowserWindow::on_property_delete(indigo_property* property, char *message) {
	property_define_delete(property, message, true);
}

void BrowserWindow::property_define_delete(indigo_property* property, char *message, bool action_deleted) {
	Q_UNUSED(message);

	if (current_path->type == TREE_NODE_ROOT) return;

	if (current_path->type == TREE_NODE_PROPERTY) {
		/* We can not have selected property to be defined -> so just clean window */
		if (!strcmp(current_path->property, property->name) &&
		    !strcmp(current_path->device, property->device) &&
		    !strcmp(current_path->group, property->group)) {
			if (action_deleted) {
				indigo_debug("SELECTED PROPERTY deleted\n");
				clear_window();
			}
			return;
		}
	} else if (current_path->type == TREE_NODE_GROUP) {
		/* If we have deleted or defined property update group window */
		if (!strcmp(current_path->device, property->device) &&
		    !strcmp(current_path->group, property->group)) {
			indigo_debug("PROPERTY IN GROUP defined/deleted\n");
			clear_window();
			repaint_property_window(current_path->node);
			return;
		}
	}
	if (!strcmp(property->name,"")) {
		if (current_path->type == TREE_NODE_PROPERTY) {
			if (!strcmp(current_path->device, property->device)) {
				indigo_debug("SELECTED DEVICE deleted (Peoperty selected)\n");
				current_path->ClearSelection();
				clear_window();
				return;
			}
		} else if (current_path->type == TREE_NODE_GROUP) {
			if (!strcmp(current_path->device, property->device)) {
				indigo_debug("SELECTED DEVICE deleted (Group selected)\n");
				current_path->ClearSelection();
				clear_window();
				return;
			}
		}
	}
}


void BrowserWindow::clear_window() {
	indigo_debug("CLEAR_WINDOW!\n");
	mSelectionLine->setText("");
	delete mScrollArea;

	mScrollArea = new QScrollArea();
	mScrollArea->setObjectName("PROPERTY_AREA");
	mScrollArea->setWidgetResizable(true);
	mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	mScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	mFormLayout->addWidget(mScrollArea);
	mScrollArea->setMinimumWidth(600);
}

void BrowserWindow::repaint_property_window(TreeNode* node) {
	char selected_str[PATH_LEN] = "";

	if (node != nullptr && node->node_type == TREE_NODE_PROPERTY) {
		indigo_debug( "SELECTION CHANGED current_node->node_type == TREE_NODE_PROPERTY\n");

		snprintf(selected_str, PATH_LEN, "%s . %s", current_path->device, current_path->property);
		mSelectionLine->setText(selected_str);

		PropertyNode* p = reinterpret_cast<PropertyNode*>(node);
		QIndigoProperty* ip = new QIndigoProperty(p->property);

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
	} else if (node != nullptr && node->node_type == TREE_NODE_GROUP) {
		indigo_debug("SELECTION CHANGED node->node_type == TREE_NODE_GROUP\n");
		GroupNode* g = reinterpret_cast<GroupNode*>(node);
		QWidget* ppanel = new QWidget();
		QVBoxLayout* playout = new QVBoxLayout;
		playout->setSpacing(10);
		playout->setContentsMargins(10, 10, 10, 10);
		playout->setSizeConstraint(QLayout::SetMinimumSize);
		ppanel->setLayout(playout);
		snprintf(selected_str, PATH_LEN, "%s . %s", current_path->device, current_path->group);
		mSelectionLine->setText(selected_str);

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
	} else if (node != nullptr && node->node_type == TREE_NODE_DEVICE) {
		indigo_debug("SELECTION CHANGED node->node_type == TREE_NODE_DEVICE\n");
		DeviceNode* d = reinterpret_cast<DeviceNode*>(node);
		//  Iterate Groups and find "Main" and select it
		for (int i = 0; i < d->children.count; i++) {
			GroupNode* g = d->children.nodes[i];
			if (!strcmp(g->name(), "Main")) {
				current_path->select(g);
				repaint_property_window(current_path->node);
				return;
			}
		}
		// There is no "Main" goroup
		indigo_debug("No \"Main\" group, clearing window.\n");
		clear_window();
		snprintf(selected_str, PATH_LEN, "%s", current_path->device);
		mSelectionLine->setText(selected_str);
	}
}

void BrowserWindow::on_selection_changed(const QItemSelection &selected, const QItemSelection &) {
	if (mPropertyModel->no_repaint_flag) return;
	indigo_debug( "SELECTION CHANGED\n");

	//  Deal with the outgoing selection
	if (current_path->type != TREE_NODE_ROOT) {
		indigo_debug("SELECTION CHANGED no current node\n");
		clear_window();
	}

	if (selected.indexes().empty()) {
		indigo_debug("SELECTION CHANGED selected.indexes().empty()\n");
		current_path->select(nullptr);
		clear_window();
		return;
	}

	QModelIndex s = selected.indexes().front();
	current_path->select(static_cast<TreeNode*>(s.internalPointer()));
	if (current_path->node != nullptr) {
		indigo_debug("SELECTION CHANGED current_node->node_type == %d\n", current_path->node->node_type);
		clear_window();
	}

	repaint_property_window(current_path->node);
}


void BrowserWindow::on_servers_act() {
	mIndigoServers->show();
}


void BrowserWindow::on_exit_act() {
	QApplication::quit();
}


void BrowserWindow::on_blobs_changed(bool status) {
	conf.blobs_enabled = status;
	IndigoClient::instance().enable_blobs(status);
	emit(enable_blobs(status));
	if(status) on_window_log(NULL, "BLOBs enabled");
	else on_window_log(NULL, "BLOBs disabled");
	write_conf();
	indigo_debug("%s\n", __FUNCTION__);
}


void BrowserWindow::on_bonjour_changed(bool status) {
	conf.auto_connect = status;
	mServiceModel->enable_auto_connect(conf.auto_connect);
	write_conf();
	indigo_debug("%s\n", __FUNCTION__);
}


void BrowserWindow::on_use_suffix_changed(bool status) {
	conf.indigo_use_host_suffix = status;
	write_conf();
	indigo_debug("%s\n", __FUNCTION__);
}


void BrowserWindow::on_use_state_icons_changed(bool status) {
	conf.use_state_icons = status;
	write_conf();
	mProperties->repaint();
	repaint_property_window(current_path->node);
	indigo_debug("%s\n", __FUNCTION__);
}


void BrowserWindow::on_use_system_locale_changed(bool status) {
	conf.use_system_locale = status;
	write_conf();
	if (conf.use_system_locale){
		on_window_log(nullptr, "Locale specific decimal separator will be used on next application start");
	} else {
		on_window_log(nullptr, "Dot decimal separator will be used on next application start");
	}
	indigo_debug("%s\n", __FUNCTION__);
}


void BrowserWindow::on_no_stretch() {
	conf.preview_stretch_level = STRETCH_NONE;
	preview_cache.set_stretch_level(conf.preview_stretch_level);
	emit(rebuild_blob_previews());
	repaint_property_window(current_path->node);
	write_conf();
	indigo_error("%s\n", __FUNCTION__);
}


void BrowserWindow::on_normal_stretch() {
	conf.preview_stretch_level = STRETCH_NORMAL;
	preview_cache.set_stretch_level(conf.preview_stretch_level);
	emit(rebuild_blob_previews());
	repaint_property_window(current_path->node);
	write_conf();
	indigo_error("%s\n", __FUNCTION__);
}


void BrowserWindow::on_hard_stretch() {
	conf.preview_stretch_level = STRETCH_HARD;
	preview_cache.set_stretch_level(conf.preview_stretch_level);
	emit(rebuild_blob_previews());
	repaint_property_window(current_path->node);
	write_conf();
	indigo_error("%s\n", __FUNCTION__);
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


void BrowserWindow::on_acl_load_act() {
	QString filter = "INDIGO Device Access Control (*.idac);; All files (*)";
	QString file_name = QFileDialog::getOpenFileName(this, "Load Device ACL...", QDir::currentPath(), filter);
	if (!file_name.isNull()) {
		char fname[PATH_MAX];
		strcpy(fname, file_name.toStdString().c_str());
		char message[PATH_MAX];
		indigo_clear_device_tokens();
		if (indigo_load_device_tokens_from_file(fname)) {
			snprintf(message, PATH_MAX, "Current device ACL cleared, new device ACL loaded from '%s'", fname);
		} else {
			snprintf(message, PATH_MAX, "Current device ACL cleared but failed to load device ACL from '%s'", fname);
		}
		on_window_log(NULL, message);
	}
	indigo_debug("%s\n", __FUNCTION__);
}

void BrowserWindow::on_acl_append_act() {
	QString filter = "INDIGO Device Access Control (*.idac);; All files (*)";
	QString file_name = QFileDialog::getOpenFileName(this, "Append to device ACL...", QDir::currentPath(), filter);
	if (!file_name.isNull()) {
		char fname[PATH_MAX];
		strcpy(fname, file_name.toStdString().c_str());
		char message[PATH_MAX];
		if (indigo_load_device_tokens_from_file(fname)) {
			snprintf(message, PATH_MAX, "Appended to device ACL from '%s'", fname);
		} else {
			snprintf(message, PATH_MAX, "Failed to append to device ACL form '%s'", fname);
		}
		on_window_log(NULL, message);
	}
	indigo_debug("%s\n", __FUNCTION__);
}

void BrowserWindow::on_acl_save_act() {
	QString filter = "INDIGO Device Access Control (*.idac);; All files (*)";
	QString file_name = QFileDialog::getSaveFileName(this, "Save device ACL As...", QDir::currentPath(), filter);
	if (!file_name.isNull()) {
		char fname[PATH_MAX];
		strcpy(fname, file_name.toStdString().c_str());
		char message[PATH_MAX];
		if (indigo_save_device_tokens_to_file(fname)) {
			snprintf(message, PATH_MAX, "Device ACL saved as '%s'", fname);
		} else {
			snprintf(message, PATH_MAX, "Failed to save device ACL as '%s'", fname);
		}
		on_window_log(NULL, message);
	}
	indigo_debug("%s\n", __FUNCTION__);
}

void BrowserWindow::on_acl_clear_act() {
	indigo_clear_device_tokens();
	on_window_log(NULL, "Device ACL cleared");
	indigo_debug("%s\n", __FUNCTION__);
}


void BrowserWindow::on_about_act() {
	QMessageBox msgBox(this);
	QPixmap pixmap(":resource/indigo_logo.png");
	msgBox.setWindowTitle("About INDIGO Panel");
	msgBox.setTextFormat(Qt::RichText);
	msgBox.setIconPixmap(pixmap.scaledToWidth(96, Qt::SmoothTransformation));
	msgBox.setText(
		"<b>INDIGO Control Panel</b><br>"
		"Version "
		PANEL_VERSION
		"</b><br><br>"
		"Authors:<br>"
		"Rumen G.Bogdanovski<br>"
		"David Hulse<br><br>"
		"You can use this software under the terms of <b>INDIGO Astronomy open-source license</b><br><br>"
		"Copyright ©2019, The INDIGO Initiative.<br>"
		"<a href='http://www.indigo-astronomy.org'>http://www.indigo-astronomy.org</a>"
	);
	msgBox.exec();
	indigo_debug("%s\n", __FUNCTION__);
}
