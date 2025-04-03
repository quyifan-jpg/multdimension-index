#ifndef RTREE_ENTRY_H
#define RTREE_ENTRY_H

#include "Region.h"

namespace RTree
{

    // 前向声明
    class Node;
    typedef size_t id_type;

    // 条目类 - 树节点的基本单元
    class Entry
    {
    public:
        Region m_region; // 空间区域/MBR
        id_type m_id;    // 唯一标识符

        // 替换union，使用普通成员变量
        bool isLeaf;
        Node *m_childNode; // 内部节点：子节点指针
        void *m_data;      // 叶子节点：数据指针
        size_t m_dataSize; // 叶子节点：数据大小

        // 默认构造函数
        Entry() : m_id(0), isLeaf(false), m_childNode(nullptr), m_data(nullptr), m_dataSize(0) {}

        // 叶子节点的构造函数
        Entry(const Region &region, id_type id, void *data, size_t size)
            : m_region(region), m_id(id), isLeaf(true), m_childNode(nullptr), m_data(data), m_dataSize(size) {}

        // 内部节点的构造函数
        Entry(const Region &region, id_type id, Node *child)
            : m_region(region), m_id(id), isLeaf(false), m_childNode(child), m_data(nullptr), m_dataSize(0) {}

        double getEnlargement(const Region &r) const;
        double getOverlap(const Entry &other) const;
    };

} // namespace RTree

#endif // RTREE_ENTRY_H