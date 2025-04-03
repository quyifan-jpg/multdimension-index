#include "Region.h"
#include <algorithm>
#include <stdexcept>
#include <cmath>

namespace RTree
{

    Region::Region(const Point &low, const Point &high)
    {
        if (low.getDimension() != high.getDimension())
        {
            throw std::invalid_argument("Points have different dimensions");
        }

        m_low = low.m_coords;
        m_high = high.m_coords;
    }

    Region::Region(const Point &p)
    {
        m_low = p.m_coords;
        m_high = p.m_coords;
    }

    double Region::getArea() const
    {
        if (m_low.empty())
            return 0.0;

        double area = 1.0;
        for (size_t i = 0; i < m_low.size(); i++)
        {
            area *= (m_high[i] - m_low[i]);
        }
        return area;
    }

    bool Region::containsPoint(const Point &p) const
    {
        if (m_low.size() != p.getDimension())
            return false;

        for (size_t i = 0; i < m_low.size(); i++)
        {
            if (p.m_coords[i] < m_low[i] || p.m_coords[i] > m_high[i])
            {
                return false;
            }
        }
        return true;
    }

    bool Region::containsRegion(const Region &r) const
    {
        if (m_low.size() != r.getDimension())
            return false;

        for (size_t i = 0; i < m_low.size(); i++)
        {
            if (r.m_low[i] < m_low[i] || r.m_high[i] > m_high[i])
            {
                return false;
            }
        }
        return true;
    }

    bool Region::intersectsRegion(const Region &r) const
    {
        if (m_low.size() != r.getDimension())
            return false;

        for (size_t i = 0; i < m_low.size(); i++)
        {
            if (m_low[i] > r.m_high[i] || m_high[i] < r.m_low[i])
            {
                return false;
            }
        }
        return true;
    }

    double Region::getIntersectingArea(const Region &r) const
    {
        if (!intersectsRegion(r))
            return 0.0;

        double area = 1.0;
        for (size_t i = 0; i < m_low.size(); i++)
        {
            double low = std::max(m_low[i], r.m_low[i]);
            double high = std::min(m_high[i], r.m_high[i]);
            area *= (high - low);
        }
        return area;
    }

    void Region::combineRegion(const Region &r)
    {
        if (m_low.empty())
        {
            m_low = r.m_low;
            m_high = r.m_high;
            return;
        }

        if (m_low.size() != r.getDimension())
        {
            throw std::invalid_argument("Regions have different dimensions");
        }

        for (size_t i = 0; i < m_low.size(); i++)
        {
            m_low[i] = std::min(m_low[i], r.m_low[i]);
            m_high[i] = std::max(m_high[i], r.m_high[i]);
        }
    }

    Point Region::getCenter() const
    {
        std::vector<double> center(m_low.size());
        for (size_t i = 0; i < m_low.size(); i++)
        {
            center[i] = (m_low[i] + m_high[i]) / 2.0;
        }
        return Point(center);
    }

    double Region::getMinDistance(const Region &r) const
    {
        if (intersectsRegion(r))
            return 0.0;

        double distance = 0.0;
        for (size_t i = 0; i < m_low.size(); i++)
        {
            double dim_distance = 0.0;
            if (m_high[i] < r.m_low[i])
            {
                dim_distance = r.m_low[i] - m_high[i];
            }
            else if (m_low[i] > r.m_high[i])
            {
                dim_distance = m_low[i] - r.m_high[i];
            }
            distance += dim_distance * dim_distance;
        }
        return std::sqrt(distance);
    }

    double Region::getMargin() const
    {
        if (m_low.empty())
            return 0.0;

        double margin = 0.0;
        for (size_t i = 0; i < m_low.size(); i++)
        {
            margin += (m_high[i] - m_low[i]);
        }
        return 2.0 * margin; // 周长是边长的2倍
    }

} // namespace RTree