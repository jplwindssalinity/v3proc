//==============================================================//
// Copyright (C) 2015, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//
#define QS_LANDMAP_FILE_KEYWORD "QS_LANDMAP_FILE"
#define QS_ICEMAP_FILE_KEYWORD "QS_ICEMAP_FILE"
#define DO_COASTAL_PROCESSING_KEYWORD "DO_COASTAL_PROCESSING"

//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
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

using std::list;
using std::map;

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

const char* useage = "%s config_file l2a-flagged.dat";

int main(int argc, char* argv[]) {

    const char* command  = no_path(argv[0]);

    if(argc < 2) {
        fprintf(stderr, useage, command);
        exit(1);
    }

    char* config_file = argv[1];
    char* l2a_flagged_file = argv[2];

    ConfigList config_list;
    if (!config_list.Read(config_file)) {
        fprintf(stderr, "%s: error reading sim config file %s\n", command,
                config_file);
        exit(1);
    }

    QSLandMap qs_landmap;
    char* qslandmap_file = config_list.Get(QS_LANDMAP_FILE_KEYWORD);
    qs_landmap.Read(qslandmap_file);

    QSIceMap qs_icemap;
    char* qsicemap_file = config_list.Get(QS_ICEMAP_FILE_KEYWORD);
    qs_icemap.Read(qsicemap_file);

    char* l2a_file = config_list.Get(L2A_FILE_KEYWORD);

    L2A l2a;
    l2a.OpenForReading(l2a_file);
    l2a.OpenForWriting(l2a_flagged_file);
    l2a.ReadHeader();
    l2a.WriteHeader();

    while(l2a.ReadDataRec()) {
    // if all sigma0 obs are land or ice; skip output of WVC

        MeasList* ml = &(l2a.frame.measList);

        int num_ocean_sigma0 = 0;
        Meas* meas = ml->GetHead();
        while(meas) {
            double alt, lon, lat;
            meas->centroid.GetAltLonGDLat(&alt, &lon, &lat);

            // Transform from the A, B, C kp convention to the alpha beta gamma
            // set numSlices == -1 to indicate.
            if(meas->numSlices > 0) {

                // KPR for SMAP; WAG: 10*log10(1+0.07) = 0.3 dB
                double kpr_eu = 0.07;

                double kp_alpha = (1+kpr_eu*kpr_eu)*(1+meas->A);

                meas->A = kp_alpha;
                meas->B = 0;
                meas->C = 0;

                // Set numSlices == -1 to indicate to GMF::GetVariance()
                // that the A, B, C are the alpha beta gamma KPs.
                meas->numSlices = -1;
            }

            // Set land / ice flags on centroid of slice composites
            meas->landFlag = 0;
            if( qs_landmap.IsLand(lon, lat, 1))
                meas->landFlag += 1;

            if(qs_icemap.IsIce(lon, lat, meas->beamIdx))
                meas->landFlag += 2;

            if(meas->landFlag == 0)
                num_ocean_sigma0 += 1;

            // composite SNR
            double snr = meas->value * meas->XK / meas->EnSlice;

            // Skip land/ice and low snr composites.
            int skip_composite = 0;
            if(10*log10(fabs(snr)) < -20)
                skip_composite = 1;

            // Remove composite if skip_composite != 0
            if(skip_composite) {
                meas = ml->RemoveCurrent();
                delete meas;
                meas = ml->GetCurrent();
            } else {
                meas = ml->GetNext();
            }
        }

        // Write out this WVC if it contains any ocean sigma0
        if(num_ocean_sigma0 > 0)
            l2a.WriteDataRec();
    }

    // Close and quit
    l2a.CloseInputFile();
    l2a.CloseOutputFile();
    return(0);
} 