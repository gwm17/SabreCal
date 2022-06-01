#include "Calibrator.h"
#include "TFile.h"
#include "TTree.h"
#include "DataStructs.h"
#include <iostream>
#include <fstream>
#include "TSpectrum.h"

namespace SabreCal {

    Calibrator::Calibrator(const std::string& gainfile) :
        m_gainMap(gainfile)
    {
        TH1::AddDirectory(kFALSE);
    }

    Calibrator::~Calibrator() {}

    void Calibrator::Run(const std::string& datafile, const std::string& outputfile, const std::string& plotfile)
    {
        if(!m_gainMap.IsValid())
        {
            std::cerr<<"ERR -- Bad gain map at Calibrator::Run!"<<std::endl;
            return;
        }
        
        TFile* input = TFile::Open(datafile.c_str(), "READ");
        if(!input->IsOpen())
        {
            std::cerr<<"ERR -- Bad data file "<<datafile<<" at Calibrator::Run!"<<std::endl;
            return;
        }
        
        TTree* tree = (TTree*) input->Get("Data");
	if(tree == nullptr)
	{
		std::cerr<<"ERR -- Unable to find tree Data at Calibrator::Run()"<<std::endl;
		return;
	}

        uint16_t energy, board, channel;
        tree->SetBranchAddress("Energy", &energy);
        tree->SetBranchAddress("Board", &board);
        tree->SetBranchAddress("Channel", &channel);
        
        int gchan;
        
        uint64_t nentries = tree->GetEntries();
        float flush_percent = 0.1f;
        uint64_t count=0, flush_count=0, flush_val=nentries*flush_percent;
        
        std::vector<TH1F> histograms;
        for(int i=0; i<s_totalChannels; i++)
        {
            std::string name = "channel"+std::to_string(i);
            histograms.emplace_back(name.c_str(), name.c_str(), 4096, 0.0, 16384.0);
        }
        
        for(uint64_t i=0; i<nentries; i++)
        {
            tree->GetEntry(i);
            count++;
            if(count == flush_val)
            {
                flush_count++;
                count = 0;
                std::cout<<"\rPercent of data processed: "<<flush_count*flush_percent*100.0<<"%"<<std::flush;
            }
            
            gchan = board*16 + channel; //Global channel (16 channels per board)
            auto& params = m_gainMap.GetParameters(gchan);
            
            if(energy > 100.0) //Reject electronic noise
                histograms[gchan].Fill(params.slope*(energy*s_gainFactor) + params.offset);
        }
        std::cout<<std::endl;
        input->Close();
        
        std::cout<<"Finding calibration parameters..."<<std::endl;
        FindParameters(histograms);
        std::cout<<"Writing to disk..."<<std::endl;
        WriteHistograms(histograms, plotfile);
        WriteParameters(outputfile);
    }

    void Calibrator::FindParameters(const std::vector<TH1F>& grams)
    {
        TSpectrum finder;
        double threshold = 0.5;
        double sigma = 5.0;
        
        Parameters these_params;
        double peakMean;
        
        for(auto& histo : grams)
        {
            finder.Search(&histo, sigma, "nobackground", threshold);
            if(finder.GetNPeaks() == 0)
            {
                std::cerr<<"No peaks found in histogram "<<histo.GetName()<<std::endl;
                m_params.push_back(Parameters());
            }
            else if(finder.GetNPeaks() > 1)
            {
                peakMean = 0.0;
                double peakAmp = 0.0;
                std::cerr<<"Multiple peaks found in histogram "<<histo.GetName();
                std::cerr<<"Keeping the largest peak."<<std::endl;
                double mean, amp;
                for(int i=0; i<finder.GetNPeaks(); i++)
                {
                    mean = finder.GetPositionX()[i];
                    amp = finder.GetPositionY()[i];
                    if(amp > peakAmp)
                    {
                        peakMean = mean;
                        peakAmp = amp;
                    }
                }
                these_params.slope = s_241Am_alphaKE/peakMean;
                these_params.offset = 0.0;
                m_params.push_back(these_params);
            }
            else
            {
                peakMean = finder.GetPositionX()[0];
                these_params.slope = s_241Am_alphaKE/peakMean;
                these_params.offset = 0.0;
                m_params.push_back(these_params);
            }
        }
    }

    void Calibrator::WriteHistograms(const std::vector<TH1F> &grams, const std::string &plotfile)
    {
        TFile* output = TFile::Open(plotfile.c_str(), "RECREATE");
        if(!output->IsOpen())
        {
            std::cerr<<"Unable to open file "<<plotfile<<" for saving histograms"<<std::endl;
            return;
        }
        
        for(auto& histo : grams)
        {
            histo.Write();
        }
        
        output->Close();
    }

    void Calibrator::WriteParameters(const std::string &outputfile)
    {
        std::ofstream output(outputfile);
        if(!output.is_open())
        {
            std::cerr<<"Unable to open file "<<outputfile<<" for saving parameters"<<std::endl;
            return;
        }
        output<<"Channel  Slope  Offset"<<std::endl;
        for(size_t i=0; i<m_params.size(); i++)
        {
            output<<i<<"  "<<m_params[i].slope<<"  "<<m_params[i].offset<<std::endl;
        }
        output.close();
    }
}
