// DataGenerator.h
#ifndef DATAGENERATOR_H
#define DATAGENERATOR_H

#include <vector>
#include "RTree/Point.h"

class DataGenerator
{
public:
    // 生成均匀分布的数据
    std::vector<RTree::Point> generateUniformData(int numPoints);
    // 可扩展：生成高斯分布、聚簇数据等
};

#endif // DATAGENERATOR_H
