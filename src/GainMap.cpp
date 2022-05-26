#include "GainMap.h"
#include <fstream>

namespace SabreCal {

    GainMap::GainMap() :
        m_isValid(false)
    {
    }

    GainMap::GainMap(const std::string& filename) :
        m_isValid(false)
    {
        Init(filename);
    }

    GainMap::~GainMap() {}

    void GainMap::Init(const std::string& filename)
    {
        std::ifstream input(filename);
        if(!input.is_open())
        {
            m_isValid = false;
            return;
        }
        std::string junk;
        int gchan;
        Parameters pars;
        
        std::getline(input, junk);
        std::getline(input, junk);
        
        while(input>>gchan)
        {
            input>>pars.slope>>pars.offset;
            m_map[gchan] = pars;
        }
        
        m_isValid = true;
    }

    const Parameters& GainMap::GetParameters(int gchan) const
    {
        auto iter = m_map.find(gchan);
        if(iter != m_map.end())
            return iter->second;
        else
            return m_dummyParams;
    }
}
