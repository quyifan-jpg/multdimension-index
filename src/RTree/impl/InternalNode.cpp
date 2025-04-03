#include "../Rtree.h"
#include <algorithm>
#include <limits>

namespace RTree
{

    InternalNode::InternalNode(uint32_t capacity) : m_capacity(capacity), m_mbr(0)
    {
        // 初始化MBR为无效区域
    }

    InternalNode::~InternalNode()
    {
        for (Node *child : m_children)
        {
            delete child;
        }
    }

    bool InternalNode::isLeaf() const
    {
        return false;
    }

    const Region &InternalNode::getMBR() const
    {
        return m_mbr;
    }

    void InternalNode::insert(Data *data)
    {
        // 选择最佳子树
        Node *child = chooseSubtree(data->getRegion());

        // 插入数据
        child->insert(data);

        // 检查是否需要分裂
        if (child->shouldSplit())
        {
            auto [original, newChild] = child->split();
            if (newChild)
            {
                addChild(newChild);
            }
        }

        // 更新MBR
        recalculateMBR();
    }

    bool InternalNode::remove(id_type id, const Region &mbr)
    {
        // 找到所有可能包含该数据的子节点
        bool found = false;
        for (Node *child : m_children)
        {
            if (child->getMBR().intersects(mbr))
            {
                if (child->remove(id, mbr))
                {
                    found = true;
                    break; // 找到并删除了，就不需要继续查找
                }
            }
        }

        if (found)
        {
            // 删除了数据，需要更新MBR
            recalculateMBR();

            // 检查是否有空子节点可以删除
            auto it = std::remove_if(m_children.begin(), m_children.end(),
                                     [](Node *child)
                                     {
                                         if (child->isLeaf() ? static_cast<LeafNode *>(child)->m_entries.empty() : static_cast<InternalNode *>(child)->m_children.empty())
                                         {
                                             delete child;
                                             return true;
                                         }
                                         return false;
                                     });

            m_children.erase(it, m_children.end());
        }

        return found;
    }

    std::vector<Data *> InternalNode::search(const Region &query)
    {
        std::vector<Data *> results;

        // 搜索所有与查询区域相交的子节点
        for (Node *child : m_children)
        {
            if (child->getMBR().intersects(query))
            {
                std::vector<Data *> childResults = child->search(query);
                results.insert(results.end(), childResults.begin(), childResults.end());
            }
        }

        return results;
    }

    bool InternalNode::shouldSplit() const
    {
        return m_children.size() > m_capacity;
    }

    std::pair<Node *, Node *> InternalNode::split()
    {
        if (m_children.size() <= 2)
        {
            return {this, nullptr};
        }

        // 使用简单的二分分裂策略
        InternalNode *newNode = new InternalNode(m_capacity);
        size_t middle = m_children.size() / 2;

        // 将后半部分子节点移动到新节点
        for (size_t i = middle; i < m_children.size(); ++i)
        {
            newNode->addChild(m_children[i]);
        }

        // 删除原节点已移动的子节点(但不删除Node对象，已转移所有权)
        m_children.resize(middle);

        // 重新计算MBR
        recalculateMBR();
        newNode->recalculateMBR();

        return {this, newNode};
    }

    void InternalNode::addChild(Node *child)
    {
        m_children.push_back(child);
        recalculateMBR();
    }

    void InternalNode::recalculateMBR()
    {
        if (m_children.empty())
        {
            m_mbr = Region(0); // 创建空区域
            return;
        }

        // 以第一个子节点的MBR为初始值
        m_mbr = m_children[0]->getMBR();

        // 合并其他所有子节点的MBR
        for (size_t i = 1; i < m_children.size(); ++i)
        {
            m_mbr.combine(m_children[i]->getMBR());
        }
    }

    Node *InternalNode::chooseSubtree(const Region &mbr) const
    {
        if (m_children.empty())
        {
            return nullptr;
        }

        // 如果子节点是叶子，选择扩展最小的节点

        auto bestChild = m_children[0];
        if (bestChild->isLeaf())
        {
            double minEnlargement = std::numeric_limits<double>::max();
            Node *bestChild = nullptr;

            for (Node *child : m_children)
            {
                // 计算合并后的区域面积增加量
                Region combined = child->getMBR();
                double originalArea = combined.getArea();
                combined.combine(mbr);
                double enlargement = combined.getArea() - originalArea;

                if (bestChild == nullptr || enlargement < minEnlargement)
                {
                    minEnlargement = enlargement;
                    bestChild = child;
                }
                else if (enlargement == minEnlargement)
                {
                    // 如果扩展相同，选择面积较小的
                    if (child->getMBR().getArea() < bestChild->getMBR().getArea())
                    {
                        bestChild = child;
                    }
                }
            }

            return bestChild;
        }
        else
        {
            // 子节点是内部节点，递归选择
            double minEnlargement = std::numeric_limits<double>::max();
            Node *bestChild = nullptr;

            for (Node *child : m_children)
            {
                // 计算合并后的区域面积增加量
                Region combined = child->getMBR();
                double originalArea = combined.getArea();
                combined.combine(mbr);
                double enlargement = combined.getArea() - originalArea;

                if (bestChild == nullptr || enlargement < minEnlargement)
                {
                    minEnlargement = enlargement;
                    bestChild = child;
                }
                else if (enlargement == minEnlargement)
                {
                    // 如果扩展相同，选择面积较小的
                    if (child->getMBR().getArea() < bestChild->getMBR().getArea())
                    {
                        bestChild = child;
                    }
                }
            }
            Node *result = bestChild->chooseSubtree(mbr); // 递归进入选择的子树
            return result;
        }
    }

    uint32_t InternalNode::getHeight() const
    {
        if (m_children.empty())
        {
            return 1; // 空内部节点也有高度1
        }

        // 内部节点的高度是其子节点中最大高度加1
        uint32_t maxChildHeight = 0;
        for (const Node *child : m_children)
        {
            maxChildHeight = std::max(maxChildHeight, child->getHeight());
        }

        return maxChildHeight + 1;
    }

} // namespace RTree