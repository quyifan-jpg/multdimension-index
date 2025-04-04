#include "RStarSplitStrategy.h"

#include "src/RTree/impl/node/Node.h"

namespace RTree
{
    // Utility class for R*-tree split
    class RStarSplitEntry
    {
    public:
        const Region *m_pRegion;
        size_t m_index;
        uint32_t m_sortDim;

        RStarSplitEntry(const Region *pRegion, size_t index, uint32_t sortDim)
            : m_pRegion(pRegion), m_index(index), m_sortDim(sortDim) {}

        // Compare entries by lower bound of a dimension
        static int compareLow(const void *a, const void *b)
        {
            RStarSplitEntry *pA = *static_cast<RStarSplitEntry **>(const_cast<void *>(a));
            RStarSplitEntry *pB = *static_cast<RStarSplitEntry **>(const_cast<void *>(b));

            double aLow = pA->m_pRegion->getLow(pA->m_sortDim);
            double bLow = pB->m_pRegion->getLow(pB->m_sortDim);

            if (aLow < bLow)
                return -1;
            if (aLow > bLow)
                return 1;
            return 0;
        }

        // Compare entries by upper bound of a dimension
        static int compareHigh(const void *a, const void *b)
        {
            RStarSplitEntry *pA = *static_cast<RStarSplitEntry **>(const_cast<void *>(a));
            RStarSplitEntry *pB = *static_cast<RStarSplitEntry **>(const_cast<void *>(b));

            double aHigh = pA->m_pRegion->getHigh(pA->m_sortDim);
            double bHigh = pB->m_pRegion->getHigh(pB->m_sortDim);

            if (aHigh < bHigh)
                return -1;
            if (aHigh > bHigh)
                return 1;
            return 0;
        }
    };

    // Add explicit default constructor and destructor definitions
    RStarSplitStrategy::RStarSplitStrategy() = default;
    RStarSplitStrategy::~RStarSplitStrategy() = default;

    std::pair<std::vector<Data *>, std::vector<Data *>>
    RStarSplitStrategy::splitLeafEntries(const std::vector<Data *> &entries, uint32_t capacity) const
    {
        std::vector<Data *> group1;
        std::vector<Data *> group2;

        // Get number of entries
        size_t numEntries = entries.size();
        if (numEntries <= 2)
        {
            if (numEntries == 2)
            {
                group1.push_back(entries[0]);
                group2.push_back(entries[1]);
            }
            else if (numEntries == 1)
            {
                group1.push_back(entries[0]);
            }
            return {group1, group2};
        }

        // Calculate distribution factor (usually around 0.4)
        double splitDistributionFactor = 0.4;
        uint32_t nodeSPF = static_cast<uint32_t>(std::floor(numEntries * splitDistributionFactor));
        uint32_t splitDistribution = numEntries - (2 * nodeSPF) + 2;

        // Create arrays for sorting by dimensions
        RStarSplitEntry **dataLow = nullptr;
        RStarSplitEntry **dataHigh = nullptr;

        try
        {
            dataLow = new RStarSplitEntry *[numEntries];
            dataHigh = new RStarSplitEntry *[numEntries];
        }
        catch (...)
        {
            delete[] dataLow;
            throw;
        }

        // Initialize entries
        for (size_t i = 0; i < numEntries; ++i)
        {
            try
            {
                dataLow[i] = new RStarSplitEntry(&entries[i]->getRegion(), i, 0);
            }
            catch (...)
            {
                for (size_t j = 0; j < i; ++j)
                {
                    delete dataLow[j];
                }
                delete[] dataLow;
                delete[] dataHigh;
                throw;
            }
            dataHigh[i] = dataLow[i];
        }

        // Find the best split axis and distribution
        double minimumMargin = std::numeric_limits<double>::max();
        uint32_t splitAxis = 0;
        uint32_t sortOrder = 0;
        uint32_t splitPoint = 0;

        // Get dimension from first entry
        uint32_t dimension = entries[0]->getRegion().getDimension();

        // Choose the split axis by minimizing the sum of perimeters
        for (uint32_t dim = 0; dim < dimension; ++dim)
        {
            // Set sort dimension
            for (size_t i = 0; i < numEntries; ++i)
            {
                dataLow[i]->m_sortDim = dim;
            }

            // Sort by lower and upper bounds
            std::qsort(dataLow, numEntries, sizeof(RStarSplitEntry *), RStarSplitEntry::compareLow);
            std::qsort(dataHigh, numEntries, sizeof(RStarSplitEntry *), RStarSplitEntry::compareHigh);

            // Calculate margin sum for all possible distributions
            double marginLow = 0.0;
            double marginHigh = 0.0;

            for (uint32_t k = 1; k <= splitDistribution; ++k)
            {
                uint32_t l = nodeSPF - 1 + k;

                // Create temporary regions for margin calculation
                Region regionLow1(dimension);
                Region regionLow2(dimension);
                Region regionHigh1(dimension);
                Region regionHigh2(dimension);

                // Initialize with first entries
                regionLow1 = entries[dataLow[0]->m_index]->getRegion();
                regionHigh1 = entries[dataHigh[0]->m_index]->getRegion();

                // Combine regions for first group
                for (size_t i = 1; i < l; ++i)
                {
                    regionLow1.combine(entries[dataLow[i]->m_index]->getRegion());
                    regionHigh1.combine(entries[dataHigh[i]->m_index]->getRegion());
                }

                // Initialize second group regions
                regionLow2 = entries[dataLow[l]->m_index]->getRegion();
                regionHigh2 = entries[dataHigh[l]->m_index]->getRegion();

                // Combine regions for second group
                for (size_t i = l + 1; i < numEntries; ++i)
                {
                    regionLow2.combine(entries[dataLow[i]->m_index]->getRegion());
                    regionHigh2.combine(entries[dataHigh[i]->m_index]->getRegion());
                }

                // Sum up margins
                marginLow += regionLow1.getMargin() + regionLow2.getMargin();
                marginHigh += regionHigh1.getMargin() + regionHigh2.getMargin();
            }

            // Keep the dimension with minimum margin
            double margin = std::min(marginLow, marginHigh);
            if (margin < minimumMargin)
            {
                minimumMargin = margin;
                splitAxis = dim;
                sortOrder = (marginLow < marginHigh) ? 0 : 1;
            }
        }

        // Sort entries by the best axis found
        for (size_t i = 0; i < numEntries; ++i)
        {
            dataLow[i]->m_sortDim = splitAxis;
        }

        // Sort using the best method (by lower or upper bound)
        std::qsort(
            dataLow,
            numEntries,
            sizeof(RStarSplitEntry *),
            (sortOrder == 0) ? RStarSplitEntry::compareLow : RStarSplitEntry::compareHigh);

        // Now choose the best distribution by minimizing overlap
        double minOverlap = std::numeric_limits<double>::max();
        double minArea = std::numeric_limits<double>::max();

        for (uint32_t k = 1; k <= splitDistribution; ++k)
        {
            uint32_t l = nodeSPF - 1 + k;

            // Create regions for the two groups
            Region region1(dimension);
            Region region2(dimension);

            // Initialize with first entry
            region1 = entries[dataLow[0]->m_index]->getRegion();

            // Combine regions for first group
            for (size_t i = 1; i < l; ++i)
            {
                region1.combine(entries[dataLow[i]->m_index]->getRegion());
            }

            // Initialize second group region
            region2 = entries[dataLow[l]->m_index]->getRegion();

            // Combine regions for second group
            for (size_t i = l + 1; i < numEntries; ++i)
            {
                region2.combine(entries[dataLow[i]->m_index]->getRegion());
            }

            // Calculate overlap area
            double overlap = calculateOverlapArea(region1, region2, dimension);

            // Calculate total area
            double area = region1.getArea() + region2.getArea();

            // Update best split point if this is better
            if (overlap < minOverlap || (overlap == minOverlap && area < minArea))
            {
                splitPoint = k;
                minOverlap = overlap;
                minArea = area;
            }
        }

        // Get final split point
        uint32_t l = nodeSPF - 1 + splitPoint;

        // Assign entries to groups
        for (size_t i = 0; i < l; ++i)
        {
            group1.push_back(entries[dataLow[i]->m_index]);
        }

        for (size_t i = l; i < numEntries; ++i)
        {
            group2.push_back(entries[dataLow[i]->m_index]);
        }

        // Clean up
        for (size_t i = 0; i < numEntries; ++i)
        {
            delete dataLow[i];
        }
        delete[] dataLow;
        delete[] dataHigh;

        return {group1, group2};
    }

    std::pair<std::vector<Node *>, std::vector<Node *>>
    RStarSplitStrategy::splitInternalChildren(const std::vector<Node *> &children, uint32_t capacity) const
    {
        std::vector<Node *> group1;
        std::vector<Node *> group2;

        // Get number of children
        size_t numChildren = children.size();
        if (numChildren <= 2)
        {
            if (numChildren == 2)
            {
                group1.push_back(children[0]);
                group2.push_back(children[1]);
            }
            else if (numChildren == 1)
            {
                group1.push_back(children[0]);
            }
            return {group1, group2};
        }

        // Calculate distribution factor (usually around 0.4)
        double splitDistributionFactor = 0.4;
        uint32_t nodeSPF = static_cast<uint32_t>(std::floor(numChildren * splitDistributionFactor));
        uint32_t splitDistribution = numChildren - (2 * nodeSPF) + 2;

        // Create arrays for sorting by dimensions
        RStarSplitEntry **dataLow = nullptr;
        RStarSplitEntry **dataHigh = nullptr;

        try
        {
            dataLow = new RStarSplitEntry *[numChildren];
            dataHigh = new RStarSplitEntry *[numChildren];
        }
        catch (...)
        {
            delete[] dataLow;
            throw;
        }

        // Initialize entries
        for (size_t i = 0; i < numChildren; ++i)
        {
            try
            {
                dataLow[i] = new RStarSplitEntry(&children[i]->getMBR(), i, 0);
            }
            catch (...)
            {
                for (size_t j = 0; j < i; ++j)
                {
                    delete dataLow[j];
                }
                delete[] dataLow;
                delete[] dataHigh;
                throw;
            }
            dataHigh[i] = dataLow[i];
        }

        // Find the best split axis and distribution
        double minimumMargin = std::numeric_limits<double>::max();
        uint32_t splitAxis = 0;
        uint32_t sortOrder = 0;
        uint32_t splitPoint = 0;

        // Get dimension from first child
        uint32_t dimension = children[0]->getMBR().getDimension();

        // Choose the split axis by minimizing the sum of perimeters
        for (uint32_t dim = 0; dim < dimension; ++dim)
        {
            // Set sort dimension
            for (size_t i = 0; i < numChildren; ++i)
            {
                dataLow[i]->m_sortDim = dim;
            }

            // Sort by lower and upper bounds
            std::qsort(dataLow, numChildren, sizeof(RStarSplitEntry *), RStarSplitEntry::compareLow);
            std::qsort(dataHigh, numChildren, sizeof(RStarSplitEntry *), RStarSplitEntry::compareHigh);

            // Calculate margin sum for all possible distributions
            double marginLow = 0.0;
            double marginHigh = 0.0;

            for (uint32_t k = 1; k <= splitDistribution; ++k)
            {
                uint32_t l = nodeSPF - 1 + k;

                // Create temporary regions for margin calculation
                Region regionLow1(dimension);
                Region regionLow2(dimension);
                Region regionHigh1(dimension);
                Region regionHigh2(dimension);

                // Initialize with first children
                regionLow1 = children[dataLow[0]->m_index]->getMBR();
                regionHigh1 = children[dataHigh[0]->m_index]->getMBR();

                // Combine regions for first group
                for (size_t i = 1; i < l; ++i)
                {
                    regionLow1.combine(children[dataLow[i]->m_index]->getMBR());
                    regionHigh1.combine(children[dataHigh[i]->m_index]->getMBR());
                }

                // Initialize second group regions
                regionLow2 = children[dataLow[l]->m_index]->getMBR();
                regionHigh2 = children[dataHigh[l]->m_index]->getMBR();

                // Combine regions for second group
                for (size_t i = l + 1; i < numChildren; ++i)
                {
                    regionLow2.combine(children[dataLow[i]->m_index]->getMBR());
                    regionHigh2.combine(children[dataHigh[i]->m_index]->getMBR());
                }

                // Sum up margins
                marginLow += regionLow1.getMargin() + regionLow2.getMargin();
                marginHigh += regionHigh1.getMargin() + regionHigh2.getMargin();
            }

            // Keep the dimension with minimum margin
            double margin = std::min(marginLow, marginHigh);
            if (margin < minimumMargin)
            {
                minimumMargin = margin;
                splitAxis = dim;
                sortOrder = (marginLow < marginHigh) ? 0 : 1;
            }
        }

        // Sort children by the best axis found
        for (size_t i = 0; i < numChildren; ++i)
        {
            dataLow[i]->m_sortDim = splitAxis;
        }

        // Sort using the best method (by lower or upper bound)
        std::qsort(
            dataLow,
            numChildren,
            sizeof(RStarSplitEntry *),
            (sortOrder == 0) ? RStarSplitEntry::compareLow : RStarSplitEntry::compareHigh);

        // Now choose the best distribution by minimizing overlap
        double minOverlap = std::numeric_limits<double>::max();
        double minArea = std::numeric_limits<double>::max();

        for (uint32_t k = 1; k <= splitDistribution; ++k)
        {
            uint32_t l = nodeSPF - 1 + k;

            // Create regions for the two groups
            Region region1(dimension);
            Region region2(dimension);

            // Initialize with first child
            region1 = children[dataLow[0]->m_index]->getMBR();

            // Combine regions for first group
            for (size_t i = 1; i < l; ++i)
            {
                region1.combine(children[dataLow[i]->m_index]->getMBR());
            }

            // Initialize second group region
            region2 = children[dataLow[l]->m_index]->getMBR();

            // Combine regions for second group
            for (size_t i = l + 1; i < numChildren; ++i)
            {
                region2.combine(children[dataLow[i]->m_index]->getMBR());
            }

            // Calculate overlap area
            double overlap = calculateOverlapArea(region1, region2, dimension);

            // Calculate total area
            double area = region1.getArea() + region2.getArea();

            // Update best split point if this is better
            if (overlap < minOverlap || (overlap == minOverlap && area < minArea))
            {
                splitPoint = k;
                minOverlap = overlap;
                minArea = area;
            }
        }

        // Get final split point
        uint32_t l = nodeSPF - 1 + splitPoint;

        // Assign children to groups
        for (size_t i = 0; i < l; ++i)
        {
            group1.push_back(children[dataLow[i]->m_index]);
        }

        for (size_t i = l; i < numChildren; ++i)
        {
            group2.push_back(children[dataLow[i]->m_index]);
        }

        // Clean up
        for (size_t i = 0; i < numChildren; ++i)
        {
            delete dataLow[i];
        }
        delete[] dataLow;
        delete[] dataHigh;

        return {group1, group2};
    }

    std::string RStarSplitStrategy::getName() const
    {
        return "RStarSplit";
    }

    // Add this method to the RStarSplitStrategy class in the private section to calculate overlap area
    double RStarSplitStrategy::calculateOverlapArea(const Region &region1, const Region &region2, uint32_t dimension) const
    {
        if (!region1.intersects(region2))
            return 0.0;

        double overlap = 1.0;

        for (uint32_t d = 0; d < dimension; ++d)
        {
            double overlapLow = std::max(region1.getLow(d), region2.getLow(d));
            double overlapHigh = std::min(region1.getHigh(d), region2.getHigh(d));

            if (overlapLow >= overlapHigh)
                return 0.0;

            overlap *= (overlapHigh - overlapLow);
        }

        return overlap;
    }
} // namespace RTree