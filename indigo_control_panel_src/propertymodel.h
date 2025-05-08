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


#ifndef PROPERTYMODEL_H
#define PROPERTYMODEL_H

#include <QAbstractItemModel>
#include <QLabel>
#include <indigo/indigo_bus.h>
#include <assert.h>

enum TreeNodeType {
	TREE_NODE_ROOT,
	TREE_NODE_DEVICE,
	TREE_NODE_GROUP,
	TREE_NODE_PROPERTY,
	TREE_NODE_ITEM
};

class QIndigoProperty;

template <class T>
class OrderedList {
public:
	OrderedList() : count(0), max(1) {
		nodes = reinterpret_cast<T**>(malloc(sizeof(T*)));
	}

	~OrderedList() {
		//  Delete the contents of the array
		for (int i = 0; i < count; i++) {
			delete nodes[i];
			nodes[i] = nullptr;
		}
		//  Free up the array
		free(nodes);
		nodes = nullptr;
	}

	int size() const { return count; }
	bool empty() const { return count == 0; }
	T* operator[](int index) const { assert(index >= 0 && index < count); return nodes[index]; }

	void remove_index(int index) {
		//  Shift higher items down one
		for (int i = index; i < count-1; i++)
			nodes[i] = nodes[i+1];

		//  Reduce count
		count--;
	}

	int index_of(T* node) const {
		for (int i = 0; i < count; i++) {
			if (nodes[i] == node) return i;
		}
		return -1;
	}

	T* find_by_name(const char* n) const {
		int index;
		return find_by_name_with_index(n, index);
	}

	T* find_by_name_with_index(const char* n, int& index) const {
		if (n) {
			for (int i = 0; i < count; i++) {
				if (strcmp(nodes[i]->name(), n) == 0) {
					index = i;
					return nodes[i];
				}
			}
		}
		index = -1;
		return nullptr;
	}

	int find_insertion_index_by_label(const char* l) const {
		for (int i = 0; i < count; i++) {
			if (strcmp(nodes[i]->label(), l) > 0) return i;
		}

		//  We need to return an index where we would insert the item
		//  0 means right at the start, count means right at the end
		return count;
	}

	void insert_at(int index, T* node) {
		//  Ensure we have enough space
		if (count + 1 > max) {
			max *= 2;
			nodes = reinterpret_cast<T**>(realloc(nodes, max * sizeof(T*)));
		}

		//  Move the nodes up to make space
		for (int i = count-1; i >= index; i--)
			nodes[i+1] = nodes[i];

		//  Actually insert
		assert(index >= 0 && index <= count);
		nodes[index] = node;
		count++;
	}

	void each(void (*func)(T*)) {
		for (int i = 0; i < count; i++) func(nodes[i]);
	}

//private:
	T** nodes;
	int count;
	int max;
};

struct TreeNode {
	TreeNode(enum TreeNodeType type) : node_type(type) {}
	virtual ~TreeNode();

	virtual int size() const { return 0; }
	virtual TreeNode* parent() { return nullptr; }
	virtual int index_of(TreeNode*) const { fprintf(stderr, "index_of on node type %d\n", node_type); assert(false); }
	virtual TreeNode* operator[] (int) { fprintf(stderr, "operator[] on node type %d\n", node_type); assert(false); }

	virtual const char* name() { return "TreeNode"; }
	virtual const char* label() { return name(); }

	enum TreeNodeType node_type;
};

template <class ParentT>
struct TreeNodeWithParent : public TreeNode {
	TreeNodeWithParent(enum TreeNodeType type, ParentT* _parent) : TreeNode(type), m_parent(_parent) {}
	virtual ~TreeNodeWithParent() {}

	virtual TreeNode* parent() { return m_parent; }

	ParentT* m_parent;
};

template <class ParentT, class ChildT>
struct TreeNodeWithChildren : public TreeNodeWithParent<ParentT> {
	TreeNodeWithChildren(enum TreeNodeType type, ParentT* _parent) : TreeNodeWithParent<ParentT>(type, _parent), children() {}
	virtual ~TreeNodeWithChildren() {}

	virtual int size() const { return children.size(); }
	virtual TreeNode* operator[] (int row) { return children[row]; }

	void each_child(void (*func)(ChildT*)) { children.each(func); }

	OrderedList<ChildT> children;
};

struct RootNode;
struct DeviceNode;
struct GroupNode;
struct PropertyNode;
struct ItemNode;


struct DeviceNode : public TreeNodeWithChildren<RootNode,GroupNode> {
	DeviceNode(const char* device_name, RootNode* parent) : TreeNodeWithChildren<RootNode,GroupNode>(TREE_NODE_DEVICE, parent), state(INDIGO_IDLE_STATE) {
		strncpy(m_name, device_name, sizeof(m_name));
		m_interface = 0;
	}

	virtual ~DeviceNode();

	virtual const char* name() { return m_name; }
	virtual int index_of(TreeNode* n) const { assert(n->node_type == TREE_NODE_GROUP); return children.index_of(reinterpret_cast<GroupNode*>(n)); }

	char m_name[INDIGO_NAME_SIZE];
	int m_interface;
	indigo_property_state state;   //  This could instead be a pointer to the connection property of the device
};


struct GroupNode : public TreeNodeWithChildren<DeviceNode,PropertyNode> {
	GroupNode(const char* group_name, DeviceNode* parent) : TreeNodeWithChildren(TREE_NODE_GROUP, parent) {
		strncpy(m_name, group_name, sizeof(m_name));
		strncpy(m_device, parent->name(), sizeof(m_device));
	}

	virtual ~GroupNode();

	virtual const char* device() { return m_device; }
	virtual const char* name() { return m_name; }
	virtual int index_of(TreeNode* n) const { assert(n->node_type == TREE_NODE_PROPERTY); return children.index_of(reinterpret_cast<PropertyNode*>(n)); }

	char m_name[INDIGO_NAME_SIZE];
	char m_device[INDIGO_NAME_SIZE];
};


struct PropertyNode : public TreeNodeWithChildren<GroupNode,ItemNode> {
public:
	PropertyNode(indigo_property* p, GroupNode* parent) : TreeNodeWithChildren(TREE_NODE_PROPERTY, parent), property(p) {}
	virtual ~PropertyNode();

	virtual const char* device() { return property->device; }
	virtual const char* group() { return property->group; }
	virtual const char* name() { return property->name; }
	virtual const char* label() { return property->label; }
	virtual int size() const { return 0; }

	indigo_property* property;
};

struct ItemNode : public TreeNodeWithParent<PropertyNode> {
public:
	ItemNode(indigo_item* i, PropertyNode* parent);
	virtual ~ItemNode();

	virtual const char* name() { return item->name; }
	virtual const char* label() { return item->label; }

	QWidget* input_control;
	indigo_item* item;
};


struct RootNode : public TreeNodeWithChildren<TreeNode,DeviceNode> {
	RootNode() : TreeNodeWithChildren(TREE_NODE_ROOT, nullptr) {}
	~RootNode();

	virtual const char* label() { return "ROOT"; }
	virtual int index_of(TreeNode* n) const { assert(n->node_type == TREE_NODE_DEVICE); return children.index_of(reinterpret_cast<DeviceNode*>(n)); }
};


class PropertyModel : public QAbstractItemModel {
	Q_OBJECT

public:
	bool no_repaint_flag;
	PropertyModel();

	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

signals:
	void property_updated(indigo_property* property, char *message);
	void property_defined(indigo_property* property, char *message);
	void property_deleted(indigo_property* property, char *message);

public slots:
	void define_property(indigo_property* property, char *message);
	void update_property(indigo_property* property, char *message);
	void delete_property(indigo_property* property, char *message);
	void enable_blobs(bool on);
	void rebuild_blob_previews();

private:
	RootNode root;
};

#endif // PROPERTYMODEL_H
