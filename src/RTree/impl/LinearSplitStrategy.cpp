#include "../Rtree.h"
#include <algorithm>
#include <limits>
#include <cmath>

namespace RTree
{
    // 添加显式的默认构造函数和析构函数定义，以确保虚函数表正确生成
    LinearSplitStrategy::LinearSplitStrategy() = default;
    LinearSplitStrategy::~LinearSplitStrategy() = default;

    std::pair<std::vector<Data *>, std::vector<Data *>>
    LinearSplitStrategy::splitLeafEntries(const std::vector<Data *> &entries, uint32_t capacity) const
    {
        std::vector<Data *> group1;
        std::vector<Data *> group2;

        // 简单的线性分裂 - 将条目平均分配到两个组中
        size_t midpoint = entries.size() / 2;

        // 确保每个组至少有capacity/2个条目
        uint32_t minEntries = capacity / 2;

        for (size_t i = 0; i < entries.size(); ++i)
        {
            if (i < midpoint)
            {
                group1.push_back(entries[i]);
            }
            else
            {
                group2.push_back(entries[i]);
            }
        }

        // 确保两组满足最小条目数要求
        if (group1.size() < minEntries)
        {
            // 从group2移动条目到group1
            while (group1.size() < minEntries && group2.size() > minEntries)
            {
                group1.push_back(group2.front());
                group2.erase(group2.begin());
            }
        }
        else if (group2.size() < minEntries)
        {
            // 从group1移动条目到group2
            while (group2.size() < minEntries && group1.size() > minEntries)
            {
                group2.insert(group2.begin(), group1.back());
                group1.pop_back();
            }
        }

        return {group1, group2};
    }

    std::pair<std::vector<Node *>, std::vector<Node *>>
    LinearSplitStrategy::splitInternalChildren(const std::vector<Node *> &children, uint32_t capacity) const
    {
        std::vector<Node *> group1;
        std::vector<Node *> group2;

        // 简单的线性分裂 - 将子节点平均分配到两个组中
        size_t midpoint = children.size() / 2;

        // 确保每个组至少有capacity/2个子节点
        uint32_t minChildren = capacity / 2;

        for (size_t i = 0; i < children.size(); ++i)
        {
            if (i < midpoint)
            {
                group1.push_back(children[i]);
            }
            else
            {
                group2.push_back(children[i]);
            }
        }

        // 确保两组满足最小子节点数要求
        if (group1.size() < minChildren)
        {
            // 从group2移动子节点到group1
            while (group1.size() < minChildren && group2.size() > minChildren)
            {
                group1.push_back(group2.front());
                group2.erase(group2.begin());
            }
        }
        else if (group2.size() < minChildren)
        {
            // 从group1移动子节点到group2
            while (group2.size() < minChildren && group1.size() > minChildren)
            {
                group2.insert(group2.begin(), group1.back());
                group1.pop_back();
            }
        }

        return {group1, group2};
    }

    std::string LinearSplitStrategy::getName() const
    {
        return "LinearSplit";
    }

} // namespace RTree