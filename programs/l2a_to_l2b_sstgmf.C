#define ANCILLARY_SST2B_FILE_KEYWORD "ANCILLARY_SST2B_FILE"

//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <vector>
#include "List.h"
#include "BufferedList.h"
#include "Misc.h"
#include "ConfigList.h"
#include "L2A.h"
#include "ConfigSim.h"
#include "L2B.h"
#include "L2AToL2B.h"
#include "Tracking.h"
#include "Array.h"
#include "Meas.h"

using std::list;
using std::map; 

//-----------//
// TEMPLATES //
//-----------//

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
main(int argc, char* argv[]) {

    const char* command = no_path(argv[0]);
    char* config_file = argv[1];

    FILE* ofp_nntrain = NULL;

    optind = 2;
    while((optind < argc) && (argv[optind][0]=='-')) {
        std::string sw = argv[optind];
        if(sw == "--out-nntrain-file" || sw == "t") {
            ofp_nntrain = fopen(argv[++optind], "w");
        } else {
            fprintf(stderr,"%s: Unknow option\n", command);
            exit(1);
        }
        ++optind;
    }

    ConfigList config_list;
    if(!config_list.Read(config_file)) {
        fprintf(
            stderr, "%s: error reading config file %s\n", command, config_file);
        exit(1);
    }

    char* anc_sst_filename = config_list.Get(ANCILLARY_SST2B_FILE_KEYWORD);
    FILE* anc_sst_ifp = fopen(anc_sst_filename, "r");
    std::vector<float> anc_sst;
    anc_sst.resize(152*3248);
    fread(&anc_sst[0], sizeof(float), 152*3248, anc_sst_ifp);
    fclose(anc_sst_ifp);

    L2A l2a;
    if(!ConfigL2A(&l2a, &config_list)) {
        fprintf(stderr, "%s: error configuring Level 2A Product\n", command);
        exit(1);
    }

    L2B l2b;
    if(!ConfigL2B(&l2b, &config_list)) {
        fprintf(stderr, "%s: error configuring Level 2B Product\n", command);
        exit(1);
    }

    Kp kp;
    if(!ConfigKp(&kp, &config_list)) {
        fprintf(stderr, "%s: error configuring Kp\n", command);
        exit(1);
    }

    L2AToL2B l2a_to_l2b;
    if(!ConfigL2AToL2B(&l2a_to_l2b, &config_list)) {
        fprintf(stderr, "%s: error configuring L2AToL2B\n", command);
        exit(1);
    }

    l2a.OpenForReading();
    l2b.OpenForWriting();

    if(!l2a.ReadHeader()) {
        fprintf(stderr, "%s: error reading Level 2A header\n", command);
        exit(1);
    }

    int along_track_bins = l2a.header.alongTrackBins;
    if(!l2b.frame.swath.Allocate(l2a.header.crossTrackBins, along_track_bins)){
        fprintf(stderr, "%s: error allocating wind swath\n", command);
        exit(1);
    }

    l2b.header.crossTrackResolution = l2a.header.crossTrackResolution;
    l2b.header.alongTrackResolution = l2a.header.alongTrackResolution;
    l2b.header.zeroIndex = l2a.header.zeroIndex;

    GMF* gmf;
    SSTGMF sst_gmf(&config_list);

    for (;;) {
        if (!l2a.ReadDataRec()) {
            switch (l2a.GetStatus()) {
            case L2A::OK:        // end of file
                break;
            case L2A::ERROR_READING_FRAME:
                fprintf(stderr, "%s: error reading Level 2A data\n", command);
                exit(1);
                break;
            case L2A::ERROR_UNKNOWN:
                fprintf(stderr, "%s: unknown error reading Level 2A data\n",
                    command);
                exit(1);
                break;
            default:
                fprintf(stderr, "%s: unknown status\n", command);
                exit(1);
            }
            break;        // done, exit do loop
        }

        int ati = l2a.frame.ati;
        int cti = l2a.frame.cti;
        int anc_idx = cti + 152*ati;
        float this_sst = anc_sst[anc_idx];
        sst_gmf.Get(this_sst, gmf);
        int retval = l2a_to_l2b.ConvertAndWrite(&l2a, gmf, &kp, &l2b);

        //--------------------------//
        // output training set data //
        //--------------------------//
        WVC* wvc0 = l2b.frame.swath.GetWVC(cti, ati);

        if(wvc0 && ofp_nntrain){

            MeasList* meas_list = &(l2a.frame.measList);
            l2a_to_l2b.PopulateOneNudgeVector(&l2b, cti, ati, meas_list);

            // compute diagnostic data
            if(wvc0->ambiguities.NodeCount() && wvc0->nudgeWV) { 
                WindVectorPlus* wvp1 = wvc0->ambiguities.GetHead();
                int nc = meas_list->NodeCount();
                int* look_idx = new int[nc];
                int nmeas[8] = {0, 0, 0, 0, 0, 0, 0, 0};
                float varest[8] = {0, 0, 0, 0, 0, 0, 0, 0};
                float meanest[8] = {0, 0, 0, 0, 0, 0, 0, 0};
                float gmf_meanest[8] = {0, 0, 0, 0, 0, 0, 0, 0};
                float sinchi_meanest[8] = {0, 0, 0, 0, 0, 0, 0, 0};
                float coschi_meanest[8] = {0, 0, 0, 0, 0, 0, 0, 0};

                Meas* meas = meas_list->GetHead();

                // loop over measurement list
                for (int c = 0; c < nc; c++) { 
                    switch (meas->measType)
                    {
                        case Meas::HH_MEAS_TYPE:
                            if (meas->scanAngle < pi / 2 ||
                                meas->scanAngle > 3 * pi / 2)
                                look_idx[c] = 0;
                            else
                                look_idx[c] = 1;
                            break;
                        case Meas::VV_MEAS_TYPE:
                            if (meas->scanAngle < pi / 2 ||
                                meas->scanAngle > 3 * pi / 2)
                                look_idx[c] = 2;
                            else
                                look_idx[c] = 3;
                            break;
                        case Meas::C_BAND_HH_MEAS_TYPE:
                            if (meas->scanAngle < pi / 2 ||
                                meas->scanAngle > 3 * pi / 2)
                                look_idx[c] = 4;
                            else
                                look_idx[c] = 5;
                            break;
                        case Meas::C_BAND_VV_MEAS_TYPE:
                            if (meas->scanAngle < pi / 2 ||
                                meas->scanAngle > 3 * pi / 2)
                                look_idx[c] = 6;
                            else
                                look_idx[c] = 7;
                            break;
                        default:
                            look_idx[c] = -1;
                            break;
                    }

                    if (look_idx[c] >= 0) {
                        varest[look_idx[c]] += meas->value*meas->value;
                        meanest[look_idx[c]] += meas->value;
                        float gmfs0;
                        float chi = wvc0->nudgeWV->dir - meas->eastAzimuth + pi;

                        gmf->GetInterpolatedValue(
                            meas->measType, meas->incidenceAngle,
                            wvc0->nudgeWV->spd, chi, &gmfs0);

                        gmf_meanest[look_idx[c]] += gmfs0;
                        sinchi_meanest[look_idx[c]] += sin(chi);
                        coschi_meanest[look_idx[c]] += cos(chi);
                        nmeas[look_idx[c]]++;
                    }
                    meas = meas_list->GetNext();
                } // end loop over measurement list

                fprintf(
                    ofp_nntrain, "%d %d %g %g", ati, cti,
                    l2a.getCrossTrackDistance(), wvc0->rainProb);

                for(int i=0;i<8;i++) {
                    meanest[i] /= nmeas[i];
                    gmf_meanest[i] /= nmeas[i];
                    sinchi_meanest[i] /= nmeas[i];
                    coschi_meanest[i] /= nmeas[i];
                    varest[i] = (
                        varest[i]-nmeas[i]*meanest[i]*meanest[i])/(nmeas[i]-1);

                    if(nmeas[i] < 1) {
                        meanest[i]=0;
                        gmf_meanest[i]=0;
                        sinchi_meanest[i]=0;
                        coschi_meanest[i]=0;
                    }

                    if(nmeas[i]<2) varest[i] = 0.1;

                    fprintf(
                        ofp_nntrain, " %d %g %g", nmeas[i], meanest[i],
                        varest[i]);
                }

                fprintf(
                    ofp_nntrain, " %g %g", wvc0->nudgeWV->spd,
                    wvc0->nudgeWV->dir*rtd);

                for(int i=0; i<8; i++) {
                    fprintf(
                        ofp_nntrain, " %g %g %g", gmf_meanest[i],
                        coschi_meanest[i], sinchi_meanest[i]);
                }

                fprintf(ofp_nntrain," %g \n", wvp1->spd);
                delete look_idx;
            } // end compute diagnostic data
        } // end nudge by hand
        /* end output training set data */
    }

    l2a_to_l2b.InitFilterAndFlush(&l2b);
    l2a.Close();
    l2b.Close();
    if(ofp_nntrain) fclose(ofp_nntrain);

    return(0);
}
