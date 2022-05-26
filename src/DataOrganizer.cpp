#include "DataOrganizer.h"
#include "TFile.h"
#include "TTree.h"

#include <iostream>

namespace SabreCal {

    DataOrganizer::DataOrganizer(const std::string& channelfile, const std::string& gainfile,
                                 const std::string& calfile) :
        m_channelMap(channelfile), m_gainMap(gainfile), m_calMap(calfile)
    {
    }

    DataOrganizer::~DataOrganizer() {}

    void DataOrganizer::Run(const std::string& inputdata, const std::string& outputdata)
    {
        if(!(m_channelMap.IsValid() && m_gainMap.IsValid() && m_calMap.IsValid()))
        {
            std::cerr<<"ERR -- Invalid map files at DataOrganizer::Run()"<<std::endl;
            return;
        }
        
        TFile* input = TFile::Open(inputdata.c_str(), "READ");
        if(!input->IsOpen())
        {
            std::cerr<<"ERR -- Unable to open input data file "<<inputdata<<" at DataOrganizer::Run()"<<std::endl;
            return;
        }
        
        TTree* intree = (TTree*) input->Get("SPSTree");
        
        TFile* output = TFile::Open(outputdata.c_str(), "RECREATE");
        if(!output->IsOpen())
        {
            std::cerr<<"ERR -- Unable to open output data file "<<inputdata<<" at DataOrganizer::Run()"<<std::endl;
            return;
        }
        
        TTree* outtree = new TTree("CalTree", "CalTree");
        
        ProcessedEvent* inevent = new ProcessedEvent();
        intree->SetBranchAddress("event", &inevent);
        
        CalEvent outevent;
        outtree->Branch("event", &outevent);
        
        std::vector<SabreTemp> rings, wedges;
        double eTemp;
        double ratio;
        SabrePair this_pair;
        
        uint64_t nevents = intree->GetEntries();
        m_stats.totalEvents = nevents;
        double flush_percent = 0.05;
        uint64_t count=0, flush_count=0, flush_val=nevents*flush_percent;
        
        for(uint64_t i=0; i<nevents; i++)
        {
            intree->GetEvent(i);
            count++;
            if(count == flush_val)
            {
                flush_count++;
                count = 0;
                std::cout<<"\rPercent of data processed: "<<flush_count*flush_percent*100.0<<"%"<<std::flush;
            }
            
            //copy FP crap
            outevent.xavg = inevent->xavg;
            outevent.scintE = inevent->scintLeft;
            outevent.cathodeE = inevent->cathode;
            outevent.anodeFrontE = inevent->anodeFront;
            outevent.anodeBackE = inevent->anodeBack;
            outevent.scintT = inevent->scintLeftTime;
            
            for(int j=0; j<5; j++)
            {
                rings.clear();
                wedges.clear();
                auto& this_sabre = inevent->sabreArray[j];
                
                int64_t diff = this_sabre.rings.size() - this_sabre.wedges.size();
                m_stats.sabreRingFrags += diff > 0 ? diff : 0;
                m_stats.sabreWedgeFrags += diff < 0 ? -1*diff : 0;
                for(auto& ring : this_sabre.rings)
                {
                    m_stats.totalSabreHits++;
                    auto& ringGains = m_gainMap.GetParameters(ring.Ch);
                    auto& ringCals = m_calMap.GetParameters(ring.Ch);
                    eTemp = ringCals.slope*(ringGains.slope*ring.Long + ringGains.offset) + ringCals.offset;
                    if(eTemp > s_sabreThreshold)
                    {
                        m_stats.goodSabreHits++;
                        rings.emplace_back(eTemp, ring.Time, ring.Ch, false);
                    }
                    else
                        m_stats.sabreBelowThreshold++;
                }
                for(auto& wedge : this_sabre.wedges)
                {
                    m_stats.totalSabreHits++;
                    auto& wedgeGains = m_gainMap.GetParameters(wedge.Ch);
                    auto& wedgeCals = m_calMap.GetParameters(wedge.Ch);
                    eTemp = wedgeCals.slope*(wedgeGains.slope*wedge.Long + wedgeGains.offset) + wedgeCals.offset;
                    if(eTemp > s_sabreThreshold)
                    {
                        m_stats.goodSabreHits++;
                        wedges.emplace_back(eTemp, wedge.Time, wedge.Ch, false);
                    }
                    else
                        m_stats.sabreBelowThreshold++;
                }
                
                //Now match rings and wedges
                for(auto& ring : rings)
                {
                    for(auto& wedge : wedges)
                    {
                        if(wedge.used || ring.used)
                            continue;
                        ratio = std::abs(1.0 - ring.energy/wedge.energy);
                        if(ratio < s_sabreMatchCond)
                        {
                            auto& ringdata = m_channelMap.GetChannel(ring.gchan);
                            auto& wedgedata = m_channelMap.GetChannel(wedge.gchan);
                            this_pair.ringch = ring.gchan;
                            this_pair.wedgech = wedge.gchan;
                            this_pair.local_ring = ringdata.localChannel;
                            this_pair.local_wedge = wedgedata.localChannel;
                            this_pair.detID = wedgedata.detID;
                            this_pair.ringE = ring.energy;
                            this_pair.wedgeE = wedge.energy;
                            outevent.sabre.push_back(this_pair);
                            m_stats.pairsMade++;
                            break;
                        }
                    }
                }
            }
            outtree->Fill();
        }
        std::cout<<std::endl;
        
        input->Close();
        output->cd();
        outtree->Write(outtree->GetName(), TObject::kOverwrite);
        output->Close();
    }

    void DataOrganizer::ShowStats()
    {
        std::cout<<"Total number of events: "<<m_stats.totalEvents<<std::endl;
        std::cout<<"Total number of SABRE hits: "<<m_stats.totalSabreHits<<std::endl;
        std::cout<<"Total number of good SABRE hits: "<<m_stats.goodSabreHits<<std::endl;
        std::cout<<"Number of ring/wedge pairs made: "<<m_stats.pairsMade<<std::endl;
        std::cout<<"Matching efficiency percentage: "<<((double)m_stats.pairsMade)/m_stats.goodSabreHits<<std::endl;
        std::cout<<"Number of hits below threshold: "<<m_stats.sabreBelowThreshold<<std::endl;
        std::cout<<"Number of wedge fragments: "<<m_stats.sabreWedgeFrags<<std::endl;
        std::cout<<"Number of ring fragments: "<<m_stats.sabreRingFrags<<std::endl;
    }
}
