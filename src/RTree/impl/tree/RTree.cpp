#include "Rtree.h"
#include <algorithm>

#include "src/RTree/impl/node/InternalNode.h"
#include "src/RTree/impl/node/LeafNode.h"
#include "src/RTree/impl/strategy/LinearSplitStrategy.h"

namespace RTree
{
    // Create global static LinearSplitStrategy instance
    static const LinearSplitStrategy s_defaultSplitStrategy;
    RTree::RTree(uint32_t dimension, uint32_t nodeCapacity, const SplitStrategy *splitStrategy)
        : m_dimension(dimension), m_nodeCapacity(nodeCapacity), m_nextId(1)
    {
        if (splitStrategy)
        {
            setSplitStrategy(splitStrategy);
        }
        else
        {
            setSplitStrategy(&s_defaultSplitStrategy);
        }
        m_root_node = new LeafNode(nodeCapacity, m_splitStrategy);
    }

    RTree::~RTree()
    {
        delete m_root_node;
    }

    void RTree::insert(uint32_t dataLength, const uint8_t *pData, const Region &mbr, id_type id)
    {
        // Create data object
        Data *data = new Data(dataLength, pData, mbr, id);
        // Insert data
        insertData_impl(data);

        // Next available ID
        m_nextId = std::max(m_nextId, id + 1);
    }

    bool RTree::remove(const Region &mbr, id_type id)
    {
        return m_root_node->remove(id, mbr);
    }

    std::vector<Data *> RTree::intersectionQuery(const Region &query)
    {
        return m_root_node->search(query);
    }

    std::vector<Data *> RTree::containmentQuery(const Region &query)
    {
        std::vector<Data *> intersectedResults = m_root_node->search(query);
        std::vector<Data *> containedResults;

        // Filter out results that are fully contained
        for (Data *data : intersectedResults)
        {
            if (query.contains(data->getRegion()))
            {
                containedResults.push_back(data);
            }
        }

        return containedResults;
    }

    std::vector<Data *> RTree::pointQuery(const Point &point)
    {
        // Create a tiny region for point query
        double coords[m_dimension * 2];
        for (uint32_t i = 0; i < m_dimension; ++i)
        {
            coords[i] = point.getCoordinate(i);
            coords[i + m_dimension] = point.getCoordinate(i);
        }

        Region pointRegion(coords, coords + m_dimension, m_dimension);
        std::vector<Data *> intersectedResults = m_root_node->search(pointRegion);
        std::vector<Data *> pointResults;

        // Filter out results that contain the point
        for (Data *data : intersectedResults)
        {
            if (data->getRegion().contains(point))
            {
                pointResults.push_back(data);
            }
        }

        return pointResults;
    }

    uint32_t RTree::getDimension() const
    {
        return m_dimension;
    }

    uint32_t RTree::getNodeCapacity() const
    {
        return m_nodeCapacity;
    }

    uint32_t RTree::getHeight() const
    {
        if (!m_root_node)
        {
            return 0; // Empty tree has height 0
        }

        return m_root_node->getHeight();
    }

    void RTree::setSplitStrategy(const SplitStrategy *splitStrategy)
    {
        // If no split strategy is provided, continue using the default LinearSplitStrategy
        m_splitStrategy = splitStrategy ? splitStrategy : &s_defaultSplitStrategy;
    }

    void RTree::insertData_impl(Data *data)
    {
        // Insert data into the root node
        m_root_node->insert(data);

        // If the root node splits, need to create a new root node
        if (m_root_node->shouldSplit())
        {
            auto [original, newNode] = m_root_node->split();

            if (newNode)
            {
                // Create a new internal node as root
                InternalNode *newRoot = new InternalNode(m_nodeCapacity, m_splitStrategy);
                newRoot->addChild(original);
                newRoot->addChild(newNode);

                m_root_node = newRoot;
            }
        }
    }

    Node *RTree::findLeaf(Node *node, const Region &mbr, id_type id)
    {
        if (node->isLeaf())
        {
            LeafNode *leaf = static_cast<LeafNode *>(node);

            // Check if there's matching data
            for (Data *data : leaf->m_entries)
            {
                if (data->getIdentifier() == id)
                {
                    return leaf;
                }
            }

            return nullptr;
        }

        // For internal nodes, recursively search child nodes
        InternalNode *internalNode = static_cast<InternalNode *>(node);
        for (Node *child : internalNode->m_children)
        {
            if (child->getMBR().intersects(mbr))
            {
                Node *result = findLeaf(child, mbr, id);
                if (result)
                {
                    return result;
                }
            }
        }

        return nullptr;
    }

    void RTree::adjustTree(Node *node, Node *newNode)
    {
        // If no new node, just update the MBR
        if (newNode == nullptr)
        {
            // If it's the root node, no need to adjust further
            if (node == m_root_node)
            {
                return;
            }

            // Recursively adjust parent nodes
            // Simplified handling, just recalculate node MBR
            // In an actual implementation, need to update the MBR of the corresponding entry in the parent node
            return;
        }

        // If there's a new node, need to handle the split case

        // If the split node is the root, create a new root
        if (node == m_root_node)
        {
            InternalNode *newRoot = new InternalNode(m_nodeCapacity, m_splitStrategy);
            newRoot->addChild(node);
            newRoot->addChild(newNode);
            m_root_node = newRoot;
            return;
        }

        // Otherwise, add the new node to the parent and check if the parent needs to split
        // Simplified handling, as in our implementation, node splits are handled recursively
        // In an actual implementation, need to find the parent node, update the MBR of the corresponding entry, and add a new entry
    }

} // namespace RTree