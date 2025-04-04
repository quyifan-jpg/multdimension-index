//
// Created by Shengqiao Zhao on 2025-04-04.
//

#ifndef NODE_H
#define NODE_H
#include <vector>

#include "src/RTree/impl/common.h"

namespace RTree{
    class Data;
    class Region;
    class SplitStrategy;

    class Node
    {
    public:
        virtual ~Node() = default;
        virtual bool isLeaf() const = 0;
        virtual const Region &getMBR() const = 0;
        virtual void insert(Data *data) = 0;
        virtual bool remove(id_type id, const Region &mbr) = 0;
        virtual bool isEmpty() = 0;
        virtual std::vector<Data *> search(const Region &query) = 0;
        virtual bool shouldSplit() const = 0;
        virtual std::pair<Node *, Node *> split() = 0;
        virtual uint32_t getHeight() const = 0;
        virtual Node *chooseSubtree(const Region &mbr)
        {
            return this;
        };

        // Set split strategy
        void setSplitStrategy(const SplitStrategy *strategy)
        {
            m_splitStrategy = strategy;
        }

    protected:
        const SplitStrategy *m_splitStrategy = nullptr;
    };
}

#endif //NODE_H
