#ifndef CHANNEL_MAP_H
#define CHANNEL_MAP_H

#include <string>
#include <unordered_map>

namespace SabreCal {

	struct Channel
	{
		Channel() {}
		Channel(int id, bool ring, bool wedge, int local) :
			detID(id), isRing(ring), isWedge(wedge), localChannel(local)
		{}
		int detID = -1;
		bool isRing = false;
		bool isWedge = false;
		int localChannel = -1;
	};

	class ChannelMap
	{
	public:
		ChannelMap();
		ChannelMap(const std::string& mapfile);
		~ChannelMap();

		void Init(const std::string& mapfile);
		const Channel& GetChannel(int gchan) const;
		int GetGlobalChannel(const Channel& channel) const;

		inline const bool IsValid() const { return m_isValid; }

	private:
		bool m_isValid;
		std::unordered_map<int, Channel> m_map;
		std::unordered_map<std::string, int> m_revmap; //ReverseMap takes string detID + ring/wedge + localChannel to global channel
		Channel m_dummy; //for invalid results
	};
}

#endif