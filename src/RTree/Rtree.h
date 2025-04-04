// RTree.h - A simplified R-tree implementation

#pragma once

#include <vector>
#include <memory>
#include <cstdint>
#include <string>
#include <utility>
#include <algorithm>

namespace RTree
{

    // Forward declarations
    class Node;
    using NodePtr = std::shared_ptr<Node>;
    using WeakNodePtr = std::weak_ptr<Node>;

    // Basic geometry class definitions
    class Point
    {
    public:
        Point(uint32_t dimension = 2);
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

    class Region
    {
    public:
        Region(uint32_t m_dimension = 2);
        Region(const double *low, const double *high, uint32_t dimension);
        Region(const Point &low, const Point &high);
        Region(const Region &other);
        ~Region();

        Region &operator=(const Region &other);
        bool operator==(const Region &other) const;

        bool intersects(const Region &other) const;
        bool contains(const Region &other) const;
        bool contains(const Point &point) const;

        double getArea() const;
        double getMargin() const;
        double getMinDistance(const Point &point) const;
        double getMinDistance(const Region &other) const;

        void combine(const Region &other);
        void combine(const Point &point);
        void getCombinedRegion(Region &out, const Region &other) const;

        double getLow(uint32_t index) const;
        double getHigh(uint32_t index) const;
        uint32_t getDimension() const;

    private:
        uint32_t m_dimension;
        double *m_pLow;
        double *m_pHigh;
    };

    // Basic data type definitions
    typedef int64_t id_type;

    // Data entity class
    class Data
    {
    public:
        Data(uint32_t len, const uint8_t *pData, const Region &mbr, id_type id);
        ~Data();

        Data *clone() const;
        id_type getIdentifier() const;
        const Region &getRegion() const;
        void getData(uint32_t &len, uint8_t **data) const;

    private:
        id_type m_id;
        Region m_region;
        uint8_t *m_pData;
        uint32_t m_dataLength;
    };

    // Forward declarations
    class LeafNode;
    class InternalNode;

    // Split strategy interface
    class SplitStrategy
    {
    public:
        virtual ~SplitStrategy() = default;

        virtual std::pair<std::vector<Data *>, std::vector<Data *>>
        splitLeafEntries(const std::vector<Data *> &entries, uint32_t capacity) const = 0;
        virtual std::pair<std::vector<Node *>, std::vector<Node *>>
        splitInternalChildren(const std::vector<Node *> &children, uint32_t capacity) const = 0;
        virtual std::string getName() const = 0;
    };

    /**
     * Linear split strategy
     * Simply divides the node into two equal parts
     */
    class LinearSplitStrategy : public SplitStrategy
    {
    public:
        // Add explicit default constructor and destructor
        LinearSplitStrategy();
        ~LinearSplitStrategy() override;

        std::pair<std::vector<Data *>, std::vector<Data *>>
        splitLeafEntries(const std::vector<Data *> &entries, uint32_t capacity) const override;

        std::pair<std::vector<Node *>, std::vector<Node *>>
        splitInternalChildren(const std::vector<Node *> &children, uint32_t capacity) const override;

        std::string getName() const override;
    };

    class QuadraticSplitStrategy : public SplitStrategy
    {
    public:
        // Add explicit default constructor and destructor
        QuadraticSplitStrategy();
        ~QuadraticSplitStrategy() override;

        std::pair<std::vector<Data *>, std::vector<Data *>>
        splitLeafEntries(const std::vector<Data *> &entries, uint32_t capacity) const override;

        std::pair<std::vector<Node *>, std::vector<Node *>>
        splitInternalChildren(const std::vector<Node *> &children, uint32_t capacity) const override;

        std::string getName() const override;

    private:
        std::pair<size_t, size_t> pickSeeds(const std::vector<Data *> &entries) const;
        std::pair<size_t, size_t> pickSeeds(const std::vector<Node *> &children) const;
        double calculateAreaEnlargement(const Region &r1, const Region &r2) const;
    };

    // R*-tree split strategy
    class RStarSplitStrategy : public SplitStrategy
    {
    public:
        // Add explicit default constructor and destructor
        RStarSplitStrategy();
        ~RStarSplitStrategy() override;

        std::pair<std::vector<Data *>, std::vector<Data *>>
        splitLeafEntries(const std::vector<Data *> &entries, uint32_t capacity) const override;

        std::pair<std::vector<Node *>, std::vector<Node *>>
        splitInternalChildren(const std::vector<Node *> &children, uint32_t capacity) const override;

        std::string getName() const override;

    private:
        // Helper method to calculate overlap area between two regions
        double calculateOverlapArea(const Region &region1, const Region &region2, uint32_t dimension) const;
    };

    // R-tree node class
    class Node : public std::enable_shared_from_this<Node>
    {
    public:
        virtual ~Node() = default;
        virtual bool isLeaf() const = 0;
        virtual const Region &getMBR() const = 0;
        virtual void insert(Data *data) = 0;
        virtual bool remove(id_type id, const Region &mbr) = 0;
        virtual std::vector<Data *> search(const Region &query) = 0;
        virtual bool shouldSplit() const = 0;
        virtual std::pair<Node *, Node *> split() = 0;
        virtual uint32_t getHeight() const = 0;
        virtual Node *chooseSubtree(const Region &mbr)
        {
            return this;
        };

        // Set split strategy
        void setSplitStrategy(const SplitStrategy *strategy)
        {
            m_splitStrategy = strategy;
        }

    protected:
        const SplitStrategy *m_splitStrategy = nullptr;
    };

    // Leaf node
    class LeafNode : public Node
    {
    public:
        LeafNode(uint32_t capacity = 8, const SplitStrategy *splitStrategy = nullptr);
        ~LeafNode() override;

        bool isLeaf() const override;
        const Region &getMBR() const override;
        void insert(Data *data) override;
        bool remove(id_type id, const Region &mbr) override;
        std::vector<Data *> search(const Region &query) override;
        bool shouldSplit() const override;
        std::pair<Node *, Node *> split() override;
        uint32_t getHeight() const override;

    private:
        uint32_t m_capacity;
        std::vector<Data *> m_entries;
        Region m_mbr;

        void recalculateMBR();

        friend class RTree;
        friend class InternalNode;
    };

    // Internal node
    class InternalNode : public Node
    {
    public:
        InternalNode(uint32_t capacity = 8, const SplitStrategy *splitStrategy = nullptr);
        ~InternalNode() override;

        bool isLeaf() const override;
        const Region &getMBR() const override;
        void insert(Data *data) override;
        bool remove(id_type id, const Region &mbr) override;
        std::vector<Data *> search(const Region &query) override;
        bool shouldSplit() const override;
        std::pair<Node *, Node *> split() override;
        uint32_t getHeight() const override;

        void addChild(Node *child);

    private:
        uint32_t m_capacity;
        std::vector<Node *> m_children;
        Region m_mbr;
        uint32_t total_entries = 0; // Track total entries in subtree

        void recalculateMBR();
        Node *chooseSubtree(const Region &mbr) const;

        friend class RTree;
    };

    // Simple R-tree main class
    class RTree
    {
    public:
        RTree(uint32_t dimension = 2, uint32_t nodeCapacity = 8, const SplitStrategy *splitStrategy = nullptr);
        ~RTree();

        void insert(uint32_t dataLength, const uint8_t *pData, const Region &mbr, id_type id);
        bool remove(const Region &mbr, id_type id);

        // Query method - Return result set without using visitor pattern
        std::vector<Data *> intersectionQuery(const Region &query);
        std::vector<Data *> containmentQuery(const Region &query);
        std::vector<Data *> pointQuery(const Point &point);

        // Helper methods
        uint32_t getDimension() const;
        uint32_t getNodeCapacity() const;
        uint32_t getHeight() const;

        // Set split strategy
        void setSplitStrategy(const SplitStrategy *splitStrategy);

    private:
        Node *m_root_node;
        uint32_t m_dimension;
        uint32_t m_nodeCapacity;
        id_type m_nextId;
        const SplitStrategy *m_splitStrategy;

        void insertData_impl(Data *data);
        Node *findLeaf(Node *node, const Region &mbr, id_type id);
        void adjustTree(Node *node, Node *newNode = nullptr);
    };

} // namespace RTree