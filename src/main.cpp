// test_correctness.cpp
// 用于测试R树实现的正确性，通过对比向量和R树的搜索结果

#include "RTree/Rtree.h"
#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <algorithm>
#include <cassert>
#include <set>

// 定义测试数据条目的结构
struct TestPoint
{
    double x, y;
    int id;

    // 构造函数
    TestPoint(double x, double y, int id) : x(x), y(y), id(id) {}

    // 判断点是否在查询区域内
    bool isInRegion(double minX, double minY, double maxX, double maxY) const
    {
        return (x >= minX && x <= maxX && y >= minY && y <= maxY);
    }
};

// 生成随机点
std::vector<TestPoint> generateRandomPoints(int count, double minX, double maxX, double minY, double maxY)
{
    std::vector<TestPoint> points;

    // 使用随机数生成器
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distX(minX, maxX);
    std::uniform_real_distribution<> distY(minY, maxY);

    for (int i = 0; i < count; ++i)
    {
        points.emplace_back(distX(gen), distY(gen), i + 1);
    }

    return points;
}

// 打印测试结果
void printTestResult(const std::string &testName, bool success)
{
    std::cout << testName << ": " << (success ? "PASSED" : "FAILED") << std::endl;
}

// 比较两个结果集是否相同
bool compareResults(const std::set<int> &expected, const std::set<int> &actual)
{
    if (expected.size() != actual.size())
    {
        std::cout << "  Size mismatch: expected " << expected.size()
                  << ", got " << actual.size() << std::endl;
        return false;
    }

    // 检查每个ID是否都存在
    for (int id : expected)
    {
        if (actual.find(id) == actual.end())
        {
            std::cout << "  Missing ID in results: " << id << std::endl;
            return false;
        }
    }

    // 检查是否有额外的ID
    for (int id : actual)
    {
        if (expected.find(id) == expected.end())
        {
            std::cout << "  Extra ID in results: " << id << std::endl;
            return false;
        }
    }

    return true;
}

int main()
{
    std::cout << "===== R-tree Correctness Test =====" << std::endl
              << std::endl;

    // 定义测试参数
    const int NUM_POINTS = 50000;
    const double MIN_X = 0.0, MAX_X = 100.0;
    const double MIN_Y = 0.0, MAX_Y = 100.0;

    // 创建一个2维R树，节点容量为16
    RTree::RTree rtree(2, 16);

    // 生成随机点
    std::vector<TestPoint> points = generateRandomPoints(NUM_POINTS, MIN_X, MAX_X, MIN_Y, MAX_Y);
    std::cout << "Generated " << points.size() << " random points" << std::endl;

    // 测试1: 插入测试
    std::cout << "1. Inserting points into R-tree..." << std::endl;
    auto insertStartTime = std::chrono::high_resolution_clock::now();

    for (const auto &point : points)
    {
        std::string pointName = "Point_" + std::to_string(point.id);
        double low[2] = {point.x, point.y};
        double high[2] = {point.x, point.y};
        RTree::Region region(low, high, 2);

        rtree.insert(pointName.size(), reinterpret_cast<const uint8_t *>(pointName.c_str()),
                     region, point.id);
    }

    auto insertEndTime = std::chrono::high_resolution_clock::now();
    auto insertDuration = std::chrono::duration_cast<std::chrono::microseconds>(
                              insertEndTime - insertStartTime)
                              .count();

    std::cout << "  Insertion completed in " << insertDuration << " microseconds" << std::endl;
    std::cout << "  Average per point: " << (insertDuration / NUM_POINTS) << " microseconds" << std::endl;
    std::cout << "  R-tree height: " << rtree.getHeight() << std::endl
              << std::endl;

    // 测试2: 区域查询测试
    std::cout << "2. Region query tests:" << std::endl;

    // 定义不同大小的查询区域
    struct QueryRegion
    {
        std::string name;
        double minX, minY, maxX, maxY;
    };

    std::vector<QueryRegion> queryRegions = {
        {"Small region", 20.0, 20.0, 30.0, 30.0},
        {"Medium region", 30.0, 30.0, 70.0, 70.0},
        {"Large region", 10.0, 10.0, 90.0, 90.0},
        {"Entire region", MIN_X, MIN_Y, MAX_X, MAX_Y},
        {"Empty region", -10.0, -10.0, -5.0, -5.0},
        {"Partial region", -5.0, -5.0, 5.0, 5.0}};

    for (const auto &qr : queryRegions)
    {
        std::cout << "  Testing " << qr.name << "..." << std::endl;

        // 向量中的线性搜索（暴力法）
        std::set<int> expectedResults;
        for (const auto &point : points)
        {
            if (point.isInRegion(qr.minX, qr.minY, qr.maxX, qr.maxY))
            {
                expectedResults.insert(point.id);
            }
        }

        // R树搜索
        double low[2] = {qr.minX, qr.minY};
        double high[2] = {qr.maxX, qr.maxY};
        RTree::Region queryRegion(low, high, 2);

        auto queryStartTime = std::chrono::high_resolution_clock::now();
        std::vector<RTree::Data *> queryResults = rtree.intersectionQuery(queryRegion);
        auto queryEndTime = std::chrono::high_resolution_clock::now();
        auto queryDuration = std::chrono::duration_cast<std::chrono::microseconds>(
                                 queryEndTime - queryStartTime)
                                 .count();

        // 提取结果ID
        std::set<int> actualResults;
        for (const auto *data : queryResults)
        {
            actualResults.insert(data->getIdentifier());
        }

        // 验证结果
        bool querySuccess = compareResults(expectedResults, actualResults);
        printTestResult(qr.name, querySuccess);
        std::cout << "    Found " << actualResults.size() << " results in "
                  << queryDuration << " microseconds" << std::endl;
    }

    std::cout << std::endl;

    // 测试3: 删除测试
    std::cout << "3. Delete tests:" << std::endl;

    // 删除偶数ID的点
    std::cout << "  Deleting points with even IDs..." << std::endl;
    int deleteCount = 0;
    int deleteFailCount = 0;

    auto deleteStartTime = std::chrono::high_resolution_clock::now();

    for (const auto &point : points)
    {
        if (point.id % 2 == 0)
        { // 删除偶数ID的点
            double low[2] = {point.x, point.y};
            double high[2] = {point.x, point.y};
            RTree::Region region(low, high, 2);

            if (rtree.remove(region, point.id))
            {
                deleteCount++;
            }
            else
            {
                deleteFailCount++;
            }
        }
    }

    auto deleteEndTime = std::chrono::high_resolution_clock::now();
    auto deleteDuration = std::chrono::duration_cast<std::chrono::microseconds>(
                              deleteEndTime - deleteStartTime)
                              .count();

    std::cout << "  Deletion results:" << std::endl;
    std::cout << "    Successfully deleted: " << deleteCount << " points" << std::endl;
    std::cout << "    Failed deletions: " << deleteFailCount << " points" << std::endl;
    std::cout << "    Total deletion time: " << deleteDuration << " microseconds" << std::endl;
    std::cout << "    Average per deletion: " << (deleteDuration / (deleteCount + deleteFailCount))
              << " microseconds" << std::endl;
    std::cout << "    R-tree height after deletion: " << rtree.getHeight() << std::endl
              << std::endl;

    // 测试4: 删除后的查询
    std::cout << "4. Post-deletion query tests:" << std::endl;

    for (const auto &qr : queryRegions)
    {
        std::cout << "  Testing " << qr.name << " after deletion..." << std::endl;

        // 向量中的线性搜索（暴力法）- 只考虑奇数ID（未删除的点）
        std::set<int> expectedResults;
        for (const auto &point : points)
        {
            if (point.id % 2 != 0 && point.isInRegion(qr.minX, qr.minY, qr.maxX, qr.maxY))
            {
                expectedResults.insert(point.id);
            }
        }

        // R树搜索
        double low[2] = {qr.minX, qr.minY};
        double high[2] = {qr.maxX, qr.maxY};
        RTree::Region queryRegion(low, high, 2);

        auto queryStartTime = std::chrono::high_resolution_clock::now();
        std::vector<RTree::Data *> queryResults = rtree.intersectionQuery(queryRegion);
        auto queryEndTime = std::chrono::high_resolution_clock::now();
        auto queryDuration = std::chrono::duration_cast<std::chrono::microseconds>(
                                 queryEndTime - queryStartTime)
                                 .count();

        // 提取结果ID
        std::set<int> actualResults;
        for (const auto *data : queryResults)
        {
            actualResults.insert(data->getIdentifier());
        }

        // 验证结果
        bool querySuccess = compareResults(expectedResults, actualResults);
        printTestResult(qr.name + " after deletion", querySuccess);
        std::cout << "    Found " << actualResults.size() << " results in "
                  << queryDuration << " microseconds" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "===== Correctness Test Completed =====" << std::endl;

    return 0;
}