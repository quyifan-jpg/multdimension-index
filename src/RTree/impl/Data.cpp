#include "Data.h"

#include <cstring>

namespace RTree
{

    Data::Data(uint32_t len, const uint8_t *pData, const Region &mbr, id_type id)
        : m_id(id), m_region(mbr), m_dataLength(len)
    {
        if (len > 0 && pData != nullptr)
        {
            m_pData = new uint8_t[len];
            memcpy(m_pData, pData, len);
        }
        else
        {
            m_pData = nullptr;
            m_dataLength = 0;
        }
    }

    Data::~Data()
    {
        delete[] m_pData;
    }

    Data *Data::clone() const
    {
        return new Data(m_dataLength, m_pData, m_region, m_id);
    }

    id_type Data::getIdentifier() const
    {
        return m_id;
    }

    const Region &Data::getRegion() const
    {
        return m_region;
    }

    void Data::getData(uint32_t &len, uint8_t **data) const
    {
        len = m_dataLength;

        if (m_dataLength > 0)
        {
            *data = new uint8_t[m_dataLength];
            memcpy(*data, m_pData, m_dataLength);
        }
        else
        {
            *data = nullptr;
        }
    }

} // namespace RTree