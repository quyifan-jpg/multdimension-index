//
// Created by Shengqiao Zhao on 2025-04-04.
//

#ifndef RTREE_H
#define RTREE_H
#include <cstdint>
#include <vector>

#include "src/RTree/impl/common.h"

namespace RTree {
    class Node;
    class Point;
    class Data;
    class Region;
    class SplitStrategy;

    class RTree
    {
    public:
        RTree(uint32_t dimension = 2, uint32_t nodeCapacity = 8, const SplitStrategy *splitStrategy = nullptr);
        ~RTree();

        void insert(uint32_t dataLength, const uint8_t *pData, const Region &mbr, id_type id);
        bool remove(const Region &mbr, id_type id);

        // Query method - Return result set without using visitor pattern
        std::vector<Data *> intersectionQuery(const Region &query);
        std::vector<Data *> containmentQuery(const Region &query);
        std::vector<Data *> pointQuery(const Point &point);

        // Helper methods
        uint32_t getDimension() const;
        uint32_t getNodeCapacity() const;
        uint32_t getHeight() const;

        // Set split strategy
        void setSplitStrategy(const SplitStrategy *splitStrategy);

    private:
        Node *m_root_node;
        uint32_t m_dimension;
        uint32_t m_nodeCapacity;
        id_type m_nextId;
        const SplitStrategy *m_splitStrategy;

        void insertData_impl(Data *data);
        Node *findLeaf(Node *node, const Region &mbr, id_type id);
        void adjustTree(Node *node, Node *newNode = nullptr);
    };

}

#endif //RTREE_H
