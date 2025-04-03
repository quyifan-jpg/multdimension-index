#include "Entry.h"

namespace RTree {

double Entry::getEnlargement(const Region& r) const {
    // 计算扩展区域后的面积增加量
    Region combined = m_region;
    combined.combineRegion(r);
    return combined.getArea() - m_region.getArea();
}

double Entry::getOverlap(const Entry& other) const {
    return m_region.getIntersectingArea(other.m_region);
}

} // namespace RTree 