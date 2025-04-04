#include "Point.h"

#include <cstring>
#include <stdexcept>

namespace RTree
{

    Point::Point(uint32_t dimension) : m_dimension(dimension)
    {
        m_pCoords = new double[dimension];
        for (uint32_t i = 0; i < dimension; ++i)
        {
            m_pCoords[i] = 0.0;
        }
    }

    Point::Point(const double *coords, uint32_t dimension) : m_dimension(dimension)
    {
        m_pCoords = new double[dimension];
        memcpy(m_pCoords, coords, dimension * sizeof(double));
    }

    Point::Point(const Point &other) : m_dimension(other.m_dimension)
    {
        m_pCoords = new double[m_dimension];
        memcpy(m_pCoords, other.m_pCoords, m_dimension * sizeof(double));
    }

    Point::~Point()
    {
        delete[] m_pCoords;
    }

    Point &Point::operator=(const Point &other)
    {
        if (this != &other)
        {
            if (m_dimension != other.m_dimension)
            {
                delete[] m_pCoords;
                m_dimension = other.m_dimension;
                m_pCoords = new double[m_dimension];
            }
            memcpy(m_pCoords, other.m_pCoords, m_dimension * sizeof(double));
        }
        return *this;
    }

    bool Point::operator==(const Point &other) const
    {
        if (m_dimension != other.m_dimension)
            return false;

        for (uint32_t i = 0; i < m_dimension; ++i)
        {
            if (m_pCoords[i] != other.m_pCoords[i])
                return false;
        }
        return true;
    }

    double Point::getCoordinate(uint32_t index) const
    {
        if (index >= m_dimension)
        {
            throw std::out_of_range("Index out of range");
        }
        return m_pCoords[index];
    }

    void Point::setCoordinate(uint32_t index, double value)
    {
        if (index >= m_dimension)
        {
            throw std::out_of_range("Index out of range");
        }
        m_pCoords[index] = value;
    }

    uint32_t Point::getDimension() const
    {
        return m_dimension;
    }

} // namespace RTree