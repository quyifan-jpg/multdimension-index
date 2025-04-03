#ifndef RTREE_POINT_H
#define RTREE_POINT_H

#include <vector>
#include <cmath>

namespace RTree
{

    // 点类 - 表示空间中的一个点
    class Point
    {
    public:
        std::vector<double> m_coords; // 坐标值

        Point() {}
        Point(const std::vector<double> &coords) : m_coords(coords) {}
        Point(double x, double y)
        {
            m_coords.push_back(x);
            m_coords.push_back(y);
        }

        size_t getDimension() const { return m_coords.size(); }
        double distance(const Point &p) const;
    };

} // namespace RTree

#endif // RTREE_POINT_H