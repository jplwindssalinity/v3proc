//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    make_rain_gmf.C
//
// SYNOPSIS
//    make_rain_gmf <hdf_l2a_file> <hdf_l2b_file> <irain_file> <array_file>
//
// DESCRIPTION
//    Accumulates sum of s0 and (s0*s0) and number of samples
//    for each beam 1 m/s NCEP wind speeds 5 degree relative azimuth (chi) bins
//    chi=(NCEP wind dir - measurement direction + pi), 0.5 km*mm/hr 
//    integrated rain rate bins
//    
//    
// OPTIONS
//
// OPERANDS
//    <hdf_l2a_file>       The input HDF L2A file.
//    <hdf_l2b_file>      The input HDF L2B file.
//    <irain_file>        The integrated rain rate file.
//    <array_file>        The accumulation input/output file.
//
// EXAMPLES
//    An example of a command line is:
//      % mudh_calc $cfg l2a.203 l2b.203 203.mudh
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
// AUTHOR
//    Bryan.W.Stiles (Bryan.W.Stiles@jpl.nasa.gov)
//----------------------------------------------------------------------

//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcs_id[] =
    "@(#) $Id$";


//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<hdf_l2a_file>",
    "<hdf_l2b_file>", "<irain_file>", "<array_file>", 0 };

#define SPD_BINS  20
#define SPD_STEP  1.0
#define IRAIN_BINS 61
#define IRAIN_STEP 0.5
#define CHI_BINS (360/5)
#define CHI_STEP 5.0
#define BEAM_BINS 2  // 0=INNER,H 1=OUTER,V
unsigned int array_size=BEAM_BINS*SPD_BINS*CHI_BINS*IRAIN_BINS;
float sums0[BEAM_BINS][SPD_BINS][CHI_BINS][IRAIN_BINS];
float sumsqrs0[BEAM_BINS][SPD_BINS][CHI_BINS][IRAIN_BINS];
int counts0[BEAM_BINS][SPD_BINS][CHI_BINS][IRAIN_BINS];
float sumspd[BEAM_BINS][SPD_BINS][CHI_BINS][IRAIN_BINS];
float sumchi[BEAM_BINS][SPD_BINS][CHI_BINS][IRAIN_BINS];
float sumirr[BEAM_BINS][SPD_BINS][CHI_BINS][IRAIN_BINS];
#define AT_WIDTH 1624
#define CT_WIDTH 76
unsigned int size=76*1624;
static unsigned char   rain_rate[AT_WIDTH][CT_WIDTH];
static unsigned char   time_dif[AT_WIDTH][CT_WIDTH];
static unsigned short  integrated_rain_rate[AT_WIDTH][CT_WIDTH];

//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include "L2AHdf.h"
#include "Misc.h"
#include "ParTab.h"
#include "ArgDefs.h"
#include "ConfigList.h"
#include "PolyErrNo.h"
#include "PolyTable.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"
#include "GMF.h"
#include "ConfigSimDefs.h"
#include "L2B.h"
#include "ConfigSim.h"
#include "SeaPac.h"


//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class List<Meas>;
template class List<MeasSpot>;
template class List<OffsetList>;
template class List<long>;
template class List<OrbitState>;
template class BufferedList<OrbitState>;
template class List<EarthPosition>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class List<WindVectorPlus>;
template class List<AngleInterval>;

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING        ""
#define MAX_S0_PER_ROW   3240
#define UNUSABLE         0x0001
#define NEGATIVE         0x0004

#define UNNORMALIZE_MLE_FLAG  0    // leave the mle alone

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

void  check_status(HdfFile::StatusE status);

//------------------//
// OPTION VARIABLES //
//------------------//



//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int    argc,
    char*  argv[])
{
    //------------//
    // initialize //
    //------------//

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc < optind + 4)
        usage(command, usage_array, 1);

    const char* l2a_hdf_file = argv[optind++];
    const char* l2b_hdf_file = argv[optind++];
    const char* irain_file = argv[optind++];
    const char* array_file = argv[optind++];


    //-------------------//
    // read the l2b file //
    //-------------------//

    L2B l2b;
    l2b.SetInputFilename(l2b_hdf_file);
    if (! l2b.ReadHDF(UNNORMALIZE_MLE_FLAG))
    {
        fprintf(stderr, "%s: error reading L2B file %s\n", command,
            l2b_hdf_file);
        exit(1);
    }
    WindSwath* swath = &(l2b.frame.swath);

    //-------------------------------------//
    // hand-read the EA configuration file //
    //-------------------------------------//

    char* ea_config_filename = getenv(ENV_CONFIG_FILENAME);
    if (ea_config_filename == NULL)
    {
        fprintf(stderr, "%s: need an EA_CONFIG_FILE environment variable\n",
            command);
        exit(1);
    }
    ConfigList ea_config_list;
    if (! ea_config_list.Read(ea_config_filename))
    {
        fprintf(stderr, "%s: error reading EA configuration file %s\n",
            command, ea_config_filename);
        exit(1);
    }

    char* poly_table_string = ea_config_list.Get(POLY_TABLE_KEYWORD);
    EA_PolynomialErrorNo poly_status = EA_POLY_OK;
    PolynomialTable* polyTable = new PolynomialTable(poly_table_string,
        poly_status);
    if ( poly_status != EA_POLY_OK)
    {
        fprintf(stderr, "%s: error creating polynomial table from %s\n",
            command, poly_table_string);
        exit(1);
    }

    char* leap_second_table_string =
        ea_config_list.Get(LEAP_SECOND_TABLE_KEYWORD);
    if (Itime::CreateLeapSecTable(leap_second_table_string) == 0)
    {
        fprintf(stderr, "%s: error creating leap second table %s\n",
            command, leap_second_table_string);
        exit(1);
    }

    //------------------//
    // set the l2a file //
    //------------------//

    Parameter* row_number_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        ROW_NUMBER, UNIT_DN);
    Parameter* num_sigma0_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        NUM_SIGMA0, UNIT_DN);
    Parameter* cell_index_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        CELL_INDEX, UNIT_DN);
    Parameter* sigma0_mode_flag_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        SIGMA0_MODE_FLAG, UNIT_DN);
    Parameter* sigma0_qual_flag_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        SIGMA0_QUAL_FLAG, UNIT_DN);
    Parameter* sigma0_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        SIGMA0, UNIT_DB);
    Parameter* surface_flag_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        SURFACE_FLAG, UNIT_DN);
    Parameter* cell_lon_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        CELL_LON, UNIT_RADIANS);
    Parameter* cell_lat_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        CELL_LAT, UNIT_RADIANS);
    Parameter* cell_azimuth_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        CELL_AZIMUTH, UNIT_DEGREES);
    Parameter* cell_incidence_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        CELL_INCIDENCE, UNIT_RADIANS);
    Parameter* kp_alpha_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        KP_ALPHA, UNIT_DN);
    Parameter* kp_beta_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        KP_BETA, UNIT_DN);
    Parameter* kp_gamma_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        KP_GAMMA, UNIT_DN);
    Parameter* sigma0_attn_map_p = ParTabAccess::GetParameter(SOURCE_L2Ax,
        SIGMA0_ATTN_MAP, UNIT_DB);

    HdfFile::StatusE status = HdfFile::OK;
    L2AHdf l2a_file(l2a_hdf_file, SOURCE_L2Ax, status);
    if (status != HdfFile::OK)
    {
        fprintf(stderr, "%s: error opening L2A file %s\n", command,
            l2a_hdf_file);
        exit(1);
    }

    check_status(l2a_file.OpenParamDatasets(row_number_p));
    check_status(l2a_file.OpenParamDatasets(num_sigma0_p));
    check_status(l2a_file.OpenParamDatasets(cell_lat_p));
    check_status(l2a_file.OpenParamDatasets(cell_lon_p));
    check_status(l2a_file.OpenParamDatasets(cell_azimuth_p));
    check_status(l2a_file.OpenParamDatasets(cell_incidence_p));
    check_status(l2a_file.OpenParamDatasets(sigma0_p));
    check_status(l2a_file.OpenParamDatasets(kp_alpha_p));
    check_status(l2a_file.OpenParamDatasets(kp_beta_p));
    check_status(l2a_file.OpenParamDatasets(kp_gamma_p));
    check_status(l2a_file.OpenParamDatasets(sigma0_attn_map_p));
    check_status(l2a_file.OpenParamDatasets(sigma0_qual_flag_p));
    check_status(l2a_file.OpenParamDatasets(sigma0_mode_flag_p));
    check_status(l2a_file.OpenParamDatasets(surface_flag_p));
    check_status(l2a_file.OpenParamDatasets(cell_index_p));

    int l2a_length = l2a_file.GetDataLength();

    //----------------------------------//
    //  READ RAIN FILE                  //
    //----------------------------------//
 
    FILE* ifp = fopen(irain_file, "r");
    if (ifp == NULL)
      {
	fprintf(stderr, "%s: error opening rain file %s\n",
                command, irain_file);
            exit(1);
      }
    if (fread(rain_rate, sizeof(char), size, ifp) != size ||
	fread(time_dif,  sizeof(char), size, ifp) != size ||
	fseek(ifp, size, SEEK_CUR) == -1 ||
	fread(integrated_rain_rate, sizeof(short), size, ifp) != size)
      {
	fprintf(stderr, "%s: error reading input rain file %s\n", command,
                irain_file);
	exit(1);
      }
    fclose(ifp);

    //----------------------------------//
    //  READ ARRAY FILE                 //
    //----------------------------------//
    ifp = fopen(array_file,"r");
    if (ifp == NULL)
      {
	fprintf(stderr, "New Array File %s\n",
                array_file);
	//--------------//
	// clear arrays //
	//--------------//

	for (int b = 0; b < BEAM_BINS; b++)
	  {
	    for (int s = 0; s < SPD_BINS; s++)
	      {
		for (int c = 0; c < CHI_BINS; c++)
		  {
		    for (int r = 0; r < IRAIN_BINS; r++)
		      {
			sums0[b][s][c][r]=0.0;
			sumsqrs0[b][s][c][r]=0.0;
			counts0[b][s][c][r]=0;
			sumspd[b][s][c][r]=0.0;
			sumchi[b][s][c][r]=0.0;
			sumirr[b][s][c][r]=0.0;
		      }
		  }
	      }
	  }
      }
    else{
    if (fread(sums0, sizeof(float), array_size, ifp) != array_size ||
	fread(sumsqrs0,  sizeof(float), array_size, ifp) != array_size ||
	fread(counts0, sizeof(int), array_size, ifp) != array_size ||
	fread(sumspd, sizeof(float), array_size, ifp) != array_size ||
	fread(sumchi, sizeof(float), array_size, ifp) != array_size ||
	fread(sumirr, sizeof(float), array_size, ifp) != array_size )
      {
	fprintf(stderr, "%s: error reading input rain file %s\n", command,
                array_file);
	exit(100);
      }
      fclose(ifp);
    }


    //---------------//
    // for each cell //
    //---------------//

    short row_number = 0;
    short num_sigma0;
    float cell_lat[MAX_S0_PER_ROW];
    float cell_lon[MAX_S0_PER_ROW];
    float cell_azimuth[MAX_S0_PER_ROW];
    float cell_incidence[MAX_S0_PER_ROW];
    float sigma0[MAX_S0_PER_ROW];
    float sigma0_attn_map[MAX_S0_PER_ROW];
    float kp_alpha[MAX_S0_PER_ROW];
    float kp_beta[MAX_S0_PER_ROW];
    float kp_gamma[MAX_S0_PER_ROW];
    unsigned short sigma0_qual_flag[MAX_S0_PER_ROW];
    unsigned short sigma0_mode_flag[MAX_S0_PER_ROW];
    unsigned short surface_flag[MAX_S0_PER_ROW];
    char cell_index[MAX_S0_PER_ROW];

    MeasList  meas_list_row[CT_WIDTH];

    for (int idx = 0; idx < l2a_length; idx++)
    {
        //--------------------//
        // extract parameters //
        //--------------------//

        row_number_p->extractFunc(&l2a_file, row_number_p->sdsIDs, idx, 1,
            1, &row_number, polyTable);
        int ati = row_number - 1;
        if (ati < 0 || ati >= AT_WIDTH)
            continue;

        num_sigma0_p->extractFunc(&l2a_file, num_sigma0_p->sdsIDs, idx, 1,
            1, &num_sigma0, polyTable);

        cell_index_p->extractFunc(&l2a_file, cell_index_p->sdsIDs, idx, 1,
            1, cell_index, polyTable);

        sigma0_mode_flag_p->extractFunc(&l2a_file,
            sigma0_mode_flag_p->sdsIDs, idx, 1, 1, sigma0_mode_flag,
            polyTable);

        sigma0_qual_flag_p->extractFunc(&l2a_file,
            sigma0_qual_flag_p->sdsIDs, idx, 1, 1, sigma0_qual_flag,
            polyTable);

        sigma0_p->extractFunc(&l2a_file, sigma0_p->sdsIDs, idx, 1, 1,
            sigma0, polyTable);

        surface_flag_p->extractFunc(&l2a_file, surface_flag_p->sdsIDs, idx,
            1, 1, surface_flag, polyTable);

        cell_lon_p->extractFunc(&l2a_file, cell_lon_p->sdsIDs, idx, 1, 1,
            cell_lon, polyTable);

        cell_lat_p->extractFunc(&l2a_file, cell_lat_p->sdsIDs, idx, 1, 1,
            cell_lat, polyTable);

        cell_azimuth_p->extractFunc(&l2a_file, cell_azimuth_p->sdsIDs, idx,
            1, 1, cell_azimuth, polyTable);

        cell_incidence_p->extractFunc(&l2a_file, cell_incidence_p->sdsIDs,
            idx, 1, 1, cell_incidence, polyTable);

        kp_alpha_p->extractFunc(&l2a_file, kp_alpha_p->sdsIDs, idx, 1, 1,
            kp_alpha, polyTable);

        kp_beta_p->extractFunc(&l2a_file, kp_beta_p->sdsIDs, idx, 1, 1,
            kp_beta, polyTable);

        kp_gamma_p->extractFunc(&l2a_file, kp_gamma_p->sdsIDs, idx, 1, 1,
            kp_gamma, polyTable);

        sigma0_attn_map_p->extractFunc(&l2a_file, sigma0_attn_map_p->sdsIDs,
            idx, 1, 1, sigma0_attn_map, polyTable);

        //-----------------------//
        // assemble measurements //
        //-----------------------//

        for (int j = 0; j < num_sigma0; j++)
        {
            if (sigma0_qual_flag[j] & UNUSABLE)
                continue;

            // skip land
//            int land_flag = surface_flag[j] & 0x00000001;
            int land_flag = surface_flag[j];
            if (land_flag)
                continue;

            //--------------//
            // what the...? //
            //--------------//

            int cti = cell_index[j] - 1;
            if (cti < 0 || cti >= CT_WIDTH)
            {
                continue;
            }

            Meas* new_meas = new Meas;
            new_meas->incidenceAngle = cell_incidence[j];
            new_meas->value = sigma0[j] + sigma0_attn_map[j] /
                cos(new_meas->incidenceAngle);
            new_meas->value = pow(10.0, 0.1 * new_meas->value);
            if (sigma0_qual_flag[j] & NEGATIVE)
                new_meas->value = -(new_meas->value);
            new_meas->landFlag = (int)(surface_flag[j] & 0x00000001);
            new_meas->centroid.SetAltLonGDLat(0.0, cell_lon[j], cell_lat[j]);
            new_meas->beamIdx = (int)(sigma0_mode_flag[j] & 0x0004);
            new_meas->beamIdx >>= 2;
            if (new_meas->beamIdx == 0)
                new_meas->measType = Meas::HH_MEAS_TYPE;
            else
                new_meas->measType = Meas::VV_MEAS_TYPE;
            new_meas->eastAzimuth = (450.0 - cell_azimuth[j]) * dtr;
            if (new_meas->eastAzimuth >= two_pi)
                new_meas->eastAzimuth -= two_pi;
            new_meas->numSlices = -1;
            new_meas->A = kp_alpha[j];
            new_meas->B = kp_beta[j];
            new_meas->C = kp_gamma[j];

            // hack fore/aft info into scanAngle
            if (sigma0_mode_flag[j] & 0x0008)
                new_meas->scanAngle = 0.0;    // fore
            else
                new_meas->scanAngle = 1.0;    // aft

            if (! meas_list_row[cti].Append(new_meas))
            {
                fprintf(stderr, "%s: error appending measurement\n",
                    command);
                exit(1);
            }
        }

        //-----------------------------//
        // got everything for this row //
        //-----------------------------//

        for (int cti = 0; cti < CT_WIDTH; cti++)
        {
            MeasList* meas_list = &(meas_list_row[cti]);
            if (meas_list->NodeCount() == 0)
                continue;

            LonLat avg_lon_lat = meas_list->AverageLonLat();

            WVC* wvc = swath->GetWVC(cti, ati);
            if (wvc == NULL)
                continue;

            WindVectorPlus* wvp = wvc->nudgeWV;
            if (wvp == NULL || wvc->ambiguities.NodeCount()<=1)
                continue;

            float spd = wvp->spd;
            float dir = wvp->dir;

            //-----------------//
            // set speed idx   //
            //-----------------//

            int ispd = int(floor(spd/SPD_STEP));
            if(ispd >= SPD_BINS) continue;

            //-----------------//
            // Set irain index //
            //-----------------//
            
            // eliminate badly co-locate data
            int co_time = time_dif[ati][cti] * 2 - 180;
            if (abs(co_time) > 30)
                    integrated_rain_rate[ati][cti] = 2000;    // flag as bad
            if(integrated_rain_rate[ati][cti]>1000) continue;

            float irr=integrated_rain_rate[ati][cti] * 0.1;
            int irain;
            if(irr==0)irain=0;
            else irain=int(floor(irr/IRAIN_STEP))+1;
            if(irain >= IRAIN_BINS) continue;
         


            //------------------------------------------//
            // calculate measurement specific indices   //
            // and accumulate                           //
            //------------------------------------------//

            for (Meas* meas = meas_list->GetHead(); meas;
                meas = meas_list->GetNext())
            {
                float chi = dir - meas->eastAzimuth + pi;
		chi=chi*rtd;
		while (chi < 0.0)
		  chi += 360;
		while (chi >= 360)
		  chi -= 360;


                int ichi=int(floor(chi/CHI_STEP));
                int ibeam=meas->beamIdx;
                float s0=meas->value;
                if(ibeam<0 || ibeam >1 || ispd <0 || ispd>19 || ichi<0 ||
		   ichi>71 || irain <0 || irain>60){
		  printf("%d %d %d %d\n",ibeam,ispd,ichi,irain);
		}
                sums0[ibeam][ispd][ichi][irain]+=s0;           
		sumsqrs0[ibeam][ispd][ichi][irain]+=s0*s0;   
                counts0[ibeam][ispd][ichi][irain]++; 
                sumspd[ibeam][ispd][ichi][irain]+=spd;                  
		sumchi[ibeam][ispd][ichi][irain]+=chi;                  
		sumirr[ibeam][ispd][ichi][irain]+=irr;                  
            }

        }
        for (int cti = 0; cti < CT_WIDTH; cti++)
        {
            MeasList* meas_list = &(meas_list_row[cti]);
            meas_list->FreeContents();
        }
    }

    l2a_file.CloseParamDatasets(row_number_p);
    l2a_file.CloseParamDatasets(num_sigma0_p);
    l2a_file.CloseParamDatasets(cell_lat_p);
    l2a_file.CloseParamDatasets(cell_lon_p);
    l2a_file.CloseParamDatasets(cell_azimuth_p);
    l2a_file.CloseParamDatasets(cell_incidence_p);
    l2a_file.CloseParamDatasets(sigma0_p);
    l2a_file.CloseParamDatasets(sigma0_qual_flag_p);
    l2a_file.CloseParamDatasets(sigma0_mode_flag_p);
    l2a_file.CloseParamDatasets(surface_flag_p);
    l2a_file.CloseParamDatasets(cell_index_p);
    l2a_file.CloseParamDatasets(sigma0_attn_map_p);

    //--------------//
    // write arrays //
    //--------------//

    FILE* ofp=fopen(array_file,"w");
    if(ofp==NULL){
      fprintf(stderr,"%s:Array file corrupted\n",command);
      exit(100);
    }
    if( fwrite(sums0, sizeof(float), array_size, ofp) !=array_size ||
	fwrite(sumsqrs0, sizeof(float), array_size, ofp) !=array_size ||
	fwrite(counts0, sizeof(int), array_size, ofp) !=array_size ||
	fwrite(sumspd, sizeof(float), array_size, ofp) !=array_size ||
	fwrite(sumchi, sizeof(float), array_size, ofp) !=array_size ||
	fwrite(sumirr, sizeof(float), array_size, ofp) !=array_size)
      {
	fprintf(stderr,"%s:Array file corrupted\n",command);
	exit(100);
      }
    fclose(ofp);
					  
    return (0);
}

//--------------//
// check_status //
//--------------//

void
check_status(
    HdfFile::StatusE  status)
{
    if (status != HdfFile::OK)
    {
        fprintf(stderr,
            "Error opening dataset.  No, I don't know which one.\n");
        exit(1);
    }
    return;
}

