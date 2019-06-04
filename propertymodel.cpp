#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include "propertymodel.h"
#include "qindigoproperty.h"
#include <indigo_names.h>


TreeNode::~TreeNode() {
	printf("CALLED: %s\n", __FUNCTION__);
}


DeviceNode::~DeviceNode() {
	printf("CALLED: %s\n", __FUNCTION__);
	printf("=== relese DEVICE\n");
}


GroupNode::~GroupNode() {
	printf("CALLED: %s\n", __FUNCTION__);
	printf("=== release GROUP\n");
}


PropertyNode::~PropertyNode() {
	printf("CALLED: %s\n", __FUNCTION__);
	indigo_release_property(property);
}


ItemNode::ItemNode(indigo_item* i, PropertyNode* parent) : TreeNodeWithParent<PropertyNode>(TREE_NODE_ITEM, parent), item(i) {
	input_control = nullptr;
}


ItemNode::~ItemNode() {
	printf("CALLED: %s\n", __FUNCTION__);
}

RootNode::~RootNode() {
	printf("CALLED: %s\n", __FUNCTION__);
}

PropertyModel::PropertyModel() {
	printf("CALLED: %s\n", __FUNCTION__);
}


void PropertyModel::define_property(indigo_property* property, const char *message) {
	//  Find or create TreeNode for property->device
	//fprintf(stderr, "Defining device [%s],  group [%s],  property [%s]\n", property->device, property->group, property->name);
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
	}
	//fprintf(stderr, "Defined device [%s],  group [%s],  property [%s]\n", property->device, property->group, property->name);
}


void PropertyModel::update_property(indigo_property* property, const char *message) {
	//  Find TreeNode for property->device
	int device_row = 0;
	DeviceNode* device = root.children.find_by_name_with_index(property->device, device_row);
	if (device == nullptr)
		return;

	//  Find TreeNode within device for property->group
	GroupNode* group = device->children.find_by_name(property->group);
	if (group == nullptr)
		return;

	//  Find TreeNode within group for property->name
	int row = 0;
	PropertyNode* p = group->children.find_by_name_with_index(property->name, row);
	if (p == nullptr)
		return;

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
	fprintf(stderr, "Deleting property [%s] on device [%s]\n", property->name, property->device);

	//  Find TreeNode for property->device
	int device_row = 0;
	DeviceNode* device = root.children.find_by_name_with_index(property->device, device_row);
	if (device == nullptr) {
		fprintf(stderr, "Deleting property on device [%s] - NOT FOUND\n", property->device);
		delete property;
		property = nullptr;
		return;
	}

	//  If we are deleting whole device - do that
	if (strlen(property->group) == 0) {
		beginRemoveRows(QModelIndex(), device_row, device_row);
		root.children.remove_index(device_row);
		endRemoveRows();
		delete device;
		device = nullptr;
		delete property;
		property = nullptr;
		return;
	}

	//  Find TreeNode within device for property->group
	fprintf(stderr, "Deleting property in group [%s]\n", property->group);
	int group_row = 0;
	GroupNode* group = device->children.find_by_name_with_index(property->group, group_row);
	if (group == nullptr) {
		fprintf(stderr, "Deleting property in group [%s] - NOT FOUND\n", property->group);
		delete property;
		property = nullptr;
		return;
	}

	//  If we are deleting whole group - do that
	if (strlen(property->name) == 0) {
		beginRemoveRows(createIndex(device_row, 0, device), group_row, group_row);
		device->children.remove_index(group_row);
		endRemoveRows();
		delete group;
		group = nullptr;
		delete property;
		property = nullptr;
		return;
	}

	//  Find or create TreeNode within group fro property->name
	fprintf(stderr, "Deleting property [%s]\n", property->name);
	int property_row = 0;
	PropertyNode* p = group->children.find_by_name_with_index(property->name, property_row);
	if (p == nullptr) {
		fprintf(stderr, "Deleting property [%s] - NOT FOUND\n", property->name);
		delete property;
		property = nullptr;
		return;
	}

	//  Remove the property
	beginRemoveRows(createIndex(group_row, 0, group), property_row, property_row);
	group->children.remove_index(property_row);
	endRemoveRows();
	fprintf(stderr, "Erasing property [%s]\n", property->name);
	delete p;
	p = nullptr;
	fprintf(stderr, "Erased property [%s]\n", property->name);

	//  Remove the group if empty
	if (group->children.empty()) {
		fprintf(stderr, "--- REMOVING EMPTY GROUP [%s]\n", property->group);
		beginRemoveRows(createIndex(device_row, 0, device), group_row, group_row);
		device->children.remove_index(group_row);
		endRemoveRows();
		// This causes SEGFAULT
		//delete group;
		group = nullptr;
		fprintf(stderr, "--- REMOVED EMPTY GROUP [%s]\n", property->group);
	}

	//  Remove the device if empty
	if (device->children.empty()) {
		fprintf(stderr, "--- REMOVING EMPTY DEVICE [%s]\n", property->device);
		beginRemoveRows(QModelIndex(), device_row, device_row);
		root.children.remove_index(device_row);
		endRemoveRows();
		// This causes SEGFAULT
		//delete device;
		device = nullptr;
		fprintf(stderr, "--- REMOVED EMPTY DEVICE [%s]\n", property->device);
	}

	//  Cleanup
	delete property;
	property = nullptr;
}



QModelIndex PropertyModel::index(int row, int column, const QModelIndex &parent) const {
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	if (!parent.isValid()) {
		//fprintf(stderr, "INDEX of node type %d returning %d,%d\n", TREE_NODE_ROOT, row, column);
		DeviceNode* n = root.children[row];
		return createIndex(row,column,n);
	}
	else {
		TreeNode* n = reinterpret_cast<TreeNode*>(parent.internalPointer());
		//fprintf(stderr, "->INDEX of node type %d returning %d,%d\n", n->node_type, row, column);
		n = (*n)[row];
		//fprintf(stderr, "  Yields row of node type %d\n", n->node_type);
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
		fprintf(stderr, "node = %p\n", node);
		return QModelIndex();
	}

	if (node->node_type == TREE_NODE_DEVICE)
		return QModelIndex();

	if (!node->parent())
		return QModelIndex();

	int row = node->parent()->index_of(node);
	//fprintf(stderr, "  Yields row %d of parent of node type %d\n", row, node->parent()->node_type);
	return createIndex(row,0, node->parent());
}


int PropertyModel::rowCount(const QModelIndex &parent) const {
	if (!parent.isValid())
		return root.size();
	TreeNode* node = static_cast<TreeNode*>(parent.internalPointer());
	//fprintf(stderr, "ROWS for node type %d -> %d\n", node->node_type, node->size());
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
			//fprintf(stderr, "DATA for node type %d [%s]\n", node->node_type, node->label());
			return QString::fromStdString(node->label());
		case Qt::DecorationRole: {
			if (node->node_type == TREE_NODE_PROPERTY) {
				PropertyNode* p = reinterpret_cast<PropertyNode*>(node);
				switch (p->property->state) {
				case INDIGO_IDLE_STATE:
					return QPixmap(":led-grey.png");
				case INDIGO_BUSY_STATE:
					return QPixmap(":led-orange.png");
				case INDIGO_ALERT_STATE:
					return QPixmap(":led-red.png");
				case INDIGO_OK_STATE:
					return QPixmap(":led-green.png");
				}
			} else if (node->node_type == TREE_NODE_DEVICE) {
				DeviceNode* d = reinterpret_cast<DeviceNode*>(node);
				switch (d->state) {
				case INDIGO_IDLE_STATE:
					return QPixmap(":led-grey.png");
				case INDIGO_BUSY_STATE:
					return QPixmap(":led-orange.png");
				case INDIGO_ALERT_STATE:
					return QPixmap(":led-red.png");
				case INDIGO_OK_STATE:
					return QPixmap(":led-green.png");
			}
			} else {
				return QVariant();
			}
		}
		}
	} else {
		fprintf(stderr, "Missing pointer\n");
	}
	return QVariant();
}
