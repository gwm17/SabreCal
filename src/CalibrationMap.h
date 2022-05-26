#ifndef CALIBRATION_MAP_H
#define CALIBRATION_MAP_H

#include <string>
#include <unordered_map>
#include "CalibrationStructs.h"

namespace SabreCal {

    class CalibrationMap
    {
    public:
        CalibrationMap();
        CalibrationMap(const std::string& filename);
        ~CalibrationMap();
        
        void Init(const std::string& filename);
        
        const Parameters& GetParameters(int gchan) const;
        inline const bool IsValid() const { return m_isValid; }
        
    private:
        bool m_isValid;
        std::unordered_map<int, Parameters> m_map;
        Parameters m_dummyParams; //for null result
    };
}

#endif
