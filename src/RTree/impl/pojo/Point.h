//
// Created by Shengqiao Zhao on 2025-04-04.
//

#ifndef POINT_H
#define POINT_H

#include <cstdint>

namespace RTree {
    class Point
    {
    public:
        explicit Point(uint32_t dimension = 2);
        Point(const double *coords, uint32_t dimension);
        Point(const Point &other);
        ~Point();

        Point &operator=(const Point &other);
        bool operator==(const Point &other) const;

        double getCoordinate(uint32_t index) const;
        void setCoordinate(uint32_t index, double value);
        uint32_t getDimension() const;

    private:
        uint32_t m_dimension;
        double *m_pCoords;
    };
}

#endif //POINT_H
