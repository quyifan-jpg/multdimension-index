#ifndef RTREE_H
#define RTREE_H

#include <vector>
#include <memory>
#include <cmath>
#include <algorithm>
#include <limits>
#include <string>
#include <queue>
#include "Point.h"
#include "Region.h"
#include "Entry.h"
#include "SplitStrategy.h"
#include "Node.h"

namespace RTree
{

    // 前向声明
    class Node;
    class Entry;
    class Point;
    class Region;
    class RTree;
    class SplitStrategy;
    class LeafNode;
    class InternalNode;

    typedef std::unique_ptr<Node> NodePtr;
    typedef size_t id_type;

    // 距离条目 - 用于最近邻查询
    struct DistanceEntry
    {
        double distance;
        void *data;

        DistanceEntry(double dist, void *d) : distance(dist), data(d) {}

        bool operator<(const DistanceEntry &other) const
        {
            return distance > other.distance; // 使用小顶堆
        }
    };

    // R-tree主类
    class RTree
    {
    private:
        std::unique_ptr<Node> m_root; // 根节点
        size_t m_size;                // 数据项数量
        size_t m_maxEntries;          // 节点最大条目数
        size_t m_minEntries;          // 节点最小条目数
        size_t m_treeHeight;          // 树高度
        id_type m_nextID;             // 下一个可用ID

        // 调整树方法 (插入后平衡)
        void adjustTree(Node *node, Node *newNode = nullptr);

        // 生成唯一ID
        id_type generateID() { return m_nextID++; }

        std::shared_ptr<SplitStrategy> m_splitStrategy;

        // 查找包含特定ID和MBR的叶子节点
        Node *findLeaf(Node *node, id_type id, const Region &mbr) const;

    public:
        // 构造函数
        RTree(size_t maxEntries = 8,
              std::shared_ptr<SplitStrategy> strategy = std::make_shared<QuadraticSplitStrategy>())
            : m_size(0), m_maxEntries(maxEntries),
              m_minEntries(maxEntries / 2), m_treeHeight(1), m_nextID(1),
              m_splitStrategy(strategy)
        {
            // 创建根节点
            m_root = std::unique_ptr<Node>(new LeafNode(this));
        }

        // 析构函数
        ~RTree() {}

        // 基本信息访问
        size_t getSize() const { return m_size; }
        size_t getHeight() const { return m_treeHeight; }
        size_t getMaxEntries() const { return m_maxEntries; }
        size_t getMinEntries() const { return m_minEntries; }
        Node *getRoot() const { return m_root.get(); }

        // 分裂策略访问和修改
        std::shared_ptr<SplitStrategy> getSplitStrategy() const { return m_splitStrategy; }
        void setSplitStrategy(std::shared_ptr<SplitStrategy> strategy)
        {
            m_splitStrategy = strategy;
        }

        // 插入数据
        void insert(void *data, size_t dataSize, const Region &mbr);

        // 搜索操作
        std::vector<void *> search(const Region &query) const;

        // 删除操作
        bool remove(id_type id, const Region &mbr);

        // 统计信息
        void printStats() const;
    };

} // namespace RTree

#endif // RTREE_H