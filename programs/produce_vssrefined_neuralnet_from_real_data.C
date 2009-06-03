//==============================================================//
// Copyright (C) 2007, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    produce_full_scat_neuralnet.C
//
// SYNOPSIS
//    produce_full_scat_neuralnet <sim_config_file> <max_speed> <rainfilelist>
//                <geometry_input_file> <dataset_outputfile_base> <num_samples>
//
// DESCRIPTION
//    
//  Takes geometry and noise information in an input file as a function
// of cross track distance. Simulates gridded measurements
// from uniformly distributed random speed and each 5 degree set of directions.
// Outputs data set for use in training expected speed given s0 + direction
// ( one for each cross track distance and each 5 degree band of directions)
// OPTIONS
//    None.
//
// OPERANDS
//    The following operands are supported/required:
//
//      <sim_config_file>  The sim_config_file needed listing
//                         all the wind retrieval parameters
//
//      <max_speed>       maximum speed in m/s
//      <rainfilelist> list of rainfile scenes 1 for each beam (numbeams at top)
//      <geometry input file> Files contains geometry, look distribution, and
//                            SNR as a function of cross track distance
//       Format is:
//       XX columns of ASCII text with a single ASCII header line. 
//       Header line is: Number_of_Beams  grid_cell_resolution 
//       Data lines are:
//       Column 1: Cross track distance   starts at -1000 km end at +1000 
//                 in 1 km steps (for example).
//       The quantities in columns 2-8 are for Beam 1 Fore Look Measurements.
//       Column 2: Number of Beam 1 Fore Look Measurements in wind vector cell
//       Column 3: Average Noise Equivalent Sigma0 in dB 
//       Column 4: Number of Looks per measurement 
//                 (typically number of range looks averaged)
//       Column 5: Azimuth angle
//       Column 6: Incidence angle
//       Column 7: Polarization  (V or H)
//       Column 8: Signal to Ambiguity Ratio  in dB
//       Columns 9-15 are the same as 2-8 but for Beam 1 Aft Look
//       Columns 16-22 are the same as 2-8 but for Beam 2 Fore Look
//       Columns 23-29 are the same as 2-8 but for Beam 2  Aft Look
//       Further columns are necessary if there are more than 2 beams.
//
//       <dataset_outputfile_base> Base name for datasets to train MLPs
//
//       <num_samples> Number of wind cells simulated for each cross track
//                     distance and relative direction. 
//                     A larger number yields a bigger data set
//                     and longer running time.
// 
// EXAMPLES
//    An example of a command line is:
//      % produce_full_scat_neuralnet quikscat.cfg 50 rainexamples.lst quikscat_gn.dat quikscat_full_ann  10000
//
// ENVIRONMENT
//    Not environment dependent.
//
// EXIT STATUS
//    The following exit values are returned:
//       0  Program executed successfully
//      >0  Program had an error
//
// NOTES
//    None.
//
// AUTHORS
//    Bryan.W.Stiles
//----------------------------------------------------------------------

//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcs_id[] =
    "@(#) $Id$";

//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Misc.h"
#include "ConfigList.h"
#include "L2A.h"
#include "ConfigSim.h"
#include "L2B.h"
#include "L2AToL2B.h"
#include "Tracking.h"
#include "Tracking.C"
#include "MLPData.h"
#include "RainDistribution.h"
#include "GeomNoiseFile.h"

using std::list;
using std::map; 

//-----------//
// TEMPLATES //
//-----------//

// Class declarations needed for templates
// eliminates need to include the entire header file
class AngleInterval;

template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<off_t>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class List<AngleInterval>;
template class std::list<string>;
template class std::map<string,string,Options::ltstr>;

//-----------//
// CONSTANTS //
//-----------//

//#define DEBUG 
#define NMAXSAMPLES 1000000
#define NDIR 1   // should be 72 if we want MLP by direction
#define CROSSTRACKSPACING 400 // number of cross track bins per MLP 
#define USE_CTD_INPUT 2  // 0 = nothing 1=CTD 2= CTD and RELDIR 
                         
#define OMIT_VAR 0


//-------//
// HACKS //
//-------//



//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<dataset_output_filebase> (updates files in place)",0};



//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int    argc,
    char*  argv[])
{
    //------------------------//
    // parse the command line //
    //------------------------//



    const char* command = no_path(argv[0]);
    if (argc!=2)
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* out_file_base = argv[clidx++];
 
 
    char out_spd_file[200], out_rain_file[200], out_obj_file[200];
    sprintf(out_spd_file,"%s_spd.dat",out_file_base); 
    sprintf(out_rain_file,"%s_rain.dat",out_file_base); 
    sprintf(out_obj_file,"%s_obj.dat",out_file_base); 

    char net_spd_file[200], net_rain_file[200], net_obj_file[200];
    sprintf(net_spd_file,"%s_spd.net",out_file_base); 
    sprintf(net_rain_file,"%s_rain.net",out_file_base); 
    sprintf(net_obj_file,"%s_obj.net",out_file_base); 


    
    MLPDataArray spdarr(out_spd_file,net_spd_file,"r+");
    MLPDataArray objarr(out_obj_file,net_obj_file,"r+");
    // MLPDataArray rainarr(out_rain_file,net_rain_file); 

    spdarr.nepochs=100;
    spdarr.max_bad_epochs=10; 
    spdarr.vss=true;
    objarr.nepochs=100;
    objarr.max_bad_epochs=10; 
    objarr.vss=true;
    objarr.reTrain();
    spdarr.reTrain();
    return (0);
}
