// main.cpp
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include "RTree/RTree.h"

using namespace RTree;

// 生成随机点用于测试
std::vector<Point> generateRandomPoints(size_t count, size_t dimension, double min, double max)
{
    std::vector<Point> points;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);

    for (size_t i = 0; i < count; i++)
    {
        std::vector<double> coords(dimension);
        for (size_t d = 0; d < dimension; d++)
        {
            coords[d] = dis(gen);
        }
        points.emplace_back(coords);
    }

    return points;
}

// 测试基本的插入和搜索操作
void testBasicOperations()
{
    std::cout << "===== 测试基本操作 =====" << std::endl;

    // 创建R树 - 使用二次分裂策略，最大条目数为16
    RTree::RTree rtree(16, std::make_shared<QuadraticSplitStrategy>());

    // 生成一些随机点
    std::vector<Point> points = generateRandomPoints(50, 2, 0.0, 100.0);

    // 插入点
    auto startTime = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < points.size(); i++)
    {
        // 创建包含点的区域
        Region mbr(points[i]);
        // 将点的索引作为数据插入
        int *data = new int(i);
        rtree.insert(data, sizeof(int), mbr);
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    std::cout << "插入 " << points.size() << " 个点，耗时 " << duration.count() << " ms" << std::endl;

    // 定义搜索区域
    Region searchRegion;
    searchRegion.m_low = {25.0, 25.0};
    searchRegion.m_high = {75.0, 75.0};

    // 搜索点
    startTime = std::chrono::high_resolution_clock::now();
    std::vector<void *> results = rtree.search(searchRegion);
    endTime = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    std::cout << "在搜索区域中找到 " << results.size() << " 个点，耗时 "
              << duration.count() << " ms" << std::endl;

    // 打印前几个结果
    std::cout << "前10个结果（如果少于10个则打印所有）:" << std::endl;
    for (size_t i = 0; i < std::min(results.size(), size_t(10)); i++)
    {
        int index = *static_cast<int *>(results[i]);
        std::cout << "  点 " << index << ": (";
        const auto &coords = points[index].m_coords;
        for (size_t d = 0; d < coords.size(); d++)
        {
            std::cout << coords[d];
            if (d < coords.size() - 1)
            {
                std::cout << ", ";
            }
        }
        std::cout << ")" << std::endl;
    }

    // 打印树的统计信息
    rtree.printStats();

    // 清理内存
    for (auto result : results)
    {
        delete static_cast<int *>(result);
    }
}

// 比较不同的分裂策略
void compareSplitStrategies()
{
    std::cout << "\n===== 比较分裂策略 =====" << std::endl;

    // 生成随机点
    size_t pointCount = 500;
    std::vector<Point> points = generateRandomPoints(pointCount, 2, 0.0, 1000.0);

    // 测试每种分裂策略
    std::vector<std::shared_ptr<SplitStrategy>> strategies = {
        std::make_shared<LinearSplitStrategy>(),
        std::make_shared<QuadraticSplitStrategy>(),
        std::make_shared<RStarSplitStrategy>()};

    std::vector<std::string> strategyNames = {
        "Linear (线性)",
        "Quadratic (二次)",
        "R* (R*树)"};

    for (size_t i = 0; i < strategies.size(); i++)
    {
        std::cout << "\n测试 " << strategyNames[i] << " 分裂策略:" << std::endl;

        // 创建树
        RTree::RTree rtree(50, strategies[i]);

        // 插入点并测量时间
        auto startTime = std::chrono::high_resolution_clock::now();

        for (size_t j = 0; j < points.size(); j++)
        {
            Region mbr(points[j]);
            int *data = new int(j);
            rtree.insert(data, sizeof(int), mbr);
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        auto insertDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        std::cout << "  插入时间: " << insertDuration.count() << " ms" << std::endl;

        // 定义搜索区域
        Region searchRegion;
        searchRegion.m_low = {250.0, 250.0};
        searchRegion.m_high = {750.0, 750.0};

        // 测量搜索时间
        startTime = std::chrono::high_resolution_clock::now();
        std::vector<void *> results = rtree.search(searchRegion);
        endTime = std::chrono::high_resolution_clock::now();
        auto searchDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        std::cout << "  搜索时间: " << searchDuration.count() << " ms" << std::endl;
        std::cout << "  找到 " << results.size() << " 个点" << std::endl;

        // 打印树的统计信息
        std::cout << "  树高度: " << rtree.getHeight() << std::endl;
        std::cout << "  数据项数量: " << rtree.getSize() << std::endl;
        std::cout << "  分裂策略: " << rtree.getSplitStrategy()->getName() << std::endl;

        // 清理内存
        for (auto result : results)
        {
            delete static_cast<int *>(result);
        }
    }
}

// 主函数
int main()
{
    std::cout << "R-Tree实现测试" << std::endl;
    std::cout << "=========================" << std::endl;

    // 测试基本操作
    testBasicOperations();

    // 比较分裂策略
    compareSplitStrategies();

    return 0;
}
