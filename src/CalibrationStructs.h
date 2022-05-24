#ifndef CALIBRATION_STRUCTS_H
#define CALIBRATION_STRUCTS_H

#include <vector>
#include <string>

namespace SabreCal {

	struct Parameters
	{
		double slope = 0;
		double offset = 0;
	};

	struct GraphData
	{
		std::vector<double> x;
		std::vector<double> y;
		std::string name = "";
		std::string title = "";
	};
}

#endif