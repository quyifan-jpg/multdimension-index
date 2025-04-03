// DataGenerator.cpp
#include "DataGenerator.h"
#include <random>

std::vector<RTree::Point> DataGenerator::generateUniformData(int numPoints)
{
    std::vector<RTree::Point> points;

    // 使用随机数生成器生成均匀分布的数据
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 100.0); // 在0到100范围内生成随机坐标

    for (int i = 0; i < numPoints; ++i)
    {
        // 默认生成2维数据
        std::vector<double> coords = {dis(gen), dis(gen)};
        points.emplace_back(coords);
    }

    return points;
}
