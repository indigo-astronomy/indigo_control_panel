#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include "propertymodel.h"

TreeNode::~TreeNode()
{
    //fprintf(stderr, "~TreeNode: Releasing children %d\n", node_type);
    for (TreeIterator i = children.begin(); i != children.end(); i++) {
        TreeNode* n = *i;
        //fprintf(stderr, "~TreeNode: Deleting child %d\n", n->node_type);
        delete n;
        //fprintf(stderr, "~TreeNode: Deleted child\n");
    }
    children.clear();
    //fprintf(stderr, "~TreeNode: Released children\n");
}

PropertyNode::~PropertyNode()
{
//fprintf(stderr, "~PropertyNode: Releasing property\n");
    indigo_release_property(property);
//fprintf(stderr, "~PropertyNode: Released property\n");
}

ItemNode::~ItemNode()
{
    //fprintf(stderr, "~ItemNode: Releasing item\n");
}


ItemNode::ItemNode(indigo_item* i, PropertyNode* parent)
    : TreeNode(TREE_NODE_ITEM, parent), item(i)
{
    input_control = nullptr;
}

void
ItemNode::checkbox_clicked(bool checked)
{
    fprintf(stderr, "CHECK BOX TOGGLE  %d [%s]\n", checked, item->label);

    //  Update the switch item
    PropertyNode* p = reinterpret_cast<PropertyNode*>(TreeNode::parent);
    indigo_set_switch(p->property, item, checked);

    fprintf(stderr, "  update GUI controls\n");
    //  Make the GUI controls consistent with the switches
    for (TreeIterator i = p->children.begin(); i != p->children.end(); i++) {
        ItemNode* x = reinterpret_cast<ItemNode*>(*i);
        if (x->input_control) {
            QCheckBox* cb = reinterpret_cast<QCheckBox*>(x->input_control);
            cb->setCheckState(x->item->sw.value ? Qt::Checked : Qt::Unchecked);
        }
    }
    fprintf(stderr, "  update indigo bus property\n");

    //  Update the property on the bus
    indigo_change_property(nullptr, p->property);
    fprintf(stderr, "  done\n");
}



PropertyModel::PropertyModel()
    : root(TREE_NODE_ROOT, nullptr)
{
}

void
PropertyModel::define_property(indigo_property* property)
{
    //  Find or create TreeNode for property->device
    int device_row = 0;
    DeviceNode* device = nullptr;
    for (TreeIterator i = root.children.begin(); i != root.children.end(); i++) {
        if ((*i)->label() == property->device) {
            device = reinterpret_cast<DeviceNode*>(*i);
            break;
        }
        device_row++;
    }
    if (device == nullptr) {
        //  Insert the device in alphabetical order
        device = new DeviceNode(property->device, &root);
        device_row = 0;
        bool inserted = false;
        for (TreeIterator i = root.children.begin(); i != root.children.end(); i++) {
            if ((*i)->label() > property->device) {
                beginInsertRows(QModelIndex(), device_row, device_row);
                root.children.insert(i, device);
                endInsertRows();
                inserted = true;
                break;
            }
            device_row++;
        }
        if (!inserted) {
            beginInsertRows(QModelIndex(), device_row, device_row);
            root.children.push_back(device);
            endInsertRows();
        }
    }

    //  Find or create TreeNode within device for property->group
    int group_row = 0;
    GroupNode* group = nullptr;
    for (TreeIterator i = device->children.begin(); i != device->children.end(); i++) {
        if ((*i)->label() == property->group) {
            group = reinterpret_cast<GroupNode*>(*i);
            break;
        }
        group_row++;
    }
    if (group == nullptr) {
        //  Insert the group in alphabetical order
        group = new GroupNode(property->group, device);
        group_row = 0;
        bool inserted = false;
        for (TreeIterator i = device->children.begin(); i != device->children.end(); i++) {
            if ((*i)->label() > property->group) {
                beginInsertRows(createIndex(device_row, 0, device), group_row, group_row);
                device->children.insert(i, group);
                endInsertRows();
                inserted = true;
                break;
            }
            group_row++;
        }
        if (!inserted) {
            beginInsertRows(createIndex(device_row, 0, device), group_row, group_row);
            device->children.push_back(group);
            endInsertRows();
        }
    }

    //  Find or create TreeNode within group for property->name
    int property_row = 0;
    PropertyNode* p = nullptr;
    for (TreeIterator i = group->children.begin(); i != group->children.end(); i++) {
        if ((*i)->label() == property->name) {
            p = reinterpret_cast<PropertyNode*>(*i);
            break;
        }
        property_row++;
    }
    if (p == nullptr) {
        //  Insert the property in alphabetical order
        p = new PropertyNode(property, group);
        property_row = 0;
        bool inserted = false;
        for (TreeIterator i = group->children.begin(); i != group->children.end(); i++) {
            if ((*i)->label() > property->label) {
                beginInsertRows(createIndex(group_row, 0, group), property_row, property_row);
                group->children.insert(i, p);
                endInsertRows();
                inserted = true;
                break;
            }
            group_row++;
        }
        if (!inserted) {
            beginInsertRows(createIndex(group_row, 0, group), property_row, property_row);
            group->children.push_back(p);
            endInsertRows();
        }

        //  Create the ItemNodes
        for (int i = 0; i < property->count; i++) {
            ItemNode* item = new ItemNode(&property->items[i], p);
            p->children.push_back(item);
        }
    }
}

void
PropertyModel::update_property(indigo_property* property)
{
    //  Find TreeNode for property->device
    TreeNode* device = nullptr;
    for (TreeIterator i = root.children.begin(); i != root.children.end(); i++) {
        if ((*i)->label() == property->device) {
            device = *i;
            break;
        }
    }
    if (device == nullptr) {
        return;
    }

    //  Find TreeNode within device for property->group
    TreeNode* group = nullptr;
    for (TreeIterator i = device->children.begin(); i != device->children.end(); i++) {
        if ((*i)->label() == property->group) {
            group = *i;
            break;
        }
    }
    if (group == nullptr) {
        return;
    }

    //  Find or create TreeNode within group fro property->name
    int row = 0;
    PropertyNode* p = nullptr;
    for (TreeIterator i = group->children.begin(); i != group->children.end(); i++) {
        if (strcmp(reinterpret_cast<PropertyNode*>(*i)->property->name, property->name) == 0) {
            p = reinterpret_cast<PropertyNode*>(*i);
            break;
        }
        row++;
    }
    if (p == nullptr) {
        fprintf(stderr, "   Didn't find property\n");
        return;
    }

    //  Update property
    indigo_property_copy_values(p->property, property, true);

    //  Update the ItemNodes so the controls reflect the underlying data
fprintf(stderr, "UPDATING [%s]\n", p->label().c_str());
    char buffer[50];
    QLineEdit* e;
    QCheckBox* cb;
    for (TreeIterator i = p->children.begin(); i != p->children.end(); i++) {
        ItemNode* item = reinterpret_cast<ItemNode*>(*i);
        if (item->input_control != nullptr) {
            switch (p->property->type) {
            case INDIGO_TEXT_VECTOR:
                e = reinterpret_cast<QLineEdit*>(item->input_control);
                e->setText(item->item->text.value);
                break;
            case INDIGO_NUMBER_VECTOR:
                sprintf(buffer, item->item->number.format, item->item->number.value);
                e = reinterpret_cast<QLineEdit*>(item->input_control);
                e->setText(buffer);
                break;
            case INDIGO_SWITCH_VECTOR:
                cb = reinterpret_cast<QCheckBox*>(item->input_control);
                cb->setCheckState(item->item->sw.value ? Qt::Checked : Qt::Unchecked);
                break;
            default:
                break;
            }
        }
    }

    //  Emit a data changed signal so the tree can update (mainly for status LEDs)
    QModelIndex index = createIndex(row, 0, p);
    emit(dataChanged(index, index));
}

void
PropertyModel::delete_property(indigo_property* property)
{
    fprintf(stderr, "Deleting property [%s] on device [%s]\n", property->name, property->device);

    //  Find TreeNode for property->device
    int device_row = 0;
    TreeNode* device = nullptr;
    for (TreeIterator i = root.children.begin(); i != root.children.end(); i++) {
        if ((*i)->label() == property->device) {
            device = *i;

            //  If we are deleting whole device - do that
            if (strlen(property->group) == 0) {
                beginRemoveRows(QModelIndex(), device_row, device_row);
                root.children.erase(i);
                endRemoveRows();
                delete device;
                device = nullptr;
            }
            break;
        }
        device_row++;
    }
    if (device == nullptr) {
        fprintf(stderr, "Deleting property on device [%s] - NOT FOUND\n", property->device);
        delete property;
        return;
    }

    //  Find TreeNode within device for property->group
    fprintf(stderr, "Deleting property in group [%s]\n", property->group);
    int group_row = 0;
    TreeNode* group = nullptr;
    TreeIterator gi;
    for (gi = device->children.begin(); gi != device->children.end(); gi++) {
        if ((*gi)->label() == property->group) {
            group = *gi;

            //  If we are deleting whole group - do that
            if (strlen(property->name) == 0) {
                beginRemoveRows(createIndex(device_row, 0, device), group_row, group_row);
                device->children.erase(gi);
                endRemoveRows();
                delete group;
                group = nullptr;
            }
            break;
        }
        group_row++;
    }
    if (group == nullptr) {
        fprintf(stderr, "Deleting property in group [%s] - NOT FOUND\n", property->group);
        delete property;
        return;
    }

    //  Find or create TreeNode within group fro property->name
    fprintf(stderr, "Deleting property [%s]\n", property->name);
    int property_row = 0;
    PropertyNode* p = nullptr;
    for (TreeIterator i = group->children.begin(); i != group->children.end(); i++) {
        if (strcmp(reinterpret_cast<PropertyNode*>(*i)->property->name, property->name) == 0) {
            p = reinterpret_cast<PropertyNode*>(*i);

            //  Remove the property
            beginRemoveRows(createIndex(group_row, 0, group), property_row, property_row);
            group->children.erase(i);
            fprintf(stderr, "Erasing property [%s]\n", property->name);
            delete p;
            endRemoveRows();

            fprintf(stderr, "Erased property [%s]\n", property->name);

            //  Remove the group if empty
            if (false || group->children.empty()) {
            fprintf(stderr, "--- REMOVING EMOTY GROUP [%s]\n", property->group);
                beginRemoveRows(createIndex(device_row, 0, device), group_row, group_row);
                device->children.erase(gi);
                delete group;
                endRemoveRows();
                group = nullptr;
                fprintf(stderr, "--- REMOVED EMOTY GROUP [%s]\n", property->group);
            }

            break;
        }
        property_row++;
    }
    delete property;
}



QModelIndex
PropertyModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    const TreeNode* node = &root;
    if (parent.isValid())
        node = static_cast<TreeNode*>(parent.internalPointer());
    TreeNode* n = node->children.at(static_cast<std::vector<TreeNode*>::size_type>(row));
    return createIndex(row,column,n);
}

QModelIndex
PropertyModel::parent(const QModelIndex &child) const
{
    if(!child.isValid())
        return QModelIndex();
    TreeNode* node = static_cast<TreeNode*>(child.internalPointer());
    if (node->parent == &root)
        return QModelIndex();
    else {
        //  Search for child in the parent's child list
        int row = 0;
        for (std::vector<TreeNode*>::iterator i = node->parent->children.begin(); i != node->parent->children.end(); i++) {
            if (*i == node)
                break;
            row++;
        }
        return createIndex(row,0, node->parent);
    }
}

int
PropertyModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return static_cast<int>(root.children.size());
    TreeNode* node = static_cast<TreeNode*>(parent.internalPointer());
    if (node->node_type == TREE_NODE_PROPERTY)
        return 0;
    else
        return static_cast<int>(node->children.size());
}

int
PropertyModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QVariant
PropertyModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();
  TreeNode* node = static_cast<TreeNode*>(index.internalPointer());
  if (node) {
    switch (role) {
    case Qt::DisplayRole:
      return QString::fromStdString(node->label());
    case Qt::DecorationRole:
      {
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
        }
        else if (node->node_type == TREE_NODE_DEVICE) {
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
        }
        else {
            return QVariant();
        }
      }
    }
  }
  else {
      fprintf(stderr, "Missing pointer\n");
  }
  return QVariant();
}

inline bool name_less_than(TreeNode*& n1, TreeNode*& n2) {
    return n1->label() < n2->label();
}

inline bool name_greater_than(TreeNode*& n1, TreeNode*& n2) {
    return n1->label() > n2->label();
}

void
PropertyModel::sort(int, Qt::SortOrder order)
{
    beginResetModel();

    //  Sort the devices
    if (order == Qt::AscendingOrder)
        std::sort(root.children.begin(), root.children.end(), name_less_than);
    else
        std::sort(root.children.begin(), root.children.end(), name_greater_than);

    //  Sort the groups within each device

    //  Sort the properties within each group

    endResetModel();
}
