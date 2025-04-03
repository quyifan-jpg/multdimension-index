#ifndef RTREE_REGION_H
#define RTREE_REGION_H

#include <vector>
#include "Point.h"

namespace RTree
{

    // 区域类 - 表示MBR(最小边界矩形)
    class Region
    {
    public:
        std::vector<double> m_low;  // 每个维度的下边界
        std::vector<double> m_high; // 每个维度的上边界

        Region() {}
        Region(const Point &low, const Point &high);
        Region(const std::vector<double> &low, const std::vector<double> &high)
            : m_low(low), m_high(high) {}
        explicit Region(const Point &p);

        size_t getDimension() const { return m_low.size(); }
        double getArea() const;
        bool containsPoint(const Point &p) const;
        bool containsRegion(const Region &r) const;
        bool intersectsRegion(const Region &r) const;
        double getIntersectingArea(const Region &r) const;
        void combineRegion(const Region &r);
        Point getCenter() const;
        double getMinDistance(const Region &r) const;
        double getMargin() const; // R*-tree分裂会用到的周长计算
    };

} // namespace RTree

#endif // RTREE_REGION_H