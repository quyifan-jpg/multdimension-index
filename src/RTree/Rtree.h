// RTree.h - 一个简化的R树实现

#pragma once

#include <vector>
#include <memory>
#include <cstdint>
#include <string>
#include <utility>
#include <algorithm>

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

    // 分裂策略接口
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
     * 线性分裂策略
     * 简单地将节点平均分成两部分
     */
    class LinearSplitStrategy : public SplitStrategy
    {
    public:
        // 添加显式默认构造函数和析构函数
        LinearSplitStrategy();
        ~LinearSplitStrategy() override;

        std::pair<std::vector<Data *>, std::vector<Data *>>
        splitLeafEntries(const std::vector<Data *> &entries, uint32_t capacity) const override;

        std::pair<std::vector<Node *>, std::vector<Node *>>
        splitInternalChildren(const std::vector<Node *> &children, uint32_t capacity) const override;

        std::string getName() const override;
    };

    /**
     * 二次分裂策略
     * 使用二次算法选择种子节点进行分裂
     */
    class QuadraticSplitStrategy : public SplitStrategy
    {
    public:
        // 添加显式默认构造函数和析构函数
        QuadraticSplitStrategy();
        ~QuadraticSplitStrategy() override;

        std::pair<std::vector<Data *>, std::vector<Data *>>
        splitLeafEntries(const std::vector<Data *> &entries, uint32_t capacity) const override;

        std::pair<std::vector<Node *>, std::vector<Node *>>
        splitInternalChildren(const std::vector<Node *> &children, uint32_t capacity) const override;

        std::string getName() const override;

    private:
        /**
         * 找出两个最适合作为分裂种子的条目
         * @param entries 数据条目集合
         * @return 包含两个种子条目索引的对
         */
        std::pair<size_t, size_t> pickSeeds(const std::vector<Data *> &entries) const;

        /**
         * 找出两个最适合作为分裂种子的节点
         * @param children 子节点集合
         * @return 包含两个种子节点索引的对
         */
        std::pair<size_t, size_t> pickSeeds(const std::vector<Node *> &children) const;

        /**
         * 计算合并两个区域后的面积增加量
         * @param r1 第一个区域
         * @param r2 第二个区域
         * @return 合并后的面积增加量
         */
        double calculateAreaEnlargement(const Region &r1, const Region &r2) const;
    };

    // R树的节点类
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

        // 设置分裂策略
        void setSplitStrategy(const SplitStrategy *strategy)
        {
            m_splitStrategy = strategy;
        }

    protected:
        const SplitStrategy *m_splitStrategy = nullptr;
    };

    // 叶子节点
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

    // 内部节点
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
        uint32_t total_entries = 0; // 追踪子树中的条目总数

        void recalculateMBR();
        Node *chooseSubtree(const Region &mbr) const;

        friend class RTree;
    };

    // 简单R树主类
    class RTree
    {
    public:
        RTree(uint32_t dimension = 2, uint32_t nodeCapacity = 8, const SplitStrategy *splitStrategy = nullptr);
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
        uint32_t getHeight() const;

        // 设置分裂策略
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