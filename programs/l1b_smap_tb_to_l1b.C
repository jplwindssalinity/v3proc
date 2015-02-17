//==============================================================//
// Copyright (C) 2015, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#define L1B_TB_LORES_ASC_FILE_KEYWORD "L1B_TB_LORES_ASC_FILE"
#define L1B_TB_LORES_DEC_FILE_KEYWORD "L1B_TB_LORES_DEC_FILE"
#define SURFACE_TEMP_ANC_FILE_KEYWORD "SURFACE_TEMP_ANC_FILE"

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

int determine_l1b_sizes(char* l1b_tbfiles[], int nframes[], int nfootprints[]){

    for(int ipart=0; ipart<2; ++ipart) {
        hid_t id = H5Fopen(l1b_tbfiles[ipart], H5F_ACC_RDONLY, H5P_DEFAULT);
        if(id<0) return(0);

        hsize_t dims[2];
        H5T_class_t class_id;
        size_t type_size;

        if(H5LTget_dataset_info(id, "/Brightness_Temperature/tb_h",
            dims, &class_id, &type_size))
            return(0);

        nframes[ipart] = dims[0];
        nfootprints[ipart] = dims[1];
    }
    return(1);
}

int main(int argc, char* argv[]){
    //-----------//
    // variables //
    //-----------//
    const char* command = no_path(argv[0]);

    char* config_file = argv[1];

    ConfigList config_list;
    if(!config_list.Read(config_file)) {
        fprintf(
            stderr, "%s: error reading config file %s\n", command, config_file);
        exit(1);
    }

    char* l1b_tbfiles[2] = {NULL, NULL};

    // These ones are required
    config_list.ExitForMissingKeywords();
    l1b_tbfiles[0] = config_list.Get(L1B_TB_LORES_ASC_FILE_KEYWORD);
    l1b_tbfiles[1] = config_list.Get(L1B_TB_LORES_DEC_FILE_KEYWORD);
    printf("l1b_tbfiles: %s %s\n", l1b_tbfiles[0], l1b_tbfiles[1]);

    // Determine number of frames in both portions of orbit
    int nframes[2] = {0, 0};
    int nfootprints[2] = {0, 0};

    if(!determine_l1b_sizes(l1b_tbfiles, nframes, nfootprints)) {
        fprintf(stderr, "Unable to determine array sizes.");
        exit(1);
    }

    // Iterate over ascending / decending portions of orbit
    for(int ipart = 0; ipart < 2; ++ipart){
        hid_t id = H5Fopen(l1b_tbfiles[ipart], H5F_ACC_RDONLY, H5P_DEFAULT);
        // do stuff... 
        // meas->value = emissivity == (TB/Tphys)
        // meas->EnSlice = TB*KB
        H5Fclose(id);
    }

    return(0);
}










