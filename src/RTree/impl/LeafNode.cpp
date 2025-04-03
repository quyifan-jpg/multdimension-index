#include "../Rtree.h"
#include <algorithm>

namespace RTree
{

    LeafNode::LeafNode(uint32_t capacity) : m_capacity(capacity), m_mbr(0)
    {
        // 初始化MBR为无效区域
    }

    LeafNode::~LeafNode()
    {
        for (Data *data : m_entries)
        {
            delete data;
        }
    }

    bool LeafNode::isLeaf() const
    {
        return true;
    }

    const Region &LeafNode::getMBR() const
    {
        return m_mbr;
    }

    void LeafNode::insert(Data *data)
    {
        m_entries.push_back(data);
        recalculateMBR();
    }

    bool LeafNode::remove(id_type id, const Region &mbr)
    {
        auto it = std::find_if(m_entries.begin(), m_entries.end(),
                               [id](const Data *data)
                               { return data->getIdentifier() == id; });

        if (it != m_entries.end())
        {
            delete *it;
            m_entries.erase(it);
            recalculateMBR();
            return true;
        }

        return false;
    }

    std::vector<Data *> LeafNode::search(const Region &query)
    {
        std::vector<Data *> results;

        for (Data *data : m_entries)
        {
            if (data->getRegion().intersects(query))
            {
                results.push_back(data);
            }
        }

        return results;
    }

    bool LeafNode::shouldSplit() const
    {
        return m_entries.size() > m_capacity;
    }

    std::pair<Node *, Node *> LeafNode::split()
    {
        if (m_entries.size() <= 2)
        {
            return {this, nullptr};
        }

        // 使用简单的二分分裂策略
        LeafNode *newNode = new LeafNode(m_capacity);
        size_t middle = m_entries.size() / 2;

        // 将后半部分数据复制到新节点
        for (size_t i = middle; i < m_entries.size(); ++i)
        {
            Data *dataCopy = m_entries[i]->clone();
            newNode->insert(dataCopy);
        }

        // 删除原节点已复制的数据
        for (size_t i = m_entries.size() - 1; i >= middle; --i)
        {
            delete m_entries[i];
            m_entries.pop_back();
        }

        // 重新计算MBR
        recalculateMBR();

        return {this, newNode};
    }

    void LeafNode::recalculateMBR()
    {
        if (m_entries.empty())
        {
            m_mbr = Region(0); // 创建空区域
            return;
        }

        // 以第一个数据的区域为初始值
        m_mbr = m_entries[0]->getRegion();

        // 合并其他所有数据的区域
        for (size_t i = 1; i < m_entries.size(); ++i)
        {
            m_mbr.combine(m_entries[i]->getRegion());
        }
    }

    uint32_t LeafNode::getHeight() const
    {
        return 1; // 叶子节点的高度始终为1
    }

} // namespace RTree