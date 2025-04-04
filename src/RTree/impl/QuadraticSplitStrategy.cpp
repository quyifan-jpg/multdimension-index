#include "../Rtree.h"
#include <algorithm>
#include <limits>
#include <cmath>

namespace RTree
{
    // 添加显式的默认构造函数和析构函数定义，以确保虚函数表正确生成
    QuadraticSplitStrategy::QuadraticSplitStrategy() = default;
    QuadraticSplitStrategy::~QuadraticSplitStrategy() = default;

    std::pair<std::vector<Data *>, std::vector<Data *>>
    QuadraticSplitStrategy::splitLeafEntries(const std::vector<Data *> &entries, uint32_t capacity) const
    {
        std::vector<Data *> group1;
        std::vector<Data *> group2;

        // 找出最佳的两个种子条目
        auto [seed1, seed2] = pickSeeds(entries);

        // 将种子条目分配到两个组
        group1.push_back(entries[seed1]);
        group2.push_back(entries[seed2]);

        // 创建工作副本，以跟踪尚未分配的条目
        std::vector<Data *> remaining;
        for (size_t i = 0; i < entries.size(); ++i)
        {
            if (i != seed1 && i != seed2)
            {
                remaining.push_back(entries[i]);
            }
        }

        // 确保每个组至少有capacity/2个条目
        uint32_t minEntries = capacity / 2;

        // 计算初始MBR
        Region mbr1 = group1[0]->getRegion();
        Region mbr2 = group2[0]->getRegion();

        // 将剩余条目分配到适当的组
        while (!remaining.empty())
        {
            // 如果一个组的条目太少，将剩余的全部分配给它
            if (group1.size() + remaining.size() <= minEntries)
            {
                for (auto &entry : remaining)
                {
                    group1.push_back(entry);
                    mbr1.combine(entry->getRegion());
                }
                remaining.clear();
                break;
            }
            if (group2.size() + remaining.size() <= minEntries)
            {
                for (auto &entry : remaining)
                {
                    group2.push_back(entry);
                    mbr2.combine(entry->getRegion());
                }
                remaining.clear();
                break;
            }

            // 计算每个条目的面积增长并选择增长最大的条目
            double maxDiff = -std::numeric_limits<double>::max();
            size_t selectedIndex = 0;
            size_t targetGroup = 0; // 0表示group1, 1表示group2

            for (size_t i = 0; i < remaining.size(); ++i)
            {
                const Region &entryRegion = remaining[i]->getRegion();

                // 计算将条目分配到group1后的面积增长
                Region combinedMbr1 = mbr1;
                combinedMbr1.combine(entryRegion);
                double growth1 = combinedMbr1.getArea() - mbr1.getArea();

                // 计算将条目分配到group2后的面积增长
                Region combinedMbr2 = mbr2;
                combinedMbr2.combine(entryRegion);
                double growth2 = combinedMbr2.getArea() - mbr2.getArea();

                // 计算增长差异
                double diff = std::abs(growth1 - growth2);
                if (diff > maxDiff)
                {
                    maxDiff = diff;
                    selectedIndex = i;
                    targetGroup = (growth1 < growth2) ? 0 : 1;
                }
            }

            // 将选中的条目分配到目标组
            Data *selectedEntry = remaining[selectedIndex];
            if (targetGroup == 0)
            {
                group1.push_back(selectedEntry);
                mbr1.combine(selectedEntry->getRegion());
            }
            else
            {
                group2.push_back(selectedEntry);
                mbr2.combine(selectedEntry->getRegion());
            }

            // 从未分配列表中移除该条目
            remaining.erase(remaining.begin() + selectedIndex);
        }

        return {group1, group2};
    }
    //     std::pair<std::vector<Data *>, std::vector<Data *>>
    // LinearSplitStrategy::splitLeafEntries(const std::vector<Data *> &entries, uint32_t capacity) const
    //
    std::pair<std::vector<Node *>, std::vector<Node *>>
    QuadraticSplitStrategy::splitInternalChildren(const std::vector<Node *> &children, uint32_t capacity) const
    {
        std::vector<Node *> group1;
        std::vector<Node *> group2;

        // 找出最佳的两个种子节点
        auto [seed1, seed2] = pickSeeds(children);

        // 将种子节点分配到两个组
        group1.push_back(children[seed1]);
        group2.push_back(children[seed2]);

        // 创建工作副本，以跟踪尚未分配的节点
        std::vector<Node *> remaining;
        for (size_t i = 0; i < children.size(); ++i)
        {
            if (i != seed1 && i != seed2)
            {
                remaining.push_back(children[i]);
            }
        }

        // 确保每个组至少有capacity/2个子节点
        uint32_t minChildren = capacity / 2;

        // 计算初始MBR
        Region mbr1 = group1[0]->getMBR();
        Region mbr2 = group2[0]->getMBR();

        // 将剩余节点分配到适当的组
        while (!remaining.empty())
        {
            // 如果一个组的节点太少，将剩余的全部分配给它
            if (group1.size() + remaining.size() <= minChildren)
            {
                for (auto &child : remaining)
                {
                    group1.push_back(child);
                    mbr1.combine(child->getMBR());
                }
                remaining.clear();
                break;
            }
            if (group2.size() + remaining.size() <= minChildren)
            {
                for (auto &child : remaining)
                {
                    group2.push_back(child);
                    mbr2.combine(child->getMBR());
                }
                remaining.clear();
                break;
            }

            // 计算每个节点的面积增长并选择增长最大的节点
            double maxDiff = -std::numeric_limits<double>::max();
            size_t selectedIndex = 0;
            size_t targetGroup = 0; // 0表示group1, 1表示group2

            for (size_t i = 0; i < remaining.size(); ++i)
            {
                const Region &childMbr = remaining[i]->getMBR();

                // 计算将节点分配到group1后的面积增长
                Region combinedMbr1 = mbr1;
                combinedMbr1.combine(childMbr);
                double growth1 = combinedMbr1.getArea() - mbr1.getArea();

                // 计算将节点分配到group2后的面积增长
                Region combinedMbr2 = mbr2;
                combinedMbr2.combine(childMbr);
                double growth2 = combinedMbr2.getArea() - mbr2.getArea();

                // 计算增长差异
                double diff = std::abs(growth1 - growth2);
                if (diff > maxDiff)
                {
                    maxDiff = diff;
                    selectedIndex = i;
                    targetGroup = (growth1 < growth2) ? 0 : 1;
                }
            }

            // 将选中的节点分配到目标组
            Node *selectedChild = remaining[selectedIndex];
            if (targetGroup == 0)
            {
                group1.push_back(selectedChild);
                mbr1.combine(selectedChild->getMBR());
            }
            else
            {
                group2.push_back(selectedChild);
                mbr2.combine(selectedChild->getMBR());
            }

            // 从未分配列表中移除该节点
            remaining.erase(remaining.begin() + selectedIndex);
        }

        return {group1, group2};
    }

    std::pair<size_t, size_t> QuadraticSplitStrategy::pickSeeds(const std::vector<Data *> &entries) const
    {
        size_t seed1 = 0;
        size_t seed2 = 0;
        double maxWastedArea = -1.0;

        // 寻找组合后浪费面积最大的一对
        for (size_t i = 0; i < entries.size(); ++i)
        {
            const Region &region1 = entries[i]->getRegion();

            for (size_t j = i + 1; j < entries.size(); ++j)
            {
                const Region &region2 = entries[j]->getRegion();

                // 计算合并后的区域
                Region combinedRegion = region1;
                combinedRegion.combine(region2);

                // 计算浪费的面积 = 合并区域面积 - 两个原始区域的面积
                double wastedArea = combinedRegion.getArea() - region1.getArea() - region2.getArea();

                if (wastedArea > maxWastedArea)
                {
                    maxWastedArea = wastedArea;
                    seed1 = i;
                    seed2 = j;
                }
            }
        }

        return {seed1, seed2};
    }

    std::pair<size_t, size_t> QuadraticSplitStrategy::pickSeeds(const std::vector<Node *> &children) const
    {
        size_t seed1 = 0;
        size_t seed2 = 0;
        double maxWastedArea = -1.0;

        // 寻找组合后浪费面积最大的一对
        for (size_t i = 0; i < children.size(); ++i)
        {
            const Region &region1 = children[i]->getMBR();

            for (size_t j = i + 1; j < children.size(); ++j)
            {
                const Region &region2 = children[j]->getMBR();

                // 计算合并后的区域
                Region combinedRegion = region1;
                combinedRegion.combine(region2);

                // 计算浪费的面积 = 合并区域面积 - 两个原始区域的面积
                double wastedArea = combinedRegion.getArea() - region1.getArea() - region2.getArea();

                if (wastedArea > maxWastedArea)
                {
                    maxWastedArea = wastedArea;
                    seed1 = i;
                    seed2 = j;
                }
            }
        }

        return {seed1, seed2};
    }

    double QuadraticSplitStrategy::calculateAreaEnlargement(const Region &r1, const Region &r2) const
    {
        Region combined = r1;
        combined.combine(r2);
        return combined.getArea() - r1.getArea();
    }

    std::string QuadraticSplitStrategy::getName() const
    {
        return "QuadraticSplit";
    }

} // namespace RTree