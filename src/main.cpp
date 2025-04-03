#include "RTree/Rtree.h"
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <random>
#include <chrono>
#include <algorithm>
#include <cmath>

// Helper function to print result summary
void printResultSummary(const std::vector<RTree::Data *> &results, int expectedCount)
{
    std::cout << "  - Found " << results.size() << " results (Expected: " << expectedCount << ")" << std::endl;

    // Check if count matches
    bool matches = (results.size() == expectedCount);
    std::cout << "  - Result count " << (matches ? "MATCHES" : "DOES NOT MATCH") << " expected count" << std::endl;

    // If there's a mismatch, provide more details
    if (!matches)
    {
        std::cout << "  - Difference: " << (static_cast<int>(results.size()) - expectedCount) << " points" << std::endl;
    }

    std::cout << std::endl;
}

// Generate a fixed grid of points (gridSize x gridSize grid)
std::vector<std::pair<double, double>> generateFixedGrid(double minX, double maxX, double minY, double maxY, int gridSize)
{
    std::vector<std::pair<double, double>> points;

    double stepX = (maxX - minX) / (gridSize - 1);
    double stepY = (maxY - minY) / (gridSize - 1);

    for (int i = 0; i < gridSize; ++i)
    {
        double y = minY + i * stepY;
        for (int j = 0; j < gridSize; ++j)
        {
            double x = minX + j * stepX;
            points.push_back({x, y});
        }
    }

    return points;
}

// Generate random data points
std::vector<std::pair<double, double>> generateRandomPoints(int count, double minX, double maxX, double minY, double maxY)
{
    std::vector<std::pair<double, double>> points;

    // Use random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distX(minX, maxX);
    std::uniform_real_distribution<> distY(minY, maxY);

    for (int i = 0; i < count; ++i)
    {
        points.push_back({distX(gen), distY(gen)});
    }

    return points;
}

// Performance test function
void performanceTest(RTree::RTree &rtree, int numQueries, double minX, double maxX, double minY, double maxY)
{
    std::cout << "===== Performance Test =====\n"
              << std::endl;

    // Generate random query regions
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distX(minX, maxX);
    std::uniform_real_distribution<> distY(minY, maxY);
    std::uniform_real_distribution<> distSize(1.0, 10.0);

    // Execute random range queries
    auto startTime = std::chrono::high_resolution_clock::now();

    int totalResults = 0;
    for (int i = 0; i < numQueries; ++i)
    {
        double centerX = distX(gen);
        double centerY = distY(gen);
        double halfWidth = distSize(gen);
        double halfHeight = distSize(gen);

        double low[2] = {centerX - halfWidth, centerY - halfHeight};
        double high[2] = {centerX + halfWidth, centerY + halfHeight};
        RTree::Region queryRegion(low, high, 2);

        auto results = rtree.intersectionQuery(queryRegion);
        totalResults += results.size();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

    std::cout << "Executed " << numQueries << " random range queries:" << std::endl;
    std::cout << "  - Total results found: " << totalResults << std::endl;
    std::cout << "  - Total execution time: " << duration.count() << " microseconds" << std::endl;
    std::cout << "  - Average per query: " << (duration.count() / numQueries) << " microseconds" << std::endl;
    std::cout << std::endl;
}

int main()
{
    std::cout << "===== R-tree Demo Program =====" << std::endl
              << std::endl;

    // Create a 2-dimensional R-tree with node capacity 32
    RTree::RTree rtree(2, 8);
    std::cout << "Created R-tree: Dimension=" << rtree.getDimension()
              << ", Node Capacity=" << rtree.getNodeCapacity() << std::endl
              << std::endl;

    // 1. Fixed Grid Test - Insert 500 points in a 22x23 grid
    std::cout << "===== Fixed Grid Test - 500+ Points =====" << std::endl;

    // Define the grid boundaries
    double gridMinX = 0.0;
    double gridMaxX = 100.0;
    double gridMinY = 0.0;
    double gridMaxY = 100.0;
    int gridSizeX = 22; // 22x23 = 506 points (slightly over 500)
    int gridSizeY = 23;

    // Generate grid points
    std::vector<std::pair<double, double>> gridPoints;

    double stepX = (gridMaxX - gridMinX) / (gridSizeX - 1);
    double stepY = (gridMaxY - gridMinY) / (gridSizeY - 1);

    for (int i = 0; i < gridSizeY; ++i)
    {
        double y = gridMinY + i * stepY;
        for (int j = 0; j < gridSizeX; ++j)
        {
            double x = gridMinX + j * stepX;
            gridPoints.push_back({x, y});
        }
    }

    std::cout << "Generated " << gridPoints.size() << " points in a " << gridSizeX << "x" << gridSizeY << " grid" << std::endl;

    // Insert grid points into R-tree
    std::cout << "Inserting grid points into R-tree..." << std::endl;
    auto insertStartTime = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < gridPoints.size(); ++i)
    {
        std::string pointName = "Grid_Point_" + std::to_string(i + 1);
        double x = gridPoints[i].first;
        double y = gridPoints[i].second;

        // Create region for point (exact point)
        double low[2] = {x, y};
        double high[2] = {x, y};
        RTree::Region region(low, high, 2);

        rtree.insert(pointName.size(), reinterpret_cast<const uint8_t *>(pointName.c_str()), region, i + 1);

        // Print progress every 100 points
        if ((i + 1) % 100 == 0)
        {
            std::cout << "  - Inserted " << (i + 1) << " points..." << std::endl;
        }
    }

    auto insertEndTime = std::chrono::high_resolution_clock::now();
    auto insertDuration = std::chrono::duration_cast<std::chrono::microseconds>(insertEndTime - insertStartTime);

    std::cout << "Successfully inserted " << gridPoints.size() << " grid points" << std::endl;
    std::cout << "  - Total insertion time: " << insertDuration.count() << " microseconds" << std::endl;
    std::cout << "  - Average per point: " << (insertDuration.count() / gridPoints.size()) << " microseconds" << std::endl
              << std::endl;

    // Query test on fixed grid with different sized regions
    std::cout << "Executing queries on the grid data:" << std::endl;

    // 1. Small region query
    std::cout << "1. Small region query:" << std::endl;
    double smallRegionLow[2] = {20.0, 20.0};
    double smallRegionHigh[2] = {30.0, 30.0};
    RTree::Region smallRegion(smallRegionLow, smallRegionHigh, 2);

    // Calculate expected points in the small region
    int expectedSmallRegionPoints = 0;
    for (const auto &point : gridPoints)
    {
        if (point.first >= smallRegionLow[0] && point.first <= smallRegionHigh[0] &&
            point.second >= smallRegionLow[1] && point.second <= smallRegionHigh[1])
        {
            expectedSmallRegionPoints++;
        }
    }

    auto smallQueryStartTime = std::chrono::high_resolution_clock::now();
    std::vector<RTree::Data *> smallRegionResults = rtree.intersectionQuery(smallRegion);
    auto smallQueryEndTime = std::chrono::high_resolution_clock::now();
    auto smallQueryDuration = std::chrono::duration_cast<std::chrono::microseconds>(smallQueryEndTime - smallQueryStartTime);

    std::cout << "  - Query time: " << smallQueryDuration.count() << " microseconds" << std::endl;
    printResultSummary(smallRegionResults, expectedSmallRegionPoints);

    // 2. Medium region query
    std::cout << "2. Medium region query:" << std::endl;
    double mediumRegionLow[2] = {30.0, 30.0};
    double mediumRegionHigh[2] = {70.0, 70.0};
    RTree::Region mediumRegion(mediumRegionLow, mediumRegionHigh, 2);

    // Calculate expected points in the medium region
    int expectedMediumRegionPoints = 0;
    for (const auto &point : gridPoints)
    {
        if (point.first >= mediumRegionLow[0] && point.first <= mediumRegionHigh[0] &&
            point.second >= mediumRegionLow[1] && point.second <= mediumRegionHigh[1])
        {
            expectedMediumRegionPoints++;
        }
    }

    auto mediumQueryStartTime = std::chrono::high_resolution_clock::now();
    std::vector<RTree::Data *> mediumRegionResults = rtree.intersectionQuery(mediumRegion);
    auto mediumQueryEndTime = std::chrono::high_resolution_clock::now();
    auto mediumQueryDuration = std::chrono::duration_cast<std::chrono::microseconds>(mediumQueryEndTime - mediumQueryStartTime);

    std::cout << "  - Query time: " << mediumQueryDuration.count() << " microseconds" << std::endl;
    printResultSummary(mediumRegionResults, expectedMediumRegionPoints);

    // 3. Large region query (most of the grid)
    std::cout << "3. Large region query:" << std::endl;
    double largeRegionLow[2] = {10.0, 10.0};
    double largeRegionHigh[2] = {90.0, 90.0};
    RTree::Region largeRegion(largeRegionLow, largeRegionHigh, 2);

    // Calculate expected points in the large region
    int expectedLargeRegionPoints = 0;
    for (const auto &point : gridPoints)
    {
        if (point.first >= largeRegionLow[0] && point.first <= largeRegionHigh[0] &&
            point.second >= largeRegionLow[1] && point.second <= largeRegionHigh[1])
        {
            expectedLargeRegionPoints++;
        }
    }

    auto largeQueryStartTime = std::chrono::high_resolution_clock::now();
    std::vector<RTree::Data *> largeRegionResults = rtree.intersectionQuery(largeRegion);
    auto largeQueryEndTime = std::chrono::high_resolution_clock::now();
    auto largeQueryDuration = std::chrono::duration_cast<std::chrono::microseconds>(largeQueryEndTime - largeQueryStartTime);

    std::cout << "  - Query time: " << largeQueryDuration.count() << " microseconds" << std::endl;
    std::cout << "  - Query time per result: " << (largeQueryDuration.count() / std::max(1, static_cast<int>(largeRegionResults.size()))) << " microseconds" << std::endl;
    printResultSummary(largeRegionResults, expectedLargeRegionPoints);

    // 4. Point query (exact point)
    std::cout << "4. Point query (exact point):" << std::endl;
    // Pick a point from the middle of the grid
    int middleIndex = gridPoints.size() / 2;
    double pointX = gridPoints[middleIndex].first;
    double pointY = gridPoints[middleIndex].second;
    double pointLow[2] = {pointX, pointY};
    double pointHigh[2] = {pointX, pointY};
    RTree::Region pointRegion(pointLow, pointHigh, 2);

    auto pointQueryStartTime = std::chrono::high_resolution_clock::now();
    std::vector<RTree::Data *> pointResults = rtree.intersectionQuery(pointRegion);
    auto pointQueryEndTime = std::chrono::high_resolution_clock::now();
    auto pointQueryDuration = std::chrono::duration_cast<std::chrono::microseconds>(pointQueryEndTime - pointQueryStartTime);

    std::cout << "  - Query time: " << pointQueryDuration.count() << " microseconds" << std::endl;
    printResultSummary(pointResults, 1); // For a point query, we expect exactly 1 result

    // 5. Performance test with fixed grid - run 500 random queries
    std::cout << "5. Running 500 random queries on the grid:" << std::endl;
    performanceTest(rtree, 500, gridMinX, gridMaxX, gridMinY, gridMaxY);

    // 6. Batch delete test
    std::cout << "===== Batch Delete Test =====" << std::endl;

    // Delete every 5th point
    int totalPoints = gridPoints.size();
    int deleteCount = 0;
    int deleteFailCount = 0;

    auto deleteStartTime = std::chrono::high_resolution_clock::now();

    for (int i = 4; i < totalPoints; i += 5) // Start with 5th point (index 4)
    {
        double x = gridPoints[i].first;
        double y = gridPoints[i].second;
        double pointLow[2] = {x, y};
        double pointHigh[2] = {x, y};
        RTree::Region region(pointLow, pointHigh, 2);

        if (rtree.remove(region, i + 1)) // ID is index+1
        {
            deleteCount++;
        }
        else
        {
            deleteFailCount++;
        }

        // Print progress every 20 deletions
        if (deleteCount % 20 == 0 && deleteCount > 0)
        {
            std::cout << "  - Deleted " << deleteCount << " points..." << std::endl;
        }
    }

    auto deleteEndTime = std::chrono::high_resolution_clock::now();
    auto deleteDuration = std::chrono::duration_cast<std::chrono::microseconds>(deleteEndTime - deleteStartTime);

    std::cout << "Deletion results:" << std::endl;
    std::cout << "  - Successfully deleted: " << deleteCount << " points" << std::endl;
    std::cout << "  - Failed deletions: " << deleteFailCount << " points" << std::endl;
    std::cout << "  - Total deletion time: " << deleteDuration.count() << " microseconds" << std::endl;
    std::cout << "  - Average per deletion: " << (deleteDuration.count() / std::max(1, deleteCount)) << " microseconds" << std::endl
              << std::endl;

    // Verify deletion with large region query
    std::cout << "Verifying deletions with large region query:" << std::endl;

    // Calculate expected points after deletion
    int expectedPointsAfterDeletion = 0;
    for (int i = 0; i < totalPoints; ++i)
    {
        if (i % 5 != 4)
        { // Not a deleted point
            const auto &point = gridPoints[i];
            if (point.first >= largeRegionLow[0] && point.first <= largeRegionHigh[0] &&
                point.second >= largeRegionLow[1] && point.second <= largeRegionHigh[1])
            {
                expectedPointsAfterDeletion++;
            }
        }
    }

    auto verifyQueryStartTime = std::chrono::high_resolution_clock::now();
    std::vector<RTree::Data *> verificationResults = rtree.intersectionQuery(largeRegion);
    auto verifyQueryEndTime = std::chrono::high_resolution_clock::now();
    auto verifyQueryDuration = std::chrono::duration_cast<std::chrono::microseconds>(verifyQueryEndTime - verifyQueryStartTime);

    std::cout << "  - Query time: " << verifyQueryDuration.count() << " microseconds" << std::endl;
    printResultSummary(verificationResults, expectedPointsAfterDeletion);

    // 7. Final performance test after deletions - run 500 random queries again
    std::cout << "7. Running 500 random queries after deletions:" << std::endl;
    performanceTest(rtree, 500, gridMinX, gridMaxX, gridMinY, gridMaxY);

    std::cout << "===== Demo Finished =====" << std::endl;
    return 0;
}