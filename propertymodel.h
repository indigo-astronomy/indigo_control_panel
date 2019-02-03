#ifndef PROPERTYMODEL_H
#define PROPERTYMODEL_H

#include <QAbstractItemModel>
#include <QLabel>
#include <indigo_bus.h>


enum TreeNodeType {
    TREE_NODE_ROOT,
    TREE_NODE_DEVICE,
    TREE_NODE_GROUP,
    TREE_NODE_PROPERTY,
    TREE_NODE_ITEM
};

struct TreeNode {
    TreeNode(/*const char* node_name, */enum TreeNodeType type, TreeNode* _parent) : /*name(node_name), state(INDIGO_IDLE_STATE),*/ parent(_parent), children(), node_type(type) {}
    virtual ~TreeNode();

    virtual std::string& label() { static std::string name; return name; }

    TreeNode* parent;
    std::vector<TreeNode*> children;
    enum TreeNodeType node_type;

//    std::string name;
//    indigo_property_state state;

//    indigo_property* property;      //  Only for node_type == TREE_NODE_PROPERTY
};

typedef std::vector<TreeNode*>::iterator TreeIterator;

struct DeviceNode : public TreeNode {

    DeviceNode(const char* device_name, TreeNode* parent) : TreeNode(TREE_NODE_DEVICE, parent), name(device_name), state(INDIGO_IDLE_STATE) {}

    virtual std::string& label() { return name; }

    std::string name;
    indigo_property_state state;   //  This could instead be a pointer to the connection property of the device
};

struct GroupNode : public TreeNode {
    GroupNode(const char* group_name, DeviceNode* parent) : TreeNode(TREE_NODE_GROUP, parent), name(group_name) {}

    virtual std::string& label() { return name; }

    std::string name;
};

struct PropertyNode : public TreeNode {
    PropertyNode(indigo_property* p, GroupNode* parent) : TreeNode(TREE_NODE_PROPERTY, parent), name(p->label), property(p) {}

    virtual std::string& label() { return name; }

    std::string name;
    indigo_property* property;      //  Only for node_type == TREE_NODE_PROPERTY
};

struct ItemNode : public TreeNode {
    ItemNode(indigo_item* i, PropertyNode* parent);

    QLabel* input_label;
    QWidget* input_control;
    indigo_item* item;
};




class PropertyModel : public QAbstractItemModel
{
public:
    PropertyModel();

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

public slots:
    void define_property(indigo_property* property);
    void update_property(indigo_property* property);
    void delete_property(indigo_property* property);

private:
    TreeNode root;
};

#endif // PROPERTYMODEL_H
