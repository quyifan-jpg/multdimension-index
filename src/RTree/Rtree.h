// RTree.h - 一个简化的R树实现

#pragma once

#include <vector>
#include <memory>
#include <cstdint>

namespace RTree
{

    // 前向声明
    class Node;
    using NodePtr = std::shared_ptr<Node>;
    using WeakNodePtr = std::weak_ptr<Node>;

    // 基本几何类定义
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
        Region(uint32_t dimension = 2);
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

    // 基本数据类型定义
    typedef int64_t id_type;

    // 数据实体类
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

    // 前向声明
    class LeafNode;
    class InternalNode;

    // R树的节点类
    class Node
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
    };

    // 叶子节点
    class LeafNode : public Node
    {
    public:
        LeafNode(uint32_t capacity = 8);
        ~LeafNode() override;

        bool isLeaf() const override;
        const Region &getMBR() const override;
        void insert(Data *data) override;
        bool remove(id_type id, const Region &mbr) override;
        std::vector<Data *> search(const Region &query) override;
        bool shouldSplit() const override;
        std::pair<Node *, Node *> split() override;

    private:
        uint32_t m_capacity;
        std::vector<Data *> m_entries;
        Region m_mbr;

        void recalculateMBR();

        friend class RTree;
        friend class InternalNode;
    };

    // 内部节点
    class InternalNode : public Node
    {
    public:
        InternalNode(uint32_t capacity = 8);
        ~InternalNode() override;

        bool isLeaf() const override;
        const Region &getMBR() const override;
        void insert(Data *data) override;
        bool remove(id_type id, const Region &mbr) override;
        std::vector<Data *> search(const Region &query) override;
        bool shouldSplit() const override;
        std::pair<Node *, Node *> split() override;

        void addChild(Node *child);

    private:
        uint32_t m_capacity;
        std::vector<Node *> m_children;
        Region m_mbr;

        void recalculateMBR();
        Node *chooseSubtree(const Region &mbr) const;

        friend class RTree;
    };

    // 简单R树主类
    class RTree
    {
    public:
        RTree(uint32_t dimension = 2, uint32_t nodeCapacity = 8);
        ~RTree();

        void insert(uint32_t dataLength, const uint8_t *pData, const Region &mbr, id_type id);
        bool remove(const Region &mbr, id_type id);

        // 查询方法 - 返回结果集而不使用访问者模式
        std::vector<Data *> intersectionQuery(const Region &query);
        std::vector<Data *> containmentQuery(const Region &query);
        std::vector<Data *> pointQuery(const Point &point);

        // 辅助方法
        uint32_t getDimension() const;
        uint32_t getNodeCapacity() const;

    private:
        Node *m_root;
        uint32_t m_dimension;
        uint32_t m_nodeCapacity;
        id_type m_nextId;

        void insertData_impl(Data *data);
        Node *findLeaf(Node *node, const Region &mbr, id_type id);
        void adjustTree(Node *node, Node *newNode = nullptr);
    };

} // namespace RTree