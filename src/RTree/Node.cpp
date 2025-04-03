#include "Node.h"
#include "RTree.h"
#include <limits>
#include <iostream>
namespace RTree
{

    //==========================
    // Node类方法实现
    //==========================
    void Node::insertEntry(const Entry &entry)
    {
        m_entries.push_back(entry);
        updateMBR();
    }

    void Node::updateMBR()
    {
        if (m_entries.empty())
        {
            return;
        }

        m_nodeMBR = m_entries[0].m_region;
        for (size_t i = 1; i < m_entries.size(); i++)
        {
            m_nodeMBR.combineRegion(m_entries[i].m_region);
        }
    }

    Node *Node::chooseSubtree(const Region &mbr)
    {
        // 基类实现，叶子节点返回自身
        return this;
    }

    Node *Node::findLeaf(id_type id, const Region &mbr)
    {
        // 基类实现：在当前节点中查找
        int index = findEntry(id);
        if (index >= 0)
        {
            return this;
        }
        return nullptr;
    }

    int Node::findEntry(id_type id) const
    {
        for (size_t i = 0; i < m_entries.size(); i++)
        {
            if (m_entries[i].m_id == id)
            {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    void Node::removeEntry(size_t index)
    {
        if (index < m_entries.size())
        {
            m_entries.erase(m_entries.begin() + index);
            updateMBR();
        }
    }

    //==========================
    // LeafNode类方法实现
    //==========================
    LeafNode::~LeafNode()
    {
        // 不需要释放m_data，因为叶子节点不拥有数据的所有权
    }

    void LeafNode::insertData(void *data, size_t dataSize, const Region &mbr, id_type id)
    {
        Entry entry(mbr, id, data, dataSize);
        insertEntry(entry);
    }

    void LeafNode::split(const Entry &newEntry, Node *&newNode, size_t maxEntries)
    {
        // 创建新节点
        LeafNode *newLeaf = new LeafNode(m_tree);
        newNode = newLeaf;

        // 获取分裂策略
        std::shared_ptr<SplitStrategy> strategy = m_tree->getSplitStrategy();

        // 执行分裂
        std::vector<size_t> group1, group2;
        strategy->split(m_entries, newEntry, group1, group2);

        // 创建临时条目数组，包含新条目
        std::vector<Entry> allEntries = m_entries;
        allEntries.push_back(newEntry);

        // 清空当前节点条目
        m_entries.clear();

        // 将第一组条目分配到当前节点
        for (size_t idx : group1)
        {
            m_entries.push_back(allEntries[idx]);
        }

        // 将第二组条目分配到新节点
        for (size_t idx : group2)
        {
            newLeaf->m_entries.push_back(allEntries[idx]);
        }

        // 更新MBR
        updateMBR();
        newLeaf->updateMBR();

        // 设置新节点的父节点
        newLeaf->setParent(m_parent);
    }

    Node *LeafNode::findLeaf(id_type id, const Region &mbr)
    {
        // 在叶子节点中查找条目
        int index = findEntry(id);
        if (index >= 0)
        {
            return this;
        }
        return nullptr;
    }

    std::vector<void *> LeafNode::search(const Region &query) const
    {
        std::vector<void *> results;
        for (const auto &entry : m_entries)
        {
            if (entry.m_region.intersectsRegion(query))
            {
                results.push_back(entry.m_data);
            }
        }
        return results;
    }

    //==========================
    // InternalNode类方法实现
    //==========================
    InternalNode::~InternalNode()
    {
        // 内部节点拥有子节点的所有权，需要释放
        for (auto &entry : m_entries)
        {
            delete entry.m_childNode;
        }
    }

    Node *InternalNode::chooseSubtree(const Region &mbr)
    {
        // 选择扩展面积最小的子树
        double minEnlargement = std::numeric_limits<double>::max();
        double minArea = std::numeric_limits<double>::max();
        size_t chosen = 0;

        for (size_t i = 0; i < m_entries.size(); i++)
        {
            double enlargement = m_entries[i].getEnlargement(mbr);
            double area = m_entries[i].m_region.getArea();

            if (enlargement < minEnlargement ||
                (enlargement == minEnlargement && area < minArea))
            {
                minEnlargement = enlargement;
                minArea = area;
                chosen = i;
            }
        }
        Node* childNode = m_entries[chosen].m_childNode;
            if (!childNode) {
                std::cout << "Error: Null child node found!" << std::endl;
                return nullptr;
            }

        return childNode->chooseSubtree(mbr);
    }

    void InternalNode::split(const Entry &newEntry, Node *&newNode, size_t maxEntries)
    {
        // 创建新节点
        InternalNode *newInternal = new InternalNode(m_level, m_tree);
        newNode = newInternal;

        // 获取分裂策略
        std::shared_ptr<SplitStrategy> strategy = m_tree->getSplitStrategy();

        // 执行分裂
        std::vector<size_t> group1, group2;
        strategy->split(m_entries, newEntry, group1, group2);

        // 创建临时条目数组，包含新条目
        std::vector<Entry> allEntries = m_entries;
        allEntries.push_back(newEntry);

        // 清空当前节点条目
        m_entries.clear();

        // 将第一组条目分配到当前节点
        for (size_t idx : group1)
        {
            m_entries.push_back(allEntries[idx]);
            allEntries[idx].m_childNode->setParent(this);
        }

        // 将第二组条目分配到新节点
        for (size_t idx : group2)
        {
            newInternal->m_entries.push_back(allEntries[idx]);
            allEntries[idx].m_childNode->setParent(newInternal);
        }

        // 更新MBR
        updateMBR();
        newInternal->updateMBR();

        // 设置新节点的父节点
        newInternal->setParent(m_parent);
    }

    Node *InternalNode::findLeaf(id_type id, const Region &mbr)
    {
        // 首先检查当前节点的条目
        for (const auto &entry : m_entries)
        {
            if (entry.m_region.intersectsRegion(mbr))
            {
                // 递归检查子节点
                Node *result = entry.m_childNode->findLeaf(id, mbr);
                if (result)
                {
                    return result;
                }
            }
        }
        return nullptr;
    }

    void InternalNode::addChild(Node *child, const Region &mbr, id_type id)
    {
        Entry entry(mbr, id, child);
        insertEntry(entry);
        child->setParent(this);
    }

} // namespace RTree