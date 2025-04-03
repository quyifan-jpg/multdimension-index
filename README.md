# Multidimensional Index Project

This project implements the R-tree data structure and its variants for efficient indexing of multidimensional spatial data. The project includes three different node splitting strategies: Linear Split, Quadratic Split, and R*-tree Split.

## Background to read
R-trees: A Dynamic Index Structure for Spatial Searching

## Features

- Implementation of multiple R-tree variants (Linear, Quadratic, and R*-tree splitting strategies)
- Support for point data insertion and query operations
- Includes experimental functionality for Range Queries and K-Nearest Neighbor (KNN) Queries
- Provides a uniform distribution data generator
- Optional integration with [libspatialindex](https://libspatialindex.org/en/latest/) for robust spatial indexing methods

## Build Instructions

### Requirements

- CMake (version 3.10 or higher)
- C++14 compatible compiler
- libspatialindex (optional but recommended)

### Installing libspatialindex

The project can use libspatialindex, a C++ library that provides robust spatial indexing methods including R*-trees with various splitting algorithms.

- **Debian/Ubuntu**: `sudo apt-get install libspatialindex-dev`
- **Fedora/RHEL**: `sudo dnf install libspatialindex-devel`
- **macOS with Homebrew**: `brew install spatialindex`
- **Conda**: `conda install -c conda-forge libspatialindex`
- **From source**: Visit [libspatialindex website](https://libspatialindex.org/en/latest/) for download and build instructions

### Build Steps

1. Clone the repository:

```bash
git clone <repository-url>
cd multdimension-index
```

2. Create and enter the build directory:

```bash
mkdir build
cd build
```

3. Configure and build the project:

```bash
cmake ..
make
```

4. Run the program:

```bash
./rtree_app
```

## Project Structure

- `src/` - Source code directory
  - `Point.h` - Point data structure definition
  - `RTree.h/.cpp` - R-tree and its variants implementation
  - `DataGenerator.h/.cpp` - Test data generator
  - `Experiment.h/.cpp` - Experiment design and result analysis
  - `main.cpp` - Program entry point

## Usage Example

By default, the program generates 1000 uniformly distributed two-dimensional data points and builds R-trees using three different splitting strategies. It then performs range query and KNN query experiments to compare the performance of different strategies.

If libspatialindex is available, the program can leverage its robust implementation for more efficient spatial queries and operations.

## Future Improvements

- Add more splitting strategies
- Support for higher-dimensional data
- Add more complex query types
- Implement parallel processing
- Enhanced visualization of spatial index performance 