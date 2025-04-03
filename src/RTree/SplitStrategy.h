#ifndef RTREE_SPLIT_STRATEGY_H
#define RTREE_SPLIT_STRATEGY_H

#include <vector>
#include <string>
#include "Entry.h"

namespace RTree
{

    // 分裂策略接口
    class SplitStrategy
    {
    public:
        virtual ~SplitStrategy() = default;
        virtual void split(const std::vector<Entry> &entries,
                           const Entry &newEntry,
                           std::vector<size_t> &group1,
                           std::vector<size_t> &group2) = 0;
        virtual std::string getName() const = 0;
    };

    // 线性分裂策略
    class LinearSplitStrategy : public SplitStrategy
    {
    public:
        void split(const std::vector<Entry> &entries,
                   const Entry &newEntry,
                   std::vector<size_t> &group1,
                   std::vector<size_t> &group2) override;
        std::string getName() const override { return "Linear"; }
    };

    // 二次分裂策略
    class QuadraticSplitStrategy : public SplitStrategy
    {
    public:
        void split(const std::vector<Entry> &entries,
                   const Entry &newEntry,
                   std::vector<size_t> &group1,
                   std::vector<size_t> &group2) override;
        std::string getName() const override { return "Quadratic"; }
    };

    // R*-tree分裂策略
    class RStarSplitStrategy : public SplitStrategy
    {
    public:
        void split(const std::vector<Entry> &entries,
                   const Entry &newEntry,
                   std::vector<size_t> &group1,
                   std::vector<size_t> &group2) override;
        std::string getName() const override { return "RStar"; }
    };

} // namespace RTree

#endif // RTREE_SPLIT_STRATEGY_H