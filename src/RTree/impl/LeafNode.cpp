#include "../Rtree.h"
#include <algorithm>

namespace RTree
{

    LeafNode::LeafNode(uint32_t capacity, const SplitStrategy *splitStrategy)
        : m_capacity(capacity), m_mbr(0)
    {
        // Initialize MBR as an invalid region with dimension
        m_splitStrategy = splitStrategy;
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

        // Use specified split strategy or default binary split
        if (m_splitStrategy)
        {
            std::vector<Data *> group1;
            std::vector<Data *> group2;
            std::tie(group1, group2) = m_splitStrategy->splitLeafEntries(m_entries, m_capacity);
            LeafNode *newNode = new LeafNode(m_capacity, m_splitStrategy);

            std::vector<Data *> originalEntries = std::move(m_entries);
            m_entries.clear();
            for (auto *entry : group1)
            {
                m_entries.push_back(entry);
            }
            for (auto *entry : group2)
            {
                newNode->insert(entry);
            }
            recalculateMBR();
            newNode->recalculateMBR();

            return {this, newNode};
        }
        else
        {
            // Use simple binary split strategy
            LeafNode *newNode = new LeafNode(m_capacity);
            size_t middle = m_entries.size() / 2;

            // Copy the second half of data to the new node
            for (size_t i = middle; i < m_entries.size(); ++i)
            {
                Data *dataCopy = m_entries[i]->clone();
                newNode->insert(dataCopy);
            }

            // Delete the copied data from the original node
            for (size_t i = m_entries.size() - 1; i >= middle; --i)
            {
                delete m_entries[i];
                m_entries.pop_back();
            }

            // Recalculate MBR
            recalculateMBR();
            newNode->recalculateMBR();
            return {this, newNode};
        }
    }

    void LeafNode::recalculateMBR()
    {
        if (m_entries.empty())
        {
            m_mbr = Region(0); // Create empty region
            return;
        }

        // Use the region of the first data as initial value
        m_mbr = m_entries[0]->getRegion();

        // Combine regions of all other data
        for (size_t i = 1; i < m_entries.size(); ++i)
        {
            m_mbr.combine(m_entries[i]->getRegion());
        }
    }

    uint32_t LeafNode::getHeight() const
    {
        return 1; // Leaf node's height is always 1
    }

} // namespace RTree