#include "RTree.h"
#include <iostream>

namespace RTree
{

    void RTree::insert(void *data, size_t dataSize, const Region &mbr)
    {
        // 递增数据项数量
        m_size++;

        // 生成唯一ID
        id_type id = generateID();
        if (id == 20)
        {
            std::cout << "insert id: " << id << std::endl;
        }
        // 第一步：定位叶子节点
        Node *leafNode = m_root->chooseSubtree(mbr);

        // 第二步：将数据插入叶子节点
        LeafNode *leaf = static_cast<LeafNode *>(leafNode);
        leaf->insertData(data, dataSize, mbr, id);

        // 第三步：如果需要，分裂节点
        if (leaf->isOverflow(m_maxEntries))
        {
            // 创建新条目
            Entry newEntry(mbr, id, data, dataSize);

            // 分裂节点
            Node *newNode = nullptr;
            leaf->split(newEntry, newNode, m_maxEntries);

            // 调整树
            adjustTree(leaf, newNode);
        }
        else
        {
            // 没有分裂，只需调整树
            adjustTree(leaf);
        }
    }

    void RTree::adjustTree(Node *node, Node *newNode)
    {
        // 如果是根节点
        if (node == m_root.get())
        {
            if (newNode)
            {
                // 创建新的根节点
                InternalNode *newRoot = new InternalNode(m_treeHeight, this);

                // 添加旧根和新节点作为子节点
                newRoot->addChild(node, node->getMBR(), generateID());
                newRoot->addChild(newNode, newNode->getMBR(), generateID());

                // 更新树的高度
                m_treeHeight++;

                // 设置新的根节点
                m_root.reset(newRoot);
            }
            return;
        }

        // 获取父节点
        Node *parent = node->getParent();

        // 更新父节点中对应条目的MBR
        for (size_t i = 0; i < parent->getEntryCount(); i++)
        {
            Entry &entry = parent->getEntryRef(i);
            if (entry.m_childNode == node)
            {
                entry.m_region = node->getMBR();
                break;
            }
        }

        // 如果有新节点，将其添加到父节点
        if (newNode)
        {
            // 在父节点中增加新节点
            InternalNode *internalParent = static_cast<InternalNode *>(parent);
            internalParent->addChild(newNode, newNode->getMBR(), generateID());

            // 检查父节点是否溢出
            if (parent->isOverflow(m_maxEntries))
            {
                // 创建虚拟条目用于分裂
                Entry dummyEntry; // 仅用于分裂函数接口

                // 分裂父节点
                Node *newParent = nullptr;
                parent->split(dummyEntry, newParent, m_maxEntries);

                // 继续向上调整
                adjustTree(parent, newParent);
            }
            else
            {
                // 没有分裂，继续向上调整
                adjustTree(parent);
            }
        }
        else
        {
            // 更新父节点的MBR
            parent->updateMBR();

            // 继续向上调整
            adjustTree(parent);
        }
    }

    std::vector<void *> RTree::search(const Region &query) const
    {
        std::vector<void *> results;

        // 如果根节点为空，返回空结果
        if (!m_root)
        {
            return results;
        }

        // 使用栈进行深度优先搜索
        std::vector<Node *> stack;
        stack.push_back(m_root.get());

        while (!stack.empty())
        {
            Node *node = stack.back();
            stack.pop_back();

            for (size_t i = 0; i < node->getEntryCount(); i++)
            {
                const Entry &entry = node->getEntry(i);

                // 检查该条目的MBR是否与查询区域相交
                if (entry.m_region.intersectsRegion(query))
                {
                    if (node->isLeaf())
                    {
                        // 叶子节点：将数据添加到结果中
                        results.push_back(entry.m_data);
                    }
                    else
                    {
                        // 内部节点：将子节点添加到栈中
                        stack.push_back(entry.m_childNode);
                    }
                }
            }
        }

        return results;
    }

    Node *RTree::findLeaf(Node *node, id_type id, const Region &mbr) const
    {
        if (!node)
            return nullptr;

        // 递归查找
        return node->findLeaf(id, mbr);
    }

    bool RTree::remove(id_type id, const Region &mbr)
    {
        // 找到包含该条目的叶子节点
        Node *leaf = findLeaf(m_root.get(), id, mbr);
        if (!leaf || !leaf->isLeaf())
        {
            return false; // 未找到
        }

        // 找到叶子节点中的条目索引
        int entryIndex = leaf->findEntry(id);
        if (entryIndex < 0)
        {
            return false; // 未找到
        }

        // 移除条目
        leaf->removeEntry(entryIndex);
        m_size--;

        // 处理下溢（如果需要）
        if (leaf != m_root.get() && leaf->isUnderflow(m_minEntries))
        {
            // 简化版本：不处理下溢，只更新MBR
            // 完整实现应该处理节点合并或重新分配
        }

        // 向上更新MBR
        Node *current = leaf;
        while (current != m_root.get())
        {
            Node *parent = current->getParent();
            parent->updateMBR();
            current = parent;
        }

        return true;
    }

    void RTree::printStats() const
    {
        std::cout << "R-Tree Statistics:" << std::endl;
        std::cout << "  Height: " << m_treeHeight << std::endl;
        std::cout << "  Size: " << m_size << " items" << std::endl;
        std::cout << "  Max Entries: " << m_maxEntries << std::endl;
        std::cout << "  Min Entries: " << m_minEntries << std::endl;
        std::cout << "  Split Strategy: " << m_splitStrategy->getName() << std::endl;
    }

} // namespace RTree
