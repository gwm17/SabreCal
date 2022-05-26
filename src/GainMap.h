#ifndef GAIN_MAP_H
#define GAIN_MAP_H

#include <string>
#include <unordered_map>
#include "CalibrationStructs.h"

namespace SabreCal {

    class GainMap
    {
    public:
        GainMap();
        GainMap(const std::string& filename);
        ~GainMap();
        
        void Init(const std::string& filename);
        
        const Parameters& GetParameters(int gchan) const;
        inline const bool IsValid() const { return m_isValid; }
        
    private:
        std::unordered_map<int, Parameters> m_map;
        Parameters m_dummyParams; //for null results
        
        bool m_isValid;
    };
}

#endif
