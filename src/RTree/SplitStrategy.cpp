#include "SplitStrategy.h"
#include <algorithm>
#include <limits>

namespace RTree
{

    // LinearSplitStrategy实现
    void LinearSplitStrategy::split(const std::vector<Entry> &entries,
                                    const Entry &newEntry,
                                    std::vector<size_t> &group1,
                                    std::vector<size_t> &group2)
    {
        // 清空输出分组
        group1.clear();
        group2.clear();

        if (entries.empty())
        {
            group1.push_back(0); // 只有新条目
            return;
        }

        // 所有条目，包括新条目
        std::vector<Entry> allEntries = entries;
        allEntries.push_back(newEntry);

        // 找到沿某个轴距离最远的两个条目作为种子
        size_t dim = allEntries[0].m_region.getDimension();
        double maxNormSep = -1.0;
        size_t seed1 = 0, seed2 = 0;

        for (size_t d = 0; d < dim; d++)
        {
            // 在当前维度上找最低和最高的索引
            size_t minIdx = 0, maxIdx = 0;

            for (size_t i = 1; i < allEntries.size(); i++)
            {
                if (allEntries[i].m_region.m_low[d] < allEntries[minIdx].m_region.m_low[d])
                {
                    minIdx = i;
                }
                if (allEntries[i].m_region.m_high[d] > allEntries[maxIdx].m_region.m_high[d])
                {
                    maxIdx = i;
                }
            }

            // 计算归一化分离度
            double minEntryWidth = allEntries[minIdx].m_region.m_high[d] - allEntries[minIdx].m_region.m_low[d];
            double maxEntryWidth = allEntries[maxIdx].m_region.m_high[d] - allEntries[maxIdx].m_region.m_low[d];
            double width = 0;

            for (const auto &entry : allEntries)
            {
                width = std::max(width, entry.m_region.m_high[d] - entry.m_region.m_low[d]);
            }

            double normSep = 0;
            if (width > 0)
            {
                normSep = (allEntries[maxIdx].m_region.m_low[d] - allEntries[minIdx].m_region.m_high[d]) / width;
            }

            if (normSep > maxNormSep)
            {
                maxNormSep = normSep;
                seed1 = minIdx;
                seed2 = maxIdx;
            }
        }

        // 如果没有很好的分离，使用第一个和最后一个条目
        if (maxNormSep < 0)
        {
            seed1 = 0;
            seed2 = allEntries.size() - 1;
        }

        // 分配种子
        group1.push_back(seed1);
        group2.push_back(seed2);

        // 分配其余条目
        std::vector<bool> assigned(allEntries.size(), false);
        assigned[seed1] = assigned[seed2] = true;

        // 为剩余每个条目选择扩展面积最小的组
        for (size_t i = 0; i < allEntries.size(); i++)
        {
            if (assigned[i])
                continue;

            // 计算MBR
            Region mbr1, mbr2;
            if (!group1.empty())
            {
                mbr1 = allEntries[group1[0]].m_region;
                for (size_t j = 1; j < group1.size(); j++)
                {
                    mbr1.combineRegion(allEntries[group1[j]].m_region);
                }
            }

            if (!group2.empty())
            {
                mbr2 = allEntries[group2[0]].m_region;
                for (size_t j = 1; j < group2.size(); j++)
                {
                    mbr2.combineRegion(allEntries[group2[j]].m_region);
                }
            }

            // 计算扩展面积
            double area1 = mbr1.getArea();
            double area2 = mbr2.getArea();

            Region expandedMbr1 = mbr1;
            expandedMbr1.combineRegion(allEntries[i].m_region);

            Region expandedMbr2 = mbr2;
            expandedMbr2.combineRegion(allEntries[i].m_region);

            double increase1 = expandedMbr1.getArea() - area1;
            double increase2 = expandedMbr2.getArea() - area2;

            // 选择扩展面积较小的组
            if (increase1 < increase2 || (increase1 == increase2 && group1.size() < group2.size()))
            {
                group1.push_back(i);
            }
            else
            {
                group2.push_back(i);
            }

            assigned[i] = true;
        }
    }

    // QuadraticSplitStrategy实现
    void QuadraticSplitStrategy::split(const std::vector<Entry> &entries,
                                       const Entry &newEntry,
                                       std::vector<size_t> &group1,
                                       std::vector<size_t> &group2)
    {
        // 清空输出分组
        group1.clear();
        group2.clear();

        if (entries.empty())
        {
            group1.push_back(0); // 只有新条目
            return;
        }

        // 所有条目，包括新条目
        std::vector<Entry> allEntries = entries;
        allEntries.push_back(newEntry);

        // 找到两个条目，它们一起构成的MBR比分别构成的MBR的面积和要大
        double maxWaste = -1.0;
        size_t seed1 = 0, seed2 = 0;

        for (size_t i = 0; i < allEntries.size(); i++)
        {
            for (size_t j = i + 1; j < allEntries.size(); j++)
            {
                Region combined;
                combined = allEntries[i].m_region;
                combined.combineRegion(allEntries[j].m_region);

                double waste = combined.getArea() - allEntries[i].m_region.getArea() - allEntries[j].m_region.getArea();

                if (waste > maxWaste)
                {
                    maxWaste = waste;
                    seed1 = i;
                    seed2 = j;
                }
            }
        }

        // 初始化两个组
        group1.push_back(seed1);
        group2.push_back(seed2);

        std::vector<bool> assigned(allEntries.size(), false);
        assigned[seed1] = assigned[seed2] = true;

        // 逐个分配剩余条目
        while (true)
        {
            // 检查是否所有条目都已分配
            bool allAssigned = true;
            for (size_t i = 0; i < allEntries.size(); i++)
            {
                if (!assigned[i])
                {
                    allAssigned = false;
                    break;
                }
            }

            if (allAssigned)
                break;

            // 计算当前MBR
            Region mbr1, mbr2;
            if (!group1.empty())
            {
                mbr1 = allEntries[group1[0]].m_region;
                for (size_t i = 1; i < group1.size(); i++)
                {
                    mbr1.combineRegion(allEntries[group1[i]].m_region);
                }
            }

            if (!group2.empty())
            {
                mbr2 = allEntries[group2[0]].m_region;
                for (size_t i = 1; i < group2.size(); i++)
                {
                    mbr2.combineRegion(allEntries[group2[i]].m_region);
                }
            }

            // 找下一个条目：它对两组MBR的面积扩展差异最大
            double maxDiff = -1.0;
            size_t next = 0;
            int preferred = -1; // 1表示组1, 2表示组2

            for (size_t i = 0; i < allEntries.size(); i++)
            {
                if (assigned[i])
                    continue;

                // 计算将该条目加入各组后的面积扩展
                Region newMbr1 = mbr1;
                newMbr1.combineRegion(allEntries[i].m_region);
                double increase1 = newMbr1.getArea() - mbr1.getArea();

                Region newMbr2 = mbr2;
                newMbr2.combineRegion(allEntries[i].m_region);
                double increase2 = newMbr2.getArea() - mbr2.getArea();

                double diff = std::abs(increase1 - increase2);

                if (diff > maxDiff)
                {
                    maxDiff = diff;
                    next = i;
                    preferred = (increase1 < increase2) ? 1 : 2;
                }
            }

            // 分配到首选组，除非这会导致某组元素太少
            if (preferred == 2 && group1.size() >= 1)
            {
                group2.push_back(next);
            }
            else if (preferred == 1 && group2.size() >= 1)
            {
                group1.push_back(next);
            }
            else if (group1.size() < group2.size())
            {
                group1.push_back(next);
            }
            else
            {
                group2.push_back(next);
            }

            assigned[next] = true;
        }
    }

    // RStarSplitStrategy实现
    void RStarSplitStrategy::split(const std::vector<Entry> &entries,
                                   const Entry &newEntry,
                                   std::vector<size_t> &group1,
                                   std::vector<size_t> &group2)
    {
        // 清空输出分组
        group1.clear();
        group2.clear();

        if (entries.empty())
        {
            group1.push_back(0); // 只有新条目
            return;
        }

        // 所有条目，包括新条目
        std::vector<Entry> allEntries = entries;
        allEntries.push_back(newEntry);

        size_t dim = allEntries[0].m_region.getDimension();
        size_t size = allEntries.size();

        // 确定沿哪个维度分割
        double minMargin = std::numeric_limits<double>::max();
        size_t splitAxis = 0;

        for (size_t d = 0; d < dim; d++)
        {
            // 按下界和上界排序
            std::vector<size_t> sortedByLow(size);
            std::vector<size_t> sortedByHigh(size);

            for (size_t i = 0; i < size; i++)
            {
                sortedByLow[i] = i;
                sortedByHigh[i] = i;
            }

            std::sort(sortedByLow.begin(), sortedByLow.end(),
                      [&allEntries, d](size_t i1, size_t i2)
                      {
                          return allEntries[i1].m_region.m_low[d] < allEntries[i2].m_region.m_low[d];
                      });

            std::sort(sortedByHigh.begin(), sortedByHigh.end(),
                      [&allEntries, d](size_t i1, size_t i2)
                      {
                          return allEntries[i1].m_region.m_high[d] < allEntries[i2].m_region.m_high[d];
                      });

            // 考虑所有可能的分割
            size_t minFanout = size * 0.4; // 40%
            double margin = 0.0;

            for (size_t k = minFanout; k <= size - minFanout; k++)
            {
                // 分割基于下界
                Region mbr1Low, mbr2Low;
                for (size_t i = 0; i < k; i++)
                {
                    if (i == 0)
                    {
                        mbr1Low = allEntries[sortedByLow[i]].m_region;
                    }
                    else
                    {
                        mbr1Low.combineRegion(allEntries[sortedByLow[i]].m_region);
                    }
                }

                for (size_t i = k; i < size; i++)
                {
                    if (i == k)
                    {
                        mbr2Low = allEntries[sortedByLow[i]].m_region;
                    }
                    else
                    {
                        mbr2Low.combineRegion(allEntries[sortedByLow[i]].m_region);
                    }
                }

                // 分割基于上界
                Region mbr1High, mbr2High;
                for (size_t i = 0; i < k; i++)
                {
                    if (i == 0)
                    {
                        mbr1High = allEntries[sortedByHigh[i]].m_region;
                    }
                    else
                    {
                        mbr1High.combineRegion(allEntries[sortedByHigh[i]].m_region);
                    }
                }

                for (size_t i = k; i < size; i++)
                {
                    if (i == k)
                    {
                        mbr2High = allEntries[sortedByHigh[i]].m_region;
                    }
                    else
                    {
                        mbr2High.combineRegion(allEntries[sortedByHigh[i]].m_region);
                    }
                }

                // 计算周长总和
                margin += mbr1Low.getMargin() + mbr2Low.getMargin();
                margin += mbr1High.getMargin() + mbr2High.getMargin();
            }

            // 选周长和最小的轴
            if (margin < minMargin)
            {
                minMargin = margin;
                splitAxis = d;
            }
        }

        // 分割轴确定后，选择最佳分割索引
        // 按下界和上界排序
        std::vector<size_t> sortedByLow(size);
        std::vector<size_t> sortedByHigh(size);

        for (size_t i = 0; i < size; i++)
        {
            sortedByLow[i] = i;
            sortedByHigh[i] = i;
        }

        std::sort(sortedByLow.begin(), sortedByLow.end(),
                  [&allEntries, splitAxis](size_t i1, size_t i2)
                  {
                      return allEntries[i1].m_region.m_low[splitAxis] <
                             allEntries[i2].m_region.m_low[splitAxis];
                  });

        std::sort(sortedByHigh.begin(), sortedByHigh.end(),
                  [&allEntries, splitAxis](size_t i1, size_t i2)
                  {
                      return allEntries[i1].m_region.m_high[splitAxis] <
                             allEntries[i2].m_region.m_high[splitAxis];
                  });

        // 寻找最小重叠的分割
        double minOverlap = std::numeric_limits<double>::max();
        double minArea = std::numeric_limits<double>::max();
        size_t splitIndex = 0;
        bool useSortedByLow = true;

        size_t minFanout = size * 0.4; // 40%

        // 尝试基于下界的所有分割点
        for (size_t k = minFanout; k <= size - minFanout; k++)
        {
            Region mbr1, mbr2;

            // 计算两个MBR
            for (size_t i = 0; i < k; i++)
            {
                if (i == 0)
                {
                    mbr1 = allEntries[sortedByLow[i]].m_region;
                }
                else
                {
                    mbr1.combineRegion(allEntries[sortedByLow[i]].m_region);
                }
            }

            for (size_t i = k; i < size; i++)
            {
                if (i == k)
                {
                    mbr2 = allEntries[sortedByLow[i]].m_region;
                }
                else
                {
                    mbr2.combineRegion(allEntries[sortedByLow[i]].m_region);
                }
            }

            // 计算重叠和面积总和
            double overlap = mbr1.getIntersectingArea(mbr2);
            double area = mbr1.getArea() + mbr2.getArea();

            if (overlap < minOverlap || (overlap == minOverlap && area < minArea))
            {
                minOverlap = overlap;
                minArea = area;
                splitIndex = k;
                useSortedByLow = true;
            }
        }

        // 尝试基于上界的所有分割点
        for (size_t k = minFanout; k <= size - minFanout; k++)
        {
            Region mbr1, mbr2;

            // 计算两个MBR
            for (size_t i = 0; i < k; i++)
            {
                if (i == 0)
                {
                    mbr1 = allEntries[sortedByHigh[i]].m_region;
                }
                else
                {
                    mbr1.combineRegion(allEntries[sortedByHigh[i]].m_region);
                }
            }

            for (size_t i = k; i < size; i++)
            {
                if (i == k)
                {
                    mbr2 = allEntries[sortedByHigh[i]].m_region;
                }
                else
                {
                    mbr2.combineRegion(allEntries[sortedByHigh[i]].m_region);
                }
            }

            // 计算重叠和面积总和
            double overlap = mbr1.getIntersectingArea(mbr2);
            double area = mbr1.getArea() + mbr2.getArea();

            if (overlap < minOverlap || (overlap == minOverlap && area < minArea))
            {
                minOverlap = overlap;
                minArea = area;
                splitIndex = k;
                useSortedByLow = false;
            }
        }

        // 应用最终分割
        std::vector<size_t> &sortedIndices = useSortedByLow ? sortedByLow : sortedByHigh;

        for (size_t i = 0; i < splitIndex; i++)
        {
            group1.push_back(sortedIndices[i]);
        }

        for (size_t i = splitIndex; i < size; i++)
        {
            group2.push_back(sortedIndices[i]);
        }
    }

} // namespace RTree