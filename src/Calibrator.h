#ifndef CALIBRATOR_H
#define CALIBRATOR_H

#include <string>
#include <vector>
#include "ChannelMap.h"
#include "GainMap.h"
#include "CalibrationStructs.h"

#include "TH1.h"

namespace SabreCal {

    class Calibrator
    {
    public:
        Calibrator(const std::string& gainfile);
        ~Calibrator();
        
        void Run(const std::string& datafile, const std::string& outputfile, const std::string& plotfile="");
        
    private:
        void FindParameters(const std::vector<TH1F>& grams);
        void WriteHistograms(const std::vector<TH1F>& grams, const std::string& plotfile);
        void WriteParameters(const std::string& outputfile);
        
        GainMap m_gainMap;
        std::vector<Parameters> m_params;
        
        static constexpr int s_totalChannels = 128; //Total channels in SABRE
        static constexpr double s_241Am_alphaKE = 5.468; //MeV
        static constexpr double s_gainFactor = 1.5; //For if you're dumb and the electronics gain needs adjusted
    };
}

#endif /* Calibrator_hpp */
