#include "CalibrationMap.h"
#include <fstream>

namespace SabreCal {

    CalibrationMap::CalibrationMap() :
        m_isValid(false)
    {
    }

    CalibrationMap::CalibrationMap(const std::string& filename) :
        m_isValid(false)
    {
        Init(filename);
    }

    CalibrationMap::~CalibrationMap() {}

    void CalibrationMap::Init(const std::string &filename)
    {
        std::ifstream input(filename);
        if(!input.is_open())
        {
            m_isValid = false;
            return;
        }
        
        std::string junk;
        int gchan;
        Parameters params;
        
        std::getline(input, junk);
        std::getline(input, junk);
        
        while(input>>gchan)
        {
            input>>params.slope>>params.offset;
            m_map[gchan] = params;
        }
        
        m_isValid = true;
    }

    const Parameters& CalibrationMap::GetParameters(int gchan) const
    {
        auto iter = m_map.find(gchan);
        if(iter != m_map.end())
            return iter->second;
        else
            return m_dummyParams;
    }
}
