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


#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <unistd.h>
#include "propertymodel.h"
#include "qindigoproperty.h"
#include <indigo_names.h>

extern indigo_client client;

TreeNode::~TreeNode() {
	indigo_debug("CALLED: %s on %p\n", __FUNCTION__, this);
}


DeviceNode::~DeviceNode() {
	indigo_debug("CALLED: %s on %p\n", __FUNCTION__, this);
	indigo_debug("=== relese DEVICE\n");
}


GroupNode::~GroupNode() {
	indigo_debug("CALLED: %s on %p\n", __FUNCTION__, this);
	indigo_debug("=== release GROUP\n");
}


PropertyNode::~PropertyNode() {
	indigo_debug("CALLED: %s on %p\n", __FUNCTION__, this);
	if (property) {
		indigo_release_property(property);
		property = nullptr;
	}
}


ItemNode::ItemNode(indigo_item* i, PropertyNode* parent) : TreeNodeWithParent<PropertyNode>(TREE_NODE_ITEM, parent), item(i) {
	input_control = nullptr;
}


ItemNode::~ItemNode() {
	indigo_debug("CALLED: %s\n", __FUNCTION__);
}

RootNode::~RootNode() {
	indigo_debug("CALLED: %s\n", __FUNCTION__);
}

PropertyModel::PropertyModel() {
	indigo_debug("CALLED: %s\n", __FUNCTION__);
}


void PropertyModel::define_property(indigo_property* property, const char *message) {
	//  Find or create TreeNode for property->device
	//indigo_debug("Defining device [%s],  group [%s],  property [%s]\n", property->device, property->group, property->name);
	int device_row = 0;
	DeviceNode* device = root.children.find_by_name_with_index(property->device, device_row);
	if (device == nullptr) {
		//  Insert the device in alphabetical order
		device = new DeviceNode(property->device, &root);

		device_row = root.children.find_insertion_index_by_label(property->device);
		beginInsertRows(QModelIndex(), device_row, device_row);
		root.children.insert_at(device_row, device);
		endInsertRows();
	}

	//  Find or create TreeNode within device for property->group
	int group_row = 0;
	GroupNode* group = device->children.find_by_name_with_index(property->group, group_row);
	if (group == nullptr) {
		//  Insert the group in alphabetical order
		group = new GroupNode(property->group, device);

		group_row = device->children.find_insertion_index_by_label(property->group);
		beginInsertRows(createIndex(device_row, 0, device), group_row, group_row);
		device->children.insert_at(group_row, group);
		endInsertRows();
	}

	//  Find or create TreeNode within group for property->name
	int property_row = 0;
	PropertyNode* p = group->children.find_by_name_with_index(property->name, property_row);
	if (p == nullptr) {
		//  Insert the property in alphabetical order
		p = new PropertyNode(property, group);

		property_row = group->children.find_insertion_index_by_label(property->label);
		beginInsertRows(createIndex(group_row, 0, group), property_row, property_row);
		group->children.insert_at(property_row, p);
		endInsertRows();

		//  Create the ItemNodes
		for (int i = 0; i < property->count; i++) {
			ItemNode* item = new ItemNode(&property->items[i], p);
			p->children.insert_at(i, item);
		}

		//  If this is a CONNECTION property - copy status to device node
		if (strcmp(property->name, CONNECTION_PROPERTY_NAME) == 0) {
			if (property->items[0].sw.value)
				device->state = INDIGO_OK_STATE;
			else
				device->state = INDIGO_IDLE_STATE;
		}
		// Change device icon according the device intrface
		if (strcmp(property->name, INFO_PROPERTY_NAME) == 0) {
			for (int i = 0; i < property->count; ++i) {
				if (strcmp(property->items[i].name, INFO_DEVICE_INTERFACE_ITEM_NAME) == 0) {
					device->m_interface = atoi(property->items[i].text.value);
					indigo_debug("Device interface [%s] = %04x\n",device->name(), device->m_interface);
					break;
				}
			}
		}
	}
	emit(property_defined(property, message));
	//indigo_debug("Defined device [%s],  group [%s],  property [%s]\n", property->device, property->group, property->name);
}


void PropertyModel::update_property(indigo_property* property, const char *message) {
	//  Find TreeNode for property->device
	int device_row = 0;
	DeviceNode* device = root.children.find_by_name_with_index(property->device, device_row);
	if (device == nullptr) {
		return;
	}
	//  Find TreeNode within device for property->group
	GroupNode* group = device->children.find_by_name(property->group);
	if (group == nullptr) {
		return;
	}
	//  Find TreeNode within group for property->name
	int row = 0;
	PropertyNode* p = group->children.find_by_name_with_index(property->name, row);
	if (p == nullptr) {
		return;
	}
	//  Update property
	p->property->state = property->state;
	memcpy(p->property->items, property->items, sizeof(indigo_item) * property->count);

	//  If there is a property widget attached, update it
	emit(property_updated(p->property, message));

	//  Emit a data changed signal so the tree can update (mainly for status LEDs)
	QModelIndex index = createIndex(row, 0, p);
	emit(dataChanged(index, index));

	//  If its a CONNECTION property - update device node also
	if (strcmp(property->name, CONNECTION_PROPERTY_NAME) == 0) {
		if (property->items[0].sw.value)
			device->state = INDIGO_OK_STATE;
		else
			device->state = INDIGO_IDLE_STATE;

		QModelIndex index = createIndex(device_row, 0, device);
		emit(dataChanged(index, index));
	}
}

void PropertyModel::delete_property(indigo_property* property, const char *message) {
	indigo_debug("Deleting property [%s] on device [%s]\n", property->name, property->device);

	//  Find TreeNode for property->device
	int device_row = 0;
	DeviceNode* device = root.children.find_by_name_with_index(property->device, device_row);
	if (device == nullptr) {
		indigo_debug("Deleting property on device [%s] - NOT FOUND\n", property->device);
		emit(property_deleted(property, message));
		delete property;
		//property = nullptr;
		return;
	}

	//  If we are deleting whole device - do that
	if ((property) && (strlen(property->group) == 0)) {
		beginRemoveRows(QModelIndex(), device_row, device_row);
		root.children.remove_index(device_row);
		endRemoveRows();
		emit(property_deleted(property, message));
		delete property;
		//property = nullptr;
		//delete device;
		//device = nullptr;
		return;
	}

	//  Find TreeNode within device for property->group
	indigo_debug("Deleting property in group [%s]\n", property->group);
	int group_row = 0;
	GroupNode* group = device->children.find_by_name_with_index(property->group, group_row);
	if ((property) && (group == nullptr)) {
		indigo_debug("Deleting property in group [%s] - NOT FOUND\n", property->group);
		emit(property_deleted(property, message));
		delete property;
		//property = nullptr;
		return;
	}

	//  If we are deleting whole group - do that
	if (strlen(property->name) == 0) {
		beginRemoveRows(createIndex(device_row, 0, device), group_row, group_row);
		device->children.remove_index(group_row);
		endRemoveRows();
		emit(property_deleted(property, message));
		delete property;
		//property = nullptr;
		//delete group;
		//group = nullptr;
		return;
	}

	//  Find or create TreeNode within group fro property->name
	indigo_debug("Deleting property [%s]\n", property->name);
	int property_row = 0;
	PropertyNode* p = group->children.find_by_name_with_index(property->name, property_row);
	if (p == nullptr) {
		indigo_debug("Deleting property [%s] - NOT FOUND\n", property->name);
		emit(property_deleted(property, message));
		delete property;
		//property = nullptr;
		return;
	}

	//bool group_empty = false;
	//bool device_empty = false;

	//  Remove the property
	indigo_debug("Erasing property [%s] in %p %p\n", property->name, device, group);
	beginRemoveRows(createIndex(group_row, 0, group), property_row, property_row);
	group->children.remove_index(property_row);
	endRemoveRows();
	indigo_debug("Erased property [%s]\n", property->name);


	char devname[255];
	char groupname[255];
	strcpy(devname, property->device);
	strcpy(groupname, property->group);

	//  Remove the group if empty
	if (group->children.empty()) {
		indigo_debug("--- REMOVING EMPTY GROUP %p -> [%s]\n", group, groupname);
		beginRemoveRows(createIndex(device_row, 0, device), group_row, group_row);
		device->children.remove_index(group_row);
		endRemoveRows();
		//group_empty = true;
		indigo_debug("--- REMOVED EMPTY GROUP [%s]\n", groupname);
	}

	//  Remove the device if empty
	if (device->children.empty()) {
		indigo_debug("--- REMOVING EMPTY DEVICE %p -> [%s]\n", device, devname);
		beginRemoveRows(QModelIndex(), device_row, device_row);
		root.children.remove_index(device_row);
		endRemoveRows();
		//device_empty = true;
		//device = nullptr;
		indigo_debug("--- REMOVED EMPTY DEVICE [%s]\n", devname);
	}

	emit(property_deleted(property, message));
	//delete p;
	//p = nullptr;
	delete property;

	//if (group_empty) delete group;
	//if (device_empty) delete device;
}



QModelIndex PropertyModel::index(int row, int column, const QModelIndex &parent) const {
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	if (!parent.isValid()) {
		//indigo_debug("INDEX of node type %d returning %d,%d\n", TREE_NODE_ROOT, row, column);
		DeviceNode* n = root.children[row];
		return createIndex(row,column,n);
	}
	else {
		TreeNode* n = reinterpret_cast<TreeNode*>(parent.internalPointer());
		//indigo_debug("->INDEX of node type %d returning %d,%d\n", n->node_type, row, column);
		n = (*n)[row];
		//indigo_debug("  Yields row of node type %d\n", n->node_type);
		return createIndex(row,column,n);
	}
}


/* THIS fails:
Process terminating with default action of signal 11 (SIGSEGV)
==29106==  Access not within mapped region at address 0x0
*/
QModelIndex PropertyModel::parent(const QModelIndex &child) const {
	if (!child.isValid())
		return QModelIndex();

	TreeNode* node = reinterpret_cast<TreeNode*>(child.internalPointer());
	if (node == nullptr) {
		return QModelIndex();
	}
	//indigo_debug("*********************** child %p -> parent %p\n", child, node);
	if (node->node_type == TREE_NODE_DEVICE)
		return QModelIndex();

	if (!node->parent())
		return QModelIndex();

	int row = node->parent()->index_of(node);
	//indigo_debug("  Yields row %d of parent of node type %d\n", row, node->parent()->node_type);
	return createIndex(row,0, node->parent());
}


int PropertyModel::rowCount(const QModelIndex &parent) const {
	if (!parent.isValid())
		return root.size();
	TreeNode* node = static_cast<TreeNode*>(parent.internalPointer());
	//indigo_debug("ROWS for node type %d -> %d\n", node->node_type, node->size());
	return node->size();
}


int PropertyModel::columnCount(const QModelIndex &) const {
	return 1;
}

QVariant PropertyModel::data(const QModelIndex &index, int role) const {
	if (!index.isValid())
		return QVariant();

	TreeNode* node = static_cast<TreeNode*>(index.internalPointer());
	if (node) {
		switch (role) {
		case Qt::DisplayRole:
			//indigo_debug("DATA for node type %d [%s]\n", node->node_type, node->label());
			return QString::fromStdString(node->label());
		case Qt::DecorationRole: {
			if (node->node_type == TREE_NODE_PROPERTY) {
				PropertyNode* p = reinterpret_cast<PropertyNode*>(node);
				switch (p->property->state) {
				case INDIGO_IDLE_STATE:
					return QPixmap(":resource/led-grey.png");
				case INDIGO_BUSY_STATE:
					return QPixmap(":resource/led-orange.png");
				case INDIGO_ALERT_STATE:
					return QPixmap(":resource/led-red.png");
				case INDIGO_OK_STATE:
					return QPixmap(":resource/led-green.png");
				}
			} else if (node->node_type == TREE_NODE_DEVICE) {
				DeviceNode* d = reinterpret_cast<DeviceNode*>(node);
				if (d->m_interface & INDIGO_INTERFACE_CCD) {
					switch (d->state) {
					case INDIGO_OK_STATE:
						return QPixmap(":resource/ccd-green.png");
					default:
						return QPixmap(":resource/ccd-grey.png");
					}
				}
				if (d->m_interface & INDIGO_INTERFACE_MOUNT) {
					switch (d->state) {
					case INDIGO_OK_STATE:
						return QPixmap(":resource/mount-green.png");
					default:
						return QPixmap(":resource/mount-grey.png");
					}
				}
				if (d->m_interface & INDIGO_INTERFACE_GUIDER) {
					switch (d->state) {
					case INDIGO_OK_STATE:
						return QPixmap(":resource/guider-green.png");
					default:
						return QPixmap(":resource/guider-grey.png");
					}
				}
				if (d->m_interface & INDIGO_INTERFACE_WHEEL) {
					switch (d->state) {
					case INDIGO_OK_STATE:
						return QPixmap(":resource/wheel-green.png");
					default:
						return QPixmap(":resource/wheel-grey.png");
					}
				}
				if (d->m_interface == 0) { // server
						return QPixmap(":resource/server.png");
				}
				switch (d->state) {
				case INDIGO_IDLE_STATE:
					return QPixmap(":resource/led-grey-dev.png");
				case INDIGO_BUSY_STATE:
					return QPixmap(":resource/led-orange.png");
				case INDIGO_ALERT_STATE:
					return QPixmap(":resource/led-red.png");
				case INDIGO_OK_STATE:
					return QPixmap(":resource/led-green-dev.png");
				}
			} else {
				return QVariant();
			}
		}
		}
	} else {
		indigo_debug("Missing pointer\n");
	}
	return QVariant();
}

void PropertyModel::enable_blobs(bool on) {
	for (int dev_index = 0; dev_index < root.size(); dev_index++) {
		DeviceNode* dev_node = static_cast<DeviceNode*>(root.children[dev_index]);
		if (dev_node == nullptr) continue;
		for (int group_index = 0; group_index < dev_node->size(); group_index++) {
			GroupNode* group_node = static_cast<GroupNode*>(dev_node->children[group_index]);
			if (group_node == nullptr) continue;
			for (int prop_index = 0; prop_index < group_node->size(); prop_index++) {
				PropertyNode* prop_node = static_cast<PropertyNode*>(group_node->children[prop_index]);
				if (prop_node == nullptr) continue;
				indigo_property *property = prop_node->property;
				if (property == nullptr) continue;
				if (property->type == INDIGO_BLOB_VECTOR) {
					if ((on) && (property->version >= INDIGO_VERSION_2_0)) {
						indigo_enable_blob(&client, property, INDIGO_ENABLE_BLOB_URL);
					} else if (on) {
						indigo_enable_blob(&client, property, INDIGO_ENABLE_BLOB_ALSO);
					} else {
						indigo_enable_blob(&client, property, INDIGO_ENABLE_BLOB_NEVER);
					}
					indigo_debug("BLOB %s.%s -> mode = %d\n", property->device, property->name, on);
				}
			}
		}
	}
}
