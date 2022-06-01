#include "GainMatcher.h"
#include "Calibrator.h"
#include "DataOrganizer.h"

#include <string>
#include <fstream>
#include <iostream>

int main(int argc, const char** argv)
{
    std::string inputfile;
    std::string option;
    if(argc < 2)
    {
        std::cerr<<"ERR -- SabreCal requires an input configuration file."<<std::endl;
        return 1;
    }
    else if (argc == 2)
    {
        if(std::string(argv[1]) == "--help")
        {
            std::cout<<"SABRE Calibration Help"<<std::endl;
            std::cout<<"To run: ./bin/SabreCal <option> <input config>"<<std::endl;
            std::cout<<"Available Options"<<std::endl;
            std::cout<<"--gainmatch: Only run gain-matching"<<std::endl;
            std::cout<<"--calibrate: Only run calibration (requires gain-matching parameters)"<<std::endl;
            std::cout<<"--apply: Apply calibration and gain-matching parameters to a dataset (requires gain-matching and        calibration parameters)"<<std::endl;
            std::cout<<"--all: Run gain-matching, calibration, and apply to data set"<<std::endl;
            std::cout<<"Default behavior (no option) is the same as --all"<<std::endl;
            return 0;
        }
        inputfile = argv[1];
        option = "";
    }
    else
    {
        inputfile = argv[2];
        option = argv[1];
    }
    
    std::ifstream input(inputfile);
    if(!input.is_open())
    {
	std::cerr<<"ERR -- Unable to open input config file "<<inputfile<<std::endl;
	return 1;
    }

    std::string junk;
    
    std::string gaindata, caldata, inputdata, outputdata;
    std::string gainplots, calplots;
    std::string gainfile, calfile, channelfile;
    int ring_to_match, wedge_to_match;
    
    input>>junk>>gaindata;
    input>>junk>>gainplots;
    input>>junk>>caldata;
    input>>junk>>calplots;
    input>>junk>>inputdata;
    input>>junk>>outputdata;
    input>>junk>>gainfile;
    input>>junk>>calfile;
    input>>junk>>channelfile;
    input>>junk>>ring_to_match>>junk>>wedge_to_match;
    
    std::cout<<"----------SABRE Calibration----------"<<std::endl;
    std::cout<<"Gain-Matching Data: "<<gaindata<<std::endl;
    std::cout<<"Gain-Matching Plots: "<<gainplots<<std::endl;
    std::cout<<"Gain-Match Parameter file: "<<gainfile<<std::endl;
    std::cout<<"Ring-to-Match: "<<ring_to_match<<" Wedge-to-Match: "<<wedge_to_match<<std::endl;
    std::cout<<"Calibration Data: "<<caldata<<std::endl;
    std::cout<<"Calibration Plots: "<<calplots<<std::endl;
    std::cout<<"Calibration Parameter file: "<<calfile<<std::endl;
    std::cout<<"Data to Calibrate: "<<inputdata<<std::endl;
    std::cout<<"Calibration Output: "<<outputdata<<std::endl;
    std::cout<<"Channel Map: "<<channelfile<<std::endl;
    
    if(option == "--gainmatch")
    {
        std::cout<<"Calculating SABRE gain-matching parameters..."<<std::endl;
        SabreCal::GainMatcher matcher(channelfile, ring_to_match, wedge_to_match);
        if(gainplots == "None")
            matcher.Run(gaindata, gainfile);
        else
            matcher.Run(gaindata, gainfile, gainplots);
    }
    else if (option == "--calibrate")
    {
        std::cout<<"Calculating SABRE calibration parameters..."<<std::endl;
        SabreCal::Calibrator calib(gainfile);
        if(calplots == "None")
            calib.Run(caldata, calfile);
        else
            calib.Run(caldata, calfile, calplots);
    }
    else if (option == "--apply")
    {
        std::cout<<"Applying SABRE gain-matching and calibration parameters to dataset..."<<std::endl;
        SabreCal::DataOrganizer organ(channelfile, gainfile, calfile);
        organ.Run(inputdata, outputdata);
        organ.ShowStats();
    }
    else if (option == "" || option == "--all")
    {
        std::cout<<"Calculating SABRE gain-matching parameters..."<<std::endl;
        SabreCal::GainMatcher matcher(channelfile, ring_to_match, wedge_to_match);
        if(gainplots == "None")
            matcher.Run(gaindata, gainfile);
        else
            matcher.Run(gaindata, gainfile, gainplots);
        std::cout<<"Calculating SABRE calibration parameters..."<<std::endl;
        SabreCal::Calibrator calib(gainfile);
        if(calplots == "None")
            calib.Run(caldata, calfile);
        else
            calib.Run(caldata, calfile, calplots);
        std::cout<<"Applying SABRE gain-matching and calibration parameters to dataset..."<<std::endl;
        SabreCal::DataOrganizer organ(channelfile, gainfile, calfile);
        organ.Run(inputdata, outputdata);
        organ.ShowStats();
    }
    else if (option == "--help")
    {
        std::cout<<"SABRE Calibration Help"<<std::endl;
        std::cout<<"To run: ./bin/SabreCal <option> <input config>"<<std::endl;
        std::cout<<"Available Options"<<std::endl;
        std::cout<<"--gainmatch: Only run gain-matching"<<std::endl;
        std::cout<<"--calibrate: Only run calibration (requires gain-matching parameters)"<<std::endl;
        std::cout<<"--apply: Apply calibration and gain-matching parameters to a dataset (requires gain-matching and        calibration parameters)"<<std::endl;
        std::cout<<"--all: Run gain-matching, calibration, and apply to data set"<<std::endl;
        std::cout<<"Default behavior (no option) is the same as --all"<<std::endl;
        return 0;
    }
    else
    {
        std::cout<<"Unrecognized option passed to SabreCal. Use --help to see available options"<<std::endl;
        return 1;
    }
    
    std::cout<<"Finished."<<std::endl;
    return 0;
}
