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


#ifndef BROWSERWINDOW_H
#define BROWSERWINDOW_H

#include <QApplication>
#include <QMainWindow>
#include <indigo/indigo_bus.h>
#include <propertymodel.h>

class QPlainTextEdit;
class QTreeView;
class ServiceModel;
class QItemSelection;
class QVBoxLayout;
class QScrollArea;
class QIndigoServers;

struct SelectionPath {

	SelectionPath() {
		ClearSelection();
	};

	void ClearSelection() {
		device[0]= 0;
		group[0] = 0;
		property[0] = 0;
		node = nullptr;
		type = TREE_NODE_ROOT;
	};

	void select(TreeNode *node) {
		this->node = node;
		if (node == nullptr) {
			device[0]= 0;
			group[0] = 0;
			property[0] = 0;
			type = TREE_NODE_ROOT;
		} else if (node->node_type == TREE_NODE_PROPERTY) {
			PropertyNode* p = reinterpret_cast<PropertyNode*>(node);
			strncpy(group, p->property->group, sizeof(group));
			strncpy(device, p->property->device, sizeof(device));
			strncpy(property, p->property->name, sizeof(property));
			type = TREE_NODE_PROPERTY;
		} else if (node->node_type == TREE_NODE_GROUP) {
			GroupNode* g = reinterpret_cast<GroupNode*>(node);
			strncpy(group, g->name(), sizeof(group));
			strncpy(device, g->device(), sizeof(device));
			property[0] = 0;
			type = TREE_NODE_GROUP;
		} else if (node->node_type == TREE_NODE_DEVICE) {
			DeviceNode* d = reinterpret_cast<DeviceNode*>(node);
			strncpy(device, d->name(), sizeof(device));
			group[0] = 0;
			property[0] = 0;
			type = TREE_NODE_DEVICE;
		}
	}

	TreeNodeType type;
	char property[INDIGO_NAME_SIZE];
	char group[INDIGO_NAME_SIZE];
	char device[INDIGO_NAME_SIZE];
	TreeNode *node;
};


class BrowserWindow : public QMainWindow {
	Q_OBJECT
public:
	explicit BrowserWindow(QWidget *parent = nullptr);
	virtual ~BrowserWindow();
	void property_define_delete(indigo_property* property, char *message, bool action_deleted);
	void repaint_property_window(TreeNode* node);

signals:
	void enable_blobs(bool on);
	void rebuild_blob_previews();

public slots:
	void on_selection_changed(const QItemSelection &selected, const QItemSelection &deselected);
	void on_property_log(indigo_property* property, char *message);
	void on_property_define(indigo_property* property, char *message);
	void on_property_delete(indigo_property* property, char *message);
	void on_blobs_changed(bool status);
	void on_bonjour_changed(bool status);
	void on_use_suffix_changed(bool status);
	void on_use_state_icons_changed(bool status);
	void on_use_system_locale_changed(bool status);
	void on_log_error();
	void on_log_info();
	void on_log_debug();
	void on_log_trace();
	void on_servers_act();
	void on_exit_act();
	void on_about_act();
	void on_no_stretch();
	void on_normal_stretch();
	void on_hard_stretch();

private:
	QPlainTextEdit* mLog;
	QTreeView* mProperties;
	QScrollArea* mScrollArea;
	QLabel* mSelectionLine;
	QVBoxLayout* mFormLayout;

	QIndigoServers *mIndigoServers;
	ServiceModel* mServiceModel;
	PropertyModel* mPropertyModel;
	SelectionPath* current_path;

	void clear_window();
};

#endif // BROWSERWINDOW_H
