#include "Point.h"
#include <stdexcept>

namespace RTree
{

    double Point::distance(const Point &p) const
    {
        if (m_coords.size() != p.m_coords.size())
        {
            throw std::invalid_argument("Points have different dimensions");
        }

        double sum = 0.0;
        for (size_t i = 0; i < m_coords.size(); i++)
        {
            double diff = m_coords[i] - p.m_coords[i];
            sum += diff * diff;
        }
        return std::sqrt(sum);
    }

} // namespace RTree