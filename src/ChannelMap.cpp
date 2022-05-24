#include "ChannelMap.h"
#include <fstream>

namespace SabreCal {

	ChannelMap::ChannelMap() :
		m_isValid(false)
	{
	}

	ChannelMap::ChannelMap(const std::string& mapfile) :
		m_isValid(false)
	{
		Init(mapfile);
	}

	void ChannelMap::Init(const std::string& mapfile)
	{
		m_isValid = false;

		std::ifstream input(mapfile);
		std::string junk;
		std::string rev_key;
		Channel this_channel;
		int gchan;

		if(!input.is_open())
			return;

		std::getline(input, junk);
		std::getline(input, junk);

		while(input>>gchan)
		{
			input>>this_channel.detID>>junk>>this_channel.localChannel;
			rev_key = std::to_string(this_channel.detID);
			if(junk == "SABRERING")
			{
				this_channel.isRing = true;
				this_channel.isWedge = false;
				rev_key += "ring";
			}
			else if (junk == "SABREWEDGE")
			{
				this_channel.isRing = false;
				this_channel.isWedge = true;
				rev_key += "wedge";
			}
			else
				continue;
			m_map[gchan] = this_channel;
			rev_key += std::to_string(this_channel.localChannel);
			m_revmap[rev_key] = gchan;			
		}

		m_isValid = true;
	}

	const Channel& ChannelMap::GetChannel(int gchan) const
	{
		auto iter = m_map.find(gchan);
		if(iter != m_map.end())
			return iter->second;
		else
			return m_dummy;
	}

	int ChannelMap::GetGlobalChannel(const Channel& channel) const
	{
		std::string key = std::to_string(channel.detID);
		if(channel.isRing)
			key += "ring";
		else
			key += "wedge";
		key += std::to_string(channel.localChannel);

		auto iter = m_revmap.find(key);
		if(iter != m_revmap.end())
			return iter->second;
		else
			return -1;
	}
}