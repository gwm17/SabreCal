#include "GainMatcher.h"
#include "CalDict/DataStructs.h"
#include "TFile.h"
#include "TTree.h"
#include "TF1.h"
#include "TFitResult.h"

#include <iostream>
#include <fstream>

namespace SabreCal {

	GainMatcher::GainMatcher(const std::string& mapfile, int ring_to_match, int wedge_to_match) :
		m_map(mapfile), m_ringToFit(ring_to_match), m_wedgeToFit(wedge_to_match)
	{
	}

	GainMatcher::~GainMatcher() {}

	void GainMatcher::Run(const std::string& datafile, const std::string& outputfile, const std::string& graphfile)
	{
		if(!m_map.IsValid())
		{
			std::cerr<<"ERR -- GainMatcher::Run() ChannelMap is invalid."<<std::endl;
			return;
		}

		TFile* input = TFile::Open(datafile.c_str(), "READ");
		if(!input->IsOpen())
		{
			std::cerr<<"ERR -- GainMatcher::Run() data file is invalid."<<std::endl;
			return;
		}

		ProcessedEvent* eventPtr = new ProcessedEvent();
		TTree* tree = (TTree*) input->Get("SPSTree");
		tree->SetBranchAddress("event", &eventPtr);

		uint64_t nentries = tree->GetEntries();
		uint64_t count=0, flushes=0;
		float flush_percent = 0.1f;
		uint64_t flush_val = flush_percent*nentries;

		//start data loop
		for(uint64_t i=0; i<nentries; i++)
		{
			tree->GetEntry(i);
			count++;
			if(count == flush_val)
			{
				count = 0;
				flushes++;
				std::cout<<"\rPercent of data processed: "<<flush_percent*flushes*100.0f<<"%"<<std::flush;
			}

			for(int j=0; j<5; j++)
			{
				SabreDetector& this_detector = eventPtr->sabreArray[j];
				for(auto& ring : this_detector.rings)
				{
					auto& ringChannel = m_map.GetChannel(ring.Ch);
					for(auto& wedge : this_detector.wedges)
					{
						auto& wedgeChannel = m_map.GetChannel(wedge.Ch);

						if(ringChannel.localChannel == m_ringToFit && ring.Long > 200.0 && wedge.Long > 200.0)
						{
							m_data[wedge.Ch].x.push_back(wedge.Long);
							m_data[wedge.Ch].y.push_back(ring.Long);
							if(m_data[wedge.Ch].name == "")
							{
								m_data[wedge.Ch].name = "channel_"+std::to_string(wedge.Ch);
								m_data[wedge.Ch].title = m_data[wedge.Ch].name + ";" + m_data[wedge.Ch].name + ";channel_"+std::to_string(ring.Ch);
							}
						}
						if(wedgeChannel.localChannel == m_wedgeToFit && ring.Long > 200.0 && wedge.Long > 200.0)
						{
							m_data[ring.Ch].x.push_back(ring.Long);
							m_data[ring.Ch].y.push_back(wedge.Long);
							if(m_data[ring.Ch].name == "")
							{
								m_data[ring.Ch].name = "channel_"+std::to_string(ring.Ch);
								m_data[ring.Ch].title = m_data[ring.Ch].name + ";" + m_data[ring.Ch].name + ";channel_"+std::to_string(wedge.Ch);
							}
						}
					}
				}
			}
		}
		//data loop complete

		std::cout<<std::endl;
		std::cout<<"Matching..."<<std::endl;
		DoMatching(graphfile);
		std::cout<<"Writing results..."<<std::endl;
		WriteParameters(outputfile);

		input->Close();
	}

	void GainMatcher::DoMatching(const std::string& graphfile)
	{
		std::vector<TGraph*> graph_array;
		graph_array.resize(m_totalChannels);

		//Make all of the wedge graphs, and get the gain match parameters
		for(int i=0; i<m_firstRing; i++)
		{
			if(m_data[i].x.size() != 0)
			{
				graph_array[i] = new TGraph(m_data[i].x.size(), &(m_data[i].x[0]), &(m_data[i].y[0]));
				graph_array[i]->SetName(m_data[i].name.c_str());
				graph_array[i]->SetTitle(m_data[i].title.c_str());
				auto result = graph_array[i]->Fit("pol1","R|ROB|Q+");
				m_params[i].slope = result->Parameter(1);
				m_params[i].offset = result->Parameter(0);
			}
			else
				graph_array[i] = nullptr;
		}

		//Now do rings, after applying parameters from wedges
		for(int i=m_firstRing; i<m_totalChannels; i++)
		{
			if(m_data[i].x.size() != 0)
			{
				auto ringchan = m_map.GetChannel(i);
				int wedge_gchan = m_map.GetGlobalChannel({ringchan.detID, false, true, m_wedgeToFit});
				auto& params = m_params[wedge_gchan];
				for(size_t j=0; j<m_data[i].y.size(); j++) //apply wedge results
					m_data[i].y[j] = params.slope*m_data[i].y[j] + params.offset;

				graph_array[i] = new TGraph(m_data[i].x.size(), &(m_data[i].x[0]), &(m_data[i].y[0]));
				graph_array[i]->SetName(m_data[i].name.c_str());
				graph_array[i]->SetTitle(m_data[i].title.c_str());
				auto result = graph_array[i]->Fit("pol1","R|ROB|Q+");
				m_params[i].slope = result->Parameter(1);
				m_params[i].offset = result->Parameter(0);				
			}
			else
				graph_array[i] = nullptr;
		}

		if(graphfile != "")
			WriteGraphs(graph_array, graphfile);

		for(size_t i=0; i<graph_array.size(); i++)
			delete graph_array[i];
	}

	void GainMatcher::WriteGraphs(const std::vector<TGraph*>& graphs, const std::string& graphfile)
	{
		TFile* graphout = TFile::Open(graphfile.c_str(), "RECREATE");
		if(!graphout->IsOpen())
			return;

		for(auto graph : graphs)
		{
			if(graph)
				graph->Write();
		}

		graphout->Close();
	}

	void GainMatcher::WriteParameters(const std::string& outputfile)
	{
		std::ofstream output(outputfile);
		if(!output.is_open())
		{
			std::cerr<<"ERR -- Unable to write results at GainMatcher::WriteParameters() with filename "<<outputfile<<std::endl;
			return;
		}

		output<<"Channel  Slope  Offset"<<std::endl;
		for(size_t i=0; i<m_params.size(); i++)
			output<<i<<"  "<<m_params[i].slope<<"  "<<m_params[i].offset<<std::endl;

		output.close();
	}

}
