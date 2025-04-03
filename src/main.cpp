#include "RTree/Rtree.h"
#include "test/RTreeTest.h"
#include <iostream>

int main()
{
    std::cout << "===== R-tree Demo Program =====" << std::endl
              << std::endl;

    // 运行R树测试
    RTreeTest::runAllTests();

    return 0;
}