#ifndef RTREE_NODE_H
#define RTREE_NODE_H

#include <vector>
#include "Entry.h"

namespace RTree
{

    // 前向声明
    class RTree;

    // 节点基类
    class Node
    {
    protected:
        bool m_isLeaf;                // 是否是叶子节点
        size_t m_level;               // 树中的层级 (0为叶子)
        std::vector<Entry> m_entries; // 条目列表
        Region m_nodeMBR;             // 节点的MBR
        Node *m_parent;               // 父节点指针
        RTree *m_tree;                // 所属树的指针

    public:
        Node(bool isLeaf, size_t level, RTree *tree)
            : m_isLeaf(isLeaf), m_level(level), m_parent(nullptr), m_tree(tree) {}
        virtual ~Node() {}

        bool isLeaf() const { return m_isLeaf; }
        size_t getLevel() const { return m_level; }
        const Region &getMBR() const { return m_nodeMBR; }
        size_t getEntryCount() const { return m_entries.size(); }
        void setTree(RTree *tree) { m_tree = tree; }
        RTree *getTree() const { return m_tree; }

        virtual void insertEntry(const Entry &entry);
        void updateMBR();
        const Entry &getEntry(size_t index) const { return m_entries[index]; }
        Entry &getEntryRef(size_t index) { return m_entries[index]; }

        Node *getParent() const { return m_parent; }
        void setParent(Node *parent) { m_parent = parent; }

        virtual bool isOverflow(size_t maxEntries) const { return m_entries.size() > maxEntries; }
        virtual bool isUnderflow(size_t minEntries) const { return m_entries.size() < minEntries; }

        virtual Node *chooseSubtree(const Region &mbr) = 0;
        virtual Node *findLeaf(id_type id, const Region &mbr);

        int findEntry(id_type id) const;
        void removeEntry(size_t index);

        virtual void split(const Entry &newEntry, Node *&newNode, size_t maxEntries) = 0;
    };

    // 叶子节点
    class LeafNode : public Node
    {
    public:
        LeafNode(RTree *tree = nullptr) : Node(true, 0, tree) {}
        ~LeafNode() override;

        void insertData(void *data, size_t dataSize, const Region &mbr, id_type id);
        void split(const Entry &newEntry, Node *&newNode, size_t maxEntries) override;
        Node *findLeaf(id_type id, const Region &mbr) override;
        Node *chooseSubtree(const Region &mbr) override { return this; }
        std::vector<void *> search(const Region &query) const;
    };

    // 内部节点
    class InternalNode : public Node
    {
    public:
        InternalNode(size_t level, RTree *tree = nullptr) : Node(false, level, tree) {}
        ~InternalNode() override;

        Node *getChild(size_t index) const { return m_entries[index].m_childNode; }
        Node *chooseSubtree(const Region &mbr) override;
        void split(const Entry &newEntry, Node *&newNode, size_t maxEntries) override;
        Node *findLeaf(id_type id, const Region &mbr) override;
        void addChild(Node *child, const Region &mbr, id_type id);
    };

} // namespace RTree

#endif // RTREE_NODE_H