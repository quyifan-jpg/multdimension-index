// test_correctness.cpp
// Tests the correctness of R-tree implementation by comparing search results between vector and R-tree


#include <iostream>
#include <random>
#include <set>
#include <vector>
#include <__random/random_device.h>

#include "RTree/impl/strategy/LinearSplitStrategy.h"
#include "RTree/impl/strategy/QuadraticSplitStrategy.h"
#include "RTree/impl/strategy/RStarSplitStrategy.h"
#include "RTree/impl/tree/RTree.h"

// Define the structure of test data entry
struct TestPoint
{
    double x, y;
    int id;

    // Constructor
    TestPoint(double x, double y, int id) : x(x), y(y), id(id) {}

    // Determine if a point is within the query region
    bool isInRegion(double minX, double minY, double maxX, double maxY) const
    {
        return (x >= minX && x <= maxX && y >= minY && y <= maxY);
    }
};

// Generate random points
std::vector<TestPoint> generateRandomPoints(int count, double minX, double maxX, double minY, double maxY)
{
    std::vector<TestPoint> points;

    // Use random number generator
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

// Print test results
void printTestResult(const std::string &testName, bool success)
{
    std::cout << testName << ": " << (success ? "PASSED" : "FAILED") << std::endl;
}

// Compare if two result sets are the same
bool compareResults(const std::set<int> &expected, const std::set<int> &actual)
{
    if (expected.size() != actual.size())
    {
        std::cout << "  Size mismatch: expected " << expected.size()
                  << ", got " << actual.size() << std::endl;
        return false;
    }

    // Check if each ID exists
    for (int id : expected)
    {
        if (actual.find(id) == actual.end())
        {
            std::cout << "  Missing ID in results: " << id << std::endl;
            return false;
        }
    }

    // Check for extra IDs
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

    // Define test parameters
    const int NUM_POINTS = 50000;
    const double MIN_X = 0.0, MAX_X = 100.0;
    const double MIN_Y = 0.0, MAX_Y = 100.0;

    // Test statistics
    int testsPassed = 0;
    int totalTests = 0;

    // Test different split strategies
    std::cout << "Testing different split strategies:" << std::endl;

    // Linear split strategy
    RTree::LinearSplitStrategy linearSplitStrategy;
    RTree::RTree linearTree(2, 16, &linearSplitStrategy);
    std::cout << "  Created R-tree with LinearSplitStrategy" << std::endl;

    // Quadratic split strategy
    RTree::QuadraticSplitStrategy quadraticSplitStrategy;
    RTree::RTree quadraticTree(2, 16, &quadraticSplitStrategy);
    std::cout << "  Created R-tree with QuadraticSplitStrategy" << std::endl;

    // R*-tree split strategy
    RTree::RStarSplitStrategy rstarSplitStrategy;
    RTree::RTree rstarTree(2, 16, &rstarSplitStrategy);
    std::cout << "  Created R-tree with RStarSplitStrategy" << std::endl;

    // Generate random points
    std::vector<TestPoint> points = generateRandomPoints(NUM_POINTS, MIN_X, MAX_X, MIN_Y, MAX_Y);
    std::cout << "Generated " << points.size() << " random points" << std::endl;

    // Test 1: Linear Split Strategy
    std::cout << "1. Testing with Linear Split Strategy..." << std::endl;
    // Insert points
    auto insertStartTime = std::chrono::high_resolution_clock::now();
    for (const auto &point : points)
    {
        std::string pointName = "Point_" + std::to_string(point.id);
        double low[2] = {point.x, point.y};
        double high[2] = {point.x, point.y};
        RTree::Region region(low, high, 2);

        linearTree.insert(pointName.size(), reinterpret_cast<const uint8_t *>(pointName.c_str()),
                          region, point.id);
    }
    auto insertEndTime = std::chrono::high_resolution_clock::now();
    auto insertDuration = std::chrono::duration_cast<std::chrono::microseconds>(
                              insertEndTime - insertStartTime)
                              .count();
    std::cout << "  Linear Split: Insertion completed in " << insertDuration << " microseconds" << std::endl;
    std::cout << "  Linear Split: R-tree height: " << linearTree.getHeight() << std::endl
              << std::endl;

    // Test 2: Quadratic Split Strategy
    std::cout << "2. Testing with Quadratic Split Strategy..." << std::endl;
    // Insert points
    insertStartTime = std::chrono::high_resolution_clock::now();
    for (const auto &point : points)
    {
        std::string pointName = "Point_" + std::to_string(point.id);
        double low[2] = {point.x, point.y};
        double high[2] = {point.x, point.y};
        RTree::Region region(low, high, 2);

        quadraticTree.insert(pointName.size(), reinterpret_cast<const uint8_t *>(pointName.c_str()),
                             region, point.id);
    }
    insertEndTime = std::chrono::high_resolution_clock::now();
    insertDuration = std::chrono::duration_cast<std::chrono::microseconds>(
                         insertEndTime - insertStartTime)
                         .count();
    std::cout << "  Quadratic Split: Insertion completed in " << insertDuration << " microseconds" << std::endl;
    std::cout << "  Quadratic Split: R-tree height: " << quadraticTree.getHeight() << std::endl
              << std::endl;

    // Test 3: R*-tree Split Strategy
    std::cout << "3. Testing with R*-tree Split Strategy..." << std::endl;
    // Insert points
    insertStartTime = std::chrono::high_resolution_clock::now();
    for (const auto &point : points)
    {
        std::string pointName = "Point_" + std::to_string(point.id);
        double low[2] = {point.x, point.y};
        double high[2] = {point.x, point.y};
        RTree::Region region(low, high, 2);

        rstarTree.insert(pointName.size(), reinterpret_cast<const uint8_t *>(pointName.c_str()),
                         region, point.id);
    }
    insertEndTime = std::chrono::high_resolution_clock::now();
    insertDuration = std::chrono::duration_cast<std::chrono::microseconds>(
                         insertEndTime - insertStartTime)
                         .count();
    std::cout << "  R*-tree Split: Insertion completed in " << insertDuration << " microseconds" << std::endl;
    std::cout << "  R*-tree Split: R-tree height: " << rstarTree.getHeight() << std::endl
              << std::endl;

    // Define query regions for testing
    struct QueryRegion
    {
        std::string name;
        double minX, minY, maxX, maxY;
    };

    std::vector<QueryRegion> queryRegions = {
        {"Small region", 20.0, 20.0, 30.0, 30.0},
        {"Medium region", 30.0, 30.0, 70.0, 70.0},
        {"Large region", 10.0, 10.0, 90.0, 90.0}};

    // Test query performance with different strategies
    std::cout << "4. Comparing query performance:" << std::endl;

    for (const auto &qr : queryRegions)
    {
        std::cout << "  Testing " << qr.name << "..." << std::endl;

        // Prepare query region
        double low[2] = {qr.minX, qr.minY};
        double high[2] = {qr.maxX, qr.maxY};
        RTree::Region queryRegion(low, high, 2);

        // Linear split query
        auto queryStartTime = std::chrono::high_resolution_clock::now();
        std::vector<RTree::Data *> linearResults = linearTree.intersectionQuery(queryRegion);
        auto queryEndTime = std::chrono::high_resolution_clock::now();
        auto linearQueryDuration = std::chrono::duration_cast<std::chrono::microseconds>(
                                       queryEndTime - queryStartTime)
                                       .count();

        // Quadratic split query
        queryStartTime = std::chrono::high_resolution_clock::now();
        std::vector<RTree::Data *> quadraticResults = quadraticTree.intersectionQuery(queryRegion);
        queryEndTime = std::chrono::high_resolution_clock::now();
        auto quadraticQueryDuration = std::chrono::duration_cast<std::chrono::microseconds>(
                                          queryEndTime - queryStartTime)
                                          .count();

        // R*-tree split query
        queryStartTime = std::chrono::high_resolution_clock::now();
        std::vector<RTree::Data *> rstarResults = rstarTree.intersectionQuery(queryRegion);
        queryEndTime = std::chrono::high_resolution_clock::now();
        auto rstarQueryDuration = std::chrono::duration_cast<std::chrono::microseconds>(
                                      queryEndTime - queryStartTime)
                                      .count();

        std::cout << "    Linear Split: Found " << linearResults.size()
                  << " results in " << linearQueryDuration << " microseconds" << std::endl;
        std::cout << "    Quadratic Split: Found " << quadraticResults.size()
                  << " results in " << quadraticQueryDuration << " microseconds" << std::endl;
        std::cout << "    R*-tree Split: Found " << rstarResults.size()
                  << " results in " << rstarQueryDuration << " microseconds" << std::endl;
        std::cout << std::endl;
    }

    std::cout << "===== R-tree Split Strategy Comparison Completed =====" << std::endl;

    return 0;
}