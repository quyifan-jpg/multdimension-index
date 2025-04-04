//
// Created by Shengqiao Zhao on 2025-04-04.
//

#ifndef DATA_H
#define DATA_H
#include <cstdint>

#include "common.h"
#include "Region.h"

namespace RTree {
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
}

#endif //DATA_H
