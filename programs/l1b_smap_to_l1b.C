//==============================================================//
// Copyright (C) 2015, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#define L1B_S0_LORES_ASC_FILE_KEYWORD "L1B_S0_LORES_ASC_FILE"
#define L1B_S0_LORES_DEC_FILE_KEYWORD "L1B_S0_LORES_DEC_FILE"
#define L1C_S0_HIRES_ASC_FILE_KEYWORD "L1C_S0_HIRES_ASC_FILE"
#define L1C_S0_HIRES_DEC_FILE_KEYWORD "L1C_S0_HIRES_DEC_FILE"
#define L1B_TB_LORES_ASC_FILE_KEYWORD "L1B_TB_LORES_ASC_FILE"
#define L1B_TB_LORES_DEC_FILE_KEYWORD "L1B_TB_LORES_DEC_FILE"

//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "Misc.h"
#include "Ephemeris.h"
#include "ConfigList.h"
#include "L1A.h"
#include "ConfigSim.h"
#include "L1AToL1B.h"
#include "Tracking.h"
#include "QscatConfig.h"
#include "List.h"
#include "BufferedList.h"
#include "SeaPac.h"
#include "AttenMap.h"
#include "OS2XFix.h"
/* hdf5 include */
#include "hdf5.h"
#include "hdf5_hl.h"


//-----------//
// TEMPLATES //
//-----------//

class AngleInterval;

template class List<AngleInterval>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<MeasSpot>;
template class List<WindVectorPlus>;
template class List<off_t>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class std::list<string>;
template class std::map<string,string,Options::ltstr>;

//-----------//
// CONSTANTS //
//-----------//

const char* usage_array[] = {"config_file", NULL};

int main(int argc, char* argv[]){
    //-----------//
    // variables //
    //-----------//
    const char* command = NULL;

    char* config_file = NULL;
    int do_footprint = 0;

    command = no_path(argv[0]);

    //------------------------//
    // parse the command line //
    //------------------------//
    while ((optind < argc) && (argv[optind][0]=='-')) {
        std::string sw = argv[optind];
        if( sw == "-c" ) {
            config_file = argv[++optind];
        } else if ( sw == "-fp" ) {
            do_footprint = 1;
        } else {
            fprintf(stderr,"%s: Unknown option\n", command);
            exit(1);
        }
        ++optind;
    }

    if(config_file == NULL) {
        fprintf(stderr,"%s: Must specify -c config_file\n", command );
        exit(1);
    }

    ConfigList config_list;
    if(!config_list.Read(config_file)) {
        fprintf(
            stderr, "%s: error reading config file %s\n", command, config_file);
        exit(1);
    }

    //----------------------------------//
    // check for config file parameters //
    //----------------------------------//
    char* l1b_s0_input_files[2] = {NULL, NULL};
    char* l1b_tb_input_files[2] = {NULL, NULL};
    char* l1c_s0_input_files[2] = {NULL, NULL};

    // These ones are required
    config_list.ExitForMissingKeywords();
    l1b_s0_input_files[0] = config_list.Get(L1B_S0_LORES_ASC_FILE_KEYWORD);
    l1b_s0_input_files[1] = config_list.Get(L1B_S0_LORES_DEC_FILE_KEYWORD);

    // Not required
    config_list.DoNothingForMissingKeywords();
    l1b_tb_input_files[0] = config_list.Get(L1B_TB_LORES_ASC_FILE_KEYWORD);
    l1b_tb_input_files[1] = config_list.Get(L1B_TB_LORES_DEC_FILE_KEYWORD);
    l1c_s0_input_files[0] = config_list.Get(L1C_S0_HIRES_ASC_FILE_KEYWORD);
    l1c_s0_input_files[1] = config_list.Get(L1C_S0_HIRES_DEC_FILE_KEYWORD);




    return(0);
}



