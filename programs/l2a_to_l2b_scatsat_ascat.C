//==============================================================//
// Copyright (C) 2017, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <vector>
#include "hdf5.h"
#include "hdf5_hl.h"
#include "List.h"
#include "BufferedList.h"
#include "Misc.h"
#include "ConfigList.h"
#include "L2A.h"
#include "Kp.h"
#include "ConfigSim.h"
#include "L2B.h"
#include "L2AToL2B.h"
#include "Tracking.h"
#include "Array.h"
#include "Meas.h"
#include "GMF.h"
#include "Constants.h"

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

//--------------//
// MAIN PROGRAM //
//--------------//

int main(int argc, char* argv[]) {

    const char* command = no_path(argv[0]);
    char* config_file = argv[1];

    ConfigList config_list;
    config_list.Read(config_file);
    config_list.ExitForMissingKeywords();

    ETime etime;
    etime.FromCodeB("1970-001T00:00:00.000");
    double time_base = (double)etime.GetSec() + (double)etime.GetMs()/1000;

    etime.FromCodeB(config_list.Get("REV_START_TIME"));
    double rev_start =
        (double)etime.GetSec() + (double)etime.GetMs()/1000 - time_base;

    etime.FromCodeB(config_list.Get("REV_STOP_TIME"));
    double rev_stop =
        (double)etime.GetSec() + (double)etime.GetMs()/1000 - time_base;

    char* l2b_file = config_list.Get("L2B_COMBINED_FILE");

    L2A l2a_ascat, l2a_scatsat;
    L2B l2b_scatsat, l2b_scatsat_nofilt;

    l2a_ascat.SetInputFilename(config_list.Get("L2A_ASCAT_FILE"));
    l2a_ascat.OpenForReading();
    l2a_ascat.ReadHeader();

    l2a_scatsat.SetInputFilename(config_list.Get("L2A_SCATSAT_FILE"));
    l2a_scatsat.OpenForReading();
    l2a_scatsat.ReadHeader();

    l2b_scatsat.SetInputFilename(config_list.Get("L2B_SCATSAT_FILE"));
    l2b_scatsat.OpenForReading();
    l2b_scatsat.ReadHeader();
    l2b_scatsat.ReadDataRec();
    l2b_scatsat.Close();

    l2b_scatsat_nofilt.SetInputFilename(
        config_list.Get("L2B_SCATSAT_NO_MEDFILT_FILE"));
    l2b_scatsat_nofilt.OpenForReading();
    l2b_scatsat_nofilt.ReadHeader();
    l2b_scatsat_nofilt.ReadDataRec();
    l2b_scatsat_nofilt.Close();

    int nati = l2a_ascat.header.alongTrackBins;
    int ncti = l2a_ascat.header.crossTrackBins;

    if (l2a_scatsat.header.alongTrackBins != nati || 
        l2a_scatsat.header.crossTrackBins != ncti || 
        l2b_scatsat.frame.swath.GetAlongTrackBins() != nati ||
        l2b_scatsat.frame.swath.GetCrossTrackBins() != ncti ||
        l2b_scatsat_nofilt.frame.swath.GetAlongTrackBins() != nati ||
        l2b_scatsat_nofilt.frame.swath.GetCrossTrackBins() != ncti) {
        fprintf(stderr, "Size mismatch!\n");
        exit(1);
    }

    // Read in L2A ASCAT file
    L2AFrame*** l2a_ascat_swath;
    l2a_ascat_swath = (L2AFrame***)make_array(sizeof(L2AFrame *), 2, ncti, nati);
    while(l2a_ascat.ReadDataRec()) {
        L2AFrame* this_frame = new L2AFrame();
        this_frame->CopyFrame(this_frame, &l2a_ascat.frame);
         *(*(l2a_ascat_swath + this_frame->cti) + this_frame->ati) = this_frame;
    }
    l2a_ascat.Close();

    // Read in L2A SCATSAT file
    L2AFrame*** l2a_scatsat_swath;
    l2a_scatsat_swath = (L2AFrame***)make_array(sizeof(L2AFrame *), 2, ncti, nati);
    while(l2a_scatsat.ReadDataRec()) {
        L2AFrame* this_frame = new L2AFrame();
        this_frame->CopyFrame(this_frame, &l2a_scatsat.frame);
         *(*(l2a_scatsat_swath + this_frame->cti) + this_frame->ati) = this_frame;
    }
    l2a_scatsat.Close();

    // Output arrays
    int l2b_size = ncti * nati;

    std::vector<float> row_time(nati);
    std::vector<float> ascat_time_diff(l2b_size);
    std::vector<float> ascat_lat(l2b_size), ascat_lon(l2b_size);
    std::vector<float> scatsat_lat(l2b_size), scatsat_lon(l2b_size);

    for(int ati=0; ati<nati; ++ati) {
        if(ati%100 == 0)
            fprintf(stdout, "%d of %d\n", ati, nati);

        row_time[ati] = 
            rev_start + (rev_stop-rev_start)*(double)ati/(double)nati;

        for(int cti=0; cti<ncti; ++cti) {

            int l2bidx = ati + nati*cti;

            if(l2a_ascat_swath[cti][ati]) {
                MeasList* ascat_ml = &(l2a_ascat_swath[cti][ati]->measList);

                LonLat lonlat = ascat_ml->AverageLonLat();
                ascat_lon[l2bidx] = rtd * lonlat.longitude;
                ascat_lat[l2bidx] = rtd * lonlat.latitude;

                float sum_time = 0;
                float cnts = 0;

                for(Meas* meas = ascat_ml->GetHead(); meas;
                    meas = ascat_ml->GetNext()) {
                    sum_time += meas->C;
                    cnts++;
                }
                ascat_time_diff[l2bidx] = sum_time/(float)cnts - row_time[ati];

                // wrap to [-180, 180) interval
                while(ascat_lon[l2bidx]>=180) ascat_lon[l2bidx] -= 360;
                while(ascat_lon[l2bidx]<-180) ascat_lon[l2bidx] += 360;
            }

            if(l2a_scatsat_swath[cti][ati]) {
                MeasList* scatsat_ml = &(l2a_scatsat_swath[cti][ati]->measList);
                LonLat lonlat = scatsat_ml->AverageLonLat();
                scatsat_lon[l2bidx] = rtd * lonlat.longitude;
                scatsat_lat[l2bidx] = rtd * lonlat.latitude;

                // wrap to [-180, 180) interval
                while(scatsat_lon[l2bidx]>=180) scatsat_lon[l2bidx] -= 360;
                while(scatsat_lon[l2bidx]<-180) scatsat_lon[l2bidx] += 360;
            }

        }
    }


    hid_t file_id = H5Fcreate(
        l2b_file, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

    hsize_t dims[2] = {ncti, nati};
    hsize_t dims_amb[3] = {ncti, nati, 4};

    H5LTmake_dataset(
        file_id, "ascat_time_diff", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_time_diff[0]);

    H5LTmake_dataset(
        file_id, "scatsat_lon", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_lon[0]);

    H5LTmake_dataset(
        file_id, "scatsat_lat", 2, dims, H5T_NATIVE_FLOAT,
        &scatsat_lat[0]);

    H5LTmake_dataset(
        file_id, "ascat_lon", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_lon[0]);

    H5LTmake_dataset(
        file_id, "ascat_lat", 2, dims, H5T_NATIVE_FLOAT,
        &ascat_lat[0]);


    H5Fclose(file_id);

    // free the l2a swaths
    free_array((void *)l2a_ascat_swath, 2, ncti, nati);
    free_array((void *)l2a_scatsat_swath, 2, ncti, nati);
    return(0);
}

