#ifndef DATA_ORGANIZER_H
#define DATA_ORGANIZER_H

#include "ChannelMap.h"
#include "GainMap.h"
#include "CalibrationMap.h"

#include "CalDict/DataStructs.h"

#include <string>

namespace SabreCal {
    
    struct AnalysisStats
    {
        uint64_t totalEvents = 0;
        uint64_t totalSabreHits = 0;
        uint64_t goodSabreHits = 0; //Above threshold, good number for matching
        uint64_t pairsMade = 0;
        uint64_t sabreRingFrags = 0;
        uint64_t sabreWedgeFrags = 0;
        uint64_t sabreBelowThreshold = 0;
    };

    struct SabreTemp
    {
        SabreTemp(double e, double t, int g, bool u) :
            energy(e), time(t), gchan(g), used(u)
        {}
        double energy;
        double time;
        int gchan;
        bool used = false;
    };

    class DataOrganizer
    {
    public:
        DataOrganizer(const std::string& channelfile, const std::string& gainfile, const std::string& calfile);
        ~DataOrganizer();
        
        void Run(const std::string& inputdata, const std::string& outputdata);
        void ShowStats();
        
    private:
        ChannelMap m_channelMap;
        GainMap m_gainMap;
        CalibrationMap m_calMap;
        
        AnalysisStats m_stats;
        
        static constexpr double s_sabreMatchCond = 0.2; //Ring/wedge %diff
        static constexpr double s_sabreThreshold = 0.2; //MeV
        static constexpr double s_sabreTimeCond = 100.0; //ns
    };
}

#endif
