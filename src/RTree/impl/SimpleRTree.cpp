#include "../Rtree.h"
#include <algorithm>

namespace RTree
{

    RTree::RTree(uint32_t dimension, uint32_t nodeCapacity)
        : m_dimension(dimension), m_nodeCapacity(nodeCapacity), m_nextId(1)
    {
        // 创建根节点(开始为叶子节点)
        m_root = new LeafNode(nodeCapacity);
    }

    RTree::~RTree()
    {
        delete m_root;
    }

    void RTree::insert(uint32_t dataLength, const uint8_t *pData, const Region &mbr, id_type id)
    {
        // 创建数据对象
        Data *data = new Data(dataLength, pData, mbr, id);

        // 插入数据
        insertData_impl(data);

        // 下一个可用ID
        m_nextId = std::max(m_nextId, id + 1);
    }

    bool RTree::remove(const Region &mbr, id_type id)
    {
        return m_root->remove(id, mbr);
    }

    std::vector<Data *> RTree::intersectionQuery(const Region &query)
    {
        return m_root->search(query);
    }

    std::vector<Data *> RTree::containmentQuery(const Region &query)
    {
        std::vector<Data *> intersectedResults = m_root->search(query);
        std::vector<Data *> containedResults;

        // 过滤出完全被包含的结果
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
        // 为点查询创建一个极小的区域
        double coords[m_dimension * 2];
        for (uint32_t i = 0; i < m_dimension; ++i)
        {
            coords[i] = point.getCoordinate(i);
            coords[i + m_dimension] = point.getCoordinate(i);
        }

        Region pointRegion(coords, coords + m_dimension, m_dimension);
        std::vector<Data *> intersectedResults = m_root->search(pointRegion);
        std::vector<Data *> pointResults;

        // 过滤出包含该点的结果
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

    void RTree::insertData_impl(Data *data)
    {
        // 将数据插入到根节点
        m_root->insert(data);

        // 如果根节点分裂，需要创建新的根节点
        if (m_root->shouldSplit())
        {
            auto [original, newNode] = m_root->split();

            if (newNode)
            {
                // 创建新的内部节点作为根
                InternalNode *newRoot = new InternalNode(m_nodeCapacity);
                newRoot->addChild(original);
                newRoot->addChild(newNode);

                m_root = newRoot;
            }
        }
    }

    Node *RTree::findLeaf(Node *node, const Region &mbr, id_type id)
    {
        if (node->isLeaf())
        {
            LeafNode *leaf = static_cast<LeafNode *>(node);

            // 检查是否有匹配的数据
            for (Data *data : leaf->m_entries)
            {
                if (data->getIdentifier() == id)
                {
                    return leaf;
                }
            }

            return nullptr;
        }

        // 对于内部节点，递归搜索子节点
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
        // 如果没有新节点，只需要更新MBR
        if (newNode == nullptr)
        {
            // 如果是根节点，不需要再调整
            if (node == m_root)
            {
                return;
            }

            // 递归向上调整父节点
            // 这里简化处理，直接重新计算节点MBR即可
            // 在实际实现中，需要更新父节点中对应entry的MBR
            return;
        }

        // 如果有新节点，需要处理分裂的情况

        // 如果分裂的是根节点，创建新的根
        if (node == m_root)
        {
            InternalNode *newRoot = new InternalNode(m_nodeCapacity);
            newRoot->addChild(node);
            newRoot->addChild(newNode);
            m_root = newRoot;
            return;
        }

        // 否则，将新节点添加到父节点，并检查父节点是否需要分裂
        // 这里简化处理，因为我们的实现中，节点分裂时已经递归处理了
        // 在实际实现中，需要找到父节点，更新对应entry的MBR，并添加新的entry
    }

} // namespace RTree