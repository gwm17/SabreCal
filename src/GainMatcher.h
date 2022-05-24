#ifndef GAIN_MATCHER_H
#define GAIN_MATCHER_H

#include <string>
#include <array>
#include "CalibrationStructs.h"
#include "ChannelMap.h"
#include "TGraph.h"

namespace SabreCal {

	class GainMatcher
	{
	public:
		GainMatcher(const std::string& mapfile, int ring_to_match, int wedge_to_match);
		~GainMatcher();

		void Run(const std::string& datafile, const std::string& outputfile, const std::string& graphfile="");

	private:
		void DoMatching(const std::string& graphfile);
		void WriteGraphs(const std::vector<TGraph*>& graphs, const std::string& graphfile);
		void WriteParameters(const std::string& outputfile);

		ChannelMap m_map;

		static constexpr int m_totalChannels = 128;
		static constexpr int m_firstRing = 48; //Wedges are 0-47, rings 48-127
		std::array<GraphData, m_totalChannels> m_data;
		std::array<Parameters, m_totalChannels> m_params;

		int m_ringToFit;
		int m_wedgeToFit;

	};
}

#endif