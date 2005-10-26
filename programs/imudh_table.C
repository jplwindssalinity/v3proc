//==============================================================//
// Copyright (C) 1999-2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    mudh_pca_table
//
// SYNOPSIS
//    mudh_pca_table [ -m mudh_dir ] [ -e enof_dir ] [ -t tb_dir ]
//        [ -i irr_dir ] [ -c time ] [ -I irr_threshold ] [ -n ] <pca_file>
//        <start_rev> <end_rev> <output_base>
//
// DESCRIPTION
//    Generates a mudh table for classification.
//
// OPTIONS
//    [ -m mudh_dir ]  Use the mudh files located in mudh_dir.
//    [ -e enof_dir ]  Use the enof files located in enof_dir.
//    [ -t tb]         Use the Tb files located in tb_dir.
//    [ -i irr_dir ]   IRR files are always used, this can specify the dir.
//    [ -c time ]      Collocation time for IRR.
//    [ -I irr_threshold ]  The threshold definition of rain.
//    [ -n ]  Use impact rather than rain rate threshold
//
// OPERANDS
//    <pca_file>     The PCA file.  If short, assumes both beam case.
//    <start_rev>    Duh.
//    <end_rev>      Duh again.
//    <output_base>  The output base.
//
// EXAMPLES
//    An example of a command line is:
//      % mudh_pca_table -c 30 pca.021100 1500 1600 1500-1600
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
//    James N. Huddleston (James.N.Huddleston@jpl.nasa.gov)
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
#include<string.h>
//#include <ieeefp.h>
/**
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
#include "Interpolate.h"
#include "SeaPac.h"
***/
#include "mudh.h"
#include "Misc.h"
#include "Index.h"
#include<math.h>
#include<stdlib.h>
#include<unistd.h>
//-----------//
// TEMPLATES //
//-----------//
/**
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
***/
//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING         "m:i:c:I:s:d:S:D:r:f"
#define BIG_DIM           100
#define QUOTE             '"'
#define REV_DIGITS        5

#define DEFAULT_MUDH_DIR  "/mnt/phobia/3x/sws2b-s/imudh"
#define DEFAULT_IMPACT_DIR "/Users/bstiles/scatterometer/windmetrics/data/kp3/impe"
#define DEFAULT_IRR_DIR "."

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -m mudh_dir ]", "[-f ]"
    "[ -i impact_dir ]", "[ -c ssmi_colocation_time ]", "[-I irr_threshold ]", 
    "[ -s speed_threshold]", "[ -d direction_threshold ]",
    "[ -S clear_speed_threshold]", "[ -D clear_direction_threshold ]",
    "[ -r ]", "<pca_file>", "<start_rev>", "<end_rev>", "<output_base>", 0 };

// first index: 0=both_beams, 1=outer_only
// second index: impact class (0=all, 1=clear, 2= impacted)
static unsigned long  counts[2][3][DIM][DIM][DIM][DIM];

static double  rain_tab[DIM][DIM][DIM][DIM];
static double  clear_tab[DIM][DIM][DIM][DIM];
static double  count_tab[DIM][DIM][DIM][DIM];

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

    int collocation_time = 30;   // default 30 minutes
    float irr_threshold = 2.0;   // default two km*mm/hr
    float spd_threshold = 2.0;   // default 2 m/s
    float dir_threshold = 10.0;  // default 10 degrees
    float clear_spd_threshold = 0.6;
    float clear_dir_threshold = 3.0;
    int use_impact = 1;
    int total_samples=0;
    int total_impacted=0;
    int total_clear=0;
    int use_fake_mudh_filenames=0;
    char* mudh_dir = DEFAULT_MUDH_DIR;
    char* irr_dir = DEFAULT_IRR_DIR;
    char* impact_dir = DEFAULT_IMPACT_DIR;
    int opt_outer_swath = 1;   // assume do outer swath too.

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);
    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'm':
            mudh_dir = optarg;
            break;
        case 'f':
	    use_fake_mudh_filenames=1;
	    break;
        case 'i':
            impact_dir = optarg;
            break;
        case 'c':
            collocation_time = atoi(optarg);
            break;
        case 'I':
            irr_threshold = atof(optarg);
            break;
        case 's':
            spd_threshold = atof(optarg);
            break;
        case 'd':
            dir_threshold = atof(optarg);
            break;
       case 'S':
            clear_spd_threshold = atof(optarg);
            break;
        case 'D':
            clear_dir_threshold = atof(optarg);
            break;
        case 'r':
	    use_impact=0;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc < optind + 4)
        usage(command, usage_array, 1);

    const char* pca_file = argv[optind++];
    int start_rev = atoi(argv[optind++]);
    int end_rev = atoi(argv[optind++]);
    const char* output_base = argv[optind++];

    //---------------//
    // read PCA file //
    //---------------//

    // first index 0=both beams, 1=outer only
    float pca_weights[2][PC_COUNT][PARAM_COUNT];
    float pca_mean[2][PARAM_COUNT];
    float pca_std[2][PARAM_COUNT];
    float pca_min[2][PC_COUNT];
    float pca_max[2][PC_COUNT];
    Index pc_index[2][PC_COUNT];

    FILE* ifp = fopen(pca_file, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "%s: error opening PCA file %s\n", command,
            pca_file);
        exit(1);
    }
    unsigned int w_size = PC_COUNT * PARAM_COUNT;
    if (fread(pca_weights[0], sizeof(float), w_size, ifp) != w_size ||
        fread(pca_mean[0], sizeof(float), PARAM_COUNT, ifp) != PARAM_COUNT ||
        fread(pca_std[0], sizeof(float), PARAM_COUNT, ifp) != PARAM_COUNT ||
        fread(pca_min[0], sizeof(float), PC_COUNT, ifp) != PC_COUNT ||
        fread(pca_max[0], sizeof(float), PC_COUNT, ifp) != PC_COUNT)
    {
        fprintf(stderr, "%s: error reading first half of PCA file %s\n",
            command, pca_file);
        exit(1);
    }
    if (fread(pca_weights[1], sizeof(float), w_size, ifp) != w_size ||
        fread(pca_mean[1], sizeof(float), PARAM_COUNT, ifp) != PARAM_COUNT ||
        fread(pca_std[1], sizeof(float), PARAM_COUNT, ifp) != PARAM_COUNT ||
        fread(pca_min[1], sizeof(float), PC_COUNT, ifp) != PC_COUNT ||
        fread(pca_max[1], sizeof(float), PC_COUNT, ifp) != PC_COUNT)
    {
        // only complain if it is NOT EOF
        if (! feof(ifp))
        {
            fprintf(stderr, "%s: error reading second half of PCA file %s\n",
                command, pca_file);
            exit(1);
        }
        printf("  Inner swath only\n");
    }
    else
    {
        opt_outer_swath = 1;
        printf("  Full swath\n");
    }
    fclose(ifp);

    // set up indices
    for (int pc_idx = 0; pc_idx < PC_COUNT; pc_idx++)
    {
        pc_index[0][pc_idx].SpecifyEdges(pca_min[0][pc_idx],
            pca_max[0][pc_idx], DIM);
        if (opt_outer_swath)
        {
            pc_index[1][pc_idx].SpecifyEdges(pca_min[1][pc_idx],
                pca_max[1][pc_idx], DIM);
        }
    }

    //----------------------//
    // determine file needs //
    //----------------------//



    //-------------------//
    // open output files //
    //-------------------//

    char filename[2048];
    sprintf(filename, "%s.imudhtab", output_base);
    FILE* pcatab_ofp = fopen(filename, "w");
    if (pcatab_ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }
    
    char filename1[2048];
    sprintf(filename1, "%s_inner.xmg", output_base);
    FILE* efp0 = fopen(filename1, "w");
    if (efp0 == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename1);
        exit(1);
    }

    char filename2[2048];
    sprintf(filename2, "%s_outer.xmg", output_base);
    FILE* efp1 = fopen(filename2, "w");
    if (efp1 == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename2);
        exit(1);
    }


    //--------------------//
    // process rev by rev //
    //--------------------//

    unsigned char rain_rate[AT_WIDTH][CT_WIDTH];
    unsigned char time_dif[AT_WIDTH][CT_WIDTH];
    unsigned short integrated_rain_rate[AT_WIDTH][CT_WIDTH];

    unsigned short nbd_array[AT_WIDTH][CT_WIDTH];
    unsigned short spd_array[AT_WIDTH][CT_WIDTH];
    unsigned short dir_array[AT_WIDTH][CT_WIDTH];
    unsigned short mle_array[AT_WIDTH][CT_WIDTH];
    unsigned char idx_array[AT_WIDTH][CT_WIDTH][4];
 

    float spd_impact_array[AT_WIDTH][CT_WIDTH];
    float dir_impact_array[AT_WIDTH][CT_WIDTH];
    float tbh_array[AT_WIDTH][CT_WIDTH];
    float tbv_array[AT_WIDTH][CT_WIDTH];



    unsigned int array_size = CT_WIDTH * AT_WIDTH;
    int rev_count = 0;

    for (int rev = start_rev; rev <= end_rev; rev++)
    {
        //----------------//
        // read impact or rain file //
        //----------------//
      if(rev%1==0){
	printf("Rev %d\n",rev);
        fflush(stdout);
      }
        char rain_file[1024];
        char impact_file[1024];
        if(use_impact){
	  sprintf(impact_file, "%s/RI2B_%0*d.bdat", impact_dir, REV_DIGITS, rev);
	  FILE* ifp = fopen(impact_file,"r");
          if(ifp==NULL){
	    fprintf(stderr,"Impact File Not Found Skipping REV %d\n",rev);
	    continue;
	  }
	  if( fread(spd_impact_array, sizeof(float), array_size, ifp) !=
            array_size)
	    {
	      fprintf(stderr, "%s: error reading impact file %s\n", command,
		      rain_file);
	      exit(1);
	    }
	  
	  if( fread(dir_impact_array, sizeof(float), array_size, ifp) !=
	      array_size)
	    {
	      fprintf(stderr, "%s: error reading impact file %s\n", command,
		      rain_file);
	      exit(1);
	    }
          fclose(ifp);
  	}
        else{
	  sprintf(rain_file, "%s/%0*d.irain", irr_dir, REV_DIGITS, rev);
	  FILE* ifp = fopen(rain_file, "r");
	  if (ifp == NULL)
	    {
	      fprintf(stderr, "%s: error opening rain file %s (continuing)\n",
		      command, rain_file);
	      continue;
	    }
	  if (fread(rain_rate, sizeof(char), array_size, ifp) != array_size ||
	      fread(time_dif, sizeof(char), array_size, ifp) != array_size ||
	      fseek(ifp, array_size, SEEK_CUR) != 0 ||
	      fread(integrated_rain_rate, sizeof(short), array_size, ifp) !=
	      array_size)
	    {
	      fprintf(stderr, "%s: error reading rain file %s\n", command,
		      rain_file);
	      exit(1);
	    }

	  //---------------------------------------//
	  // eliminate rain data out of time range //
	  //---------------------------------------//
	  
	  for (int ati = 0; ati < AT_WIDTH; ati++)
	    {
	      for (int cti = 0; cti < CT_WIDTH; cti++)
		{
		  int co_time = time_dif[ati][cti] * 2 - 180;
		  if (abs(co_time) > collocation_time)
                    integrated_rain_rate[ati][cti] = 2000;
		}
	    }
	  fclose(ifp);
	}

        //----------------------------//
        // Initialize MUDH parameters //
        //----------------------------//
	for (int ati = 0; ati < AT_WIDTH; ati++)
        {
            for (int cti = 0; cti < CT_WIDTH; cti++)
	    {
	      nbd_array[ati][cti]=MAX_SHORT;
	      spd_array[ati][cti]=MAX_SHORT;
	      dir_array[ati][cti]=MAX_SHORT;
	      mle_array[ati][cti]=MAX_SHORT;
	      tbh_array[ati][cti]=400.0;
	      tbv_array[ati][cti]=400.0;
              for(int j=0;j<4;j++) idx_array[ati][cti][j]=(unsigned char)0; 
	    }
	}
        //----------------//
        // read MUDH file //
        //----------------//

        char mudh_file[1024];
        if(use_fake_mudh_filenames){
	sprintf(mudh_file, "%s/FAKE.MP2B_%0*d.dat", mudh_dir, REV_DIGITS, rev);
	}
	else{
	sprintf(mudh_file, "%s/MP2B_%0*d.dat", mudh_dir, REV_DIGITS, rev);
	}
        ifp = fopen(mudh_file, "r");
        // skip if file not found

        char line[200];
        char* str;
        if(ifp==NULL){
	  printf("MUDH file %s not found Skipping Rev %d\n",mudh_file,rev);
	  continue;
	}
	while(!feof(ifp)){
	  fgets(line,200,ifp);
	  if(feof(ifp)) break;

	  str=strtok(line," \t");
	  int ati=atoi(str)-1;

	  str=strtok(NULL," \t");
	  int cti=atoi(str)-1;

	  str=strtok(NULL," \t");
          float nbd=atof(str);

	  str=strtok(NULL," \t");
          float spd=atof(str);

	  str=strtok(NULL," \t");
          float dir=atof(str);

	  str=strtok(NULL," \t");
          float mle=atof(str);

	  str=strtok(NULL," \t");
          float enof=atof(str);

	  str=strtok(NULL," \t");
          float tbh=atof(str);

	  str=strtok(NULL," \t");
          float tbv=atof(str);

	  str=strtok(NULL," \t");
          float trans=atof(str);

          
          for(int i=0;i<4;i++){
	    str=strtok(NULL," \t");
	    idx_array[ati][cti][i]=(unsigned char)atoi(str)-1;	    
	  }
          
	  str=strtok(NULL," \t");
          int beam=atoi(str)-1;

          nbd_array[ati][cti]=(unsigned short)((nbd+10)*1000+0.5);
          mle_array[ati][cti]=(unsigned short)((mle+30)*1000+0.5);
          spd_array[ati][cti]=(unsigned short)(spd*100+0.5);
          dir_array[ati][cti]=(unsigned short)(dir*100+0.5);
          tbh_array[ati][cti]=tbh;
          tbv_array[ati][cti]=tbv;
          // outer swath case
          if(beam==1) {
	    tbh_array[ati][cti]=400.0;
            nbd_array[ati][cti]=MAX_SHORT;
	  }

          /*
          printf("%d %d %g %g %g %g %d %d %d %d",ati,cti,dir,spd,mle,tbv,
		 (int)idx_array[ati][cti][0],(int)idx_array[ati][cti][1],
		 (int)idx_array[ati][cti][2],(int)idx_array[ati][cti][3]);
          exit(0);
          */
	}
        fclose(ifp);

        /******** OLD MUDH FILE READING CODE commented out
        char mudh_file[1024];
        char syscom[1024];
        sprintf(mudh_file, "%s/%0*d.mudh", mudh_dir, REV_DIGITS, rev);
        ifp = fopen(mudh_file, "r");
        int gzipped=0;
        if (ifp == NULL){
	  sprintf(syscom, "gunzip %s.gz",mudh_file);
          system(syscom);
	  ifp = fopen(mudh_file, "r");
          gzipped=1;
	}
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening MUDH file %s\n", command,
                mudh_file);
            fprintf(stderr, "%s: continuing...\n", command);
            continue;
        }
        fread(nbd_array, sizeof(short), array_size, ifp);
        fread(spd_array, sizeof(short), array_size, ifp);
        fread(dir_array, sizeof(short), array_size, ifp);
        fread(mle_array, sizeof(short), array_size, ifp);
        fclose(ifp);
        if(gzipped){
	  sprintf(syscom, "gzip %s",mudh_file);
          system(syscom);
	}
        ******/


        //----------------//
        // generate table //
        //----------------//

        rev_count++;
        double param[PARAM_COUNT];
        int got_param[PARAM_COUNT];

        for (int ati = 0; ati < AT_WIDTH; ati++)
        {
            for (int cti = 0; cti < CT_WIDTH; cti++)
            {
                //------------------------//
                // compute impact state
                //------------------------//
	      double irr=0;
	      bool impacted=false;
	      bool no_impact=false;

              // Use impact case
              if(use_impact){
		if(spd_impact_array[ati][cti]<-1000 
		    || dir_impact_array[ati][cti]<-1000){
		  continue;
		}
		impacted=(fabs(spd_impact_array[ati][cti])>spd_threshold)
		  || (fabs(dir_impact_array[ati][cti])>dir_threshold);
		no_impact=(fabs(spd_impact_array[ati][cti])<clear_spd_threshold)
		  && (fabs(dir_impact_array[ati][cti])<clear_dir_threshold);
	      }
 
              // use SSM/I case
              else{
	      
	      if (integrated_rain_rate[ati][cti] >= 1000){
		continue;
	      }
	         irr = (double)integrated_rain_rate[ati][cti] * 0.1;
                 if(irr>irr_threshold) impacted=true;
		 else if(irr==0) no_impact=true;
	      }

              total_samples++;
                //------------//
                // initialize //
                //------------//

                for (int i = 0; i < PARAM_COUNT; i++)
                    got_param[i] = 0;

                //-----------------------------------//
                // is this a valid wind vector cell? //
                //-----------------------------------//

                if (spd_array[ati][cti] == MAX_SHORT ||
                    mle_array[ati][cti] == MAX_SHORT)
                {
                    continue;
                }

                //------//
                // MUDH //
                //------//

                param[SPD_IDX] = (double)spd_array[ati][cti] * 0.01;
                got_param[SPD_IDX] = 1;
                if (dir_array[ati][cti] != MAX_SHORT)
                {
                    param[DIR_IDX] = (double)dir_array[ati][cti] * 0.01;
                    got_param[DIR_IDX] = 1;
                }
                param[MLE_IDX] = (double)mle_array[ati][cti] * 0.001 - 30.0;
                got_param[MLE_IDX] = 1;
                if (nbd_array[ati][cti] != MAX_SHORT)
                {
                    param[NBD_IDX] = (double)nbd_array[ati][cti] * 0.001 -
                        10.0;
                    got_param[NBD_IDX] = 1;
                }


                //----//
                // Tb //
                //----//


		if (tbv_array[ati][cti] < 300.0)
		  {
		    param[TBV_IDX] = tbv_array[ati][cti];
		    got_param[TBV_IDX] = 1;
		    param[TBV_STD_IDX] = 0;
		    got_param[TBV_STD_IDX] = 1;
		  }

		if (tbh_array[ati][cti] < 300.0)
		  {
		    param[TBH_IDX] = tbh_array[ati][cti];
		    got_param[TBH_IDX] = 1;
		    param[TBH_STD_IDX] = 0;
		    got_param[TBH_STD_IDX] = 1;
		  }

                 
                //--------------------------//
                // determine swath location //
                //--------------------------//

                int swath_idx;
                if (got_param[NBD_IDX])
                    swath_idx = 0;    // inner swath (both beams)
                else
                    swath_idx = 1;    // outer swath (single beam)

                //-----------------------------------------------//
                // don't do the outer swath unless PCA available //
                //----------------------------------------------//

                if (swath_idx == 1 && ! opt_outer_swath)
                    continue;

                //--------------------------------------//
                // determine principle component values //
                //--------------------------------------//

                double norm_param[PARAM_COUNT];
                for (int param_idx = 0; param_idx < PARAM_COUNT; param_idx++)
                {
                    // only normalize if you have the parameter
                    if (got_param[param_idx])
                    {
                        norm_param[param_idx] =
                            (param[param_idx] -
                            pca_mean[swath_idx][param_idx]) /
                            pca_std[swath_idx][param_idx];
                    }
                }

                int missing_info = 0;
                double pc[PC_COUNT];
                int pci[PC_COUNT];
                for (int pc_idx = 0; pc_idx < PC_COUNT; pc_idx++)
                {
                    pc[pc_idx] = 0.0;
                    for (int param_idx = 0; param_idx < PARAM_COUNT;
                        param_idx++)
                    {
                        if (pca_weights[swath_idx][pc_idx][param_idx] != 0.0)
                        {
                            // the parameter is needed
                            if (got_param[param_idx])
                            {
                                pc[pc_idx] +=
                                    pca_weights[swath_idx][pc_idx][param_idx] *
                                    norm_param[param_idx];
                            }
                            else
                            {
                                missing_info = 1;
                            }
                        }
                    }

                    if (missing_info)
                        break;    // get out of this loop

                    // compute the index too
                    pc_index[swath_idx][pc_idx].GetNearestIndexClipped(
                        pc[pc_idx], &(pci[pc_idx]));
                    if (pci[pc_idx] >= DIM) pci[pc_idx] = DIM - 1;
                    if (pci[pc_idx] < 0) pci[pc_idx] = 0;
                }

                if (missing_info)
                    continue;    // go to the next WVC


                // check for bad index and use Scott's index
		// #define DEBUG
 #ifdef DEBUG
                for(int c=0;c<4;c++){
		  if(pci[c]!=idx_array[ati][cti][c]){
		    fprintf(stderr,"\nTable Index #%d is %d. It should be %d\n"
			    ,c,idx_array[ati][cti][c],pci[c]);
                    fprintf(stderr,"\nParam[%d] is %g.\n",
			    c,pc[c]);
		  }
                  pci[c]=idx_array[ati][cti][c];
		}
#endif

                // accumulate
                if (no_impact)
                {
                    // rainfree
                    counts[swath_idx][1][pci[0]][pci[1]][pci[2]][pci[3]]++;
                    total_clear++;
                }
                else if (impacted)
                {
                   // rain
                    counts[swath_idx][2][pci[0]][pci[1]][pci[2]][pci[3]]++;
                    total_impacted++;
                }
                // all
                counts[swath_idx][0][pci[0]][pci[1]][pci[2]][pci[3]]++;
            }
        }
    }

    //-------------//
    // write table //
    //-------------//


    for (int swath_idx = 0; swath_idx < 2; swath_idx++)
    {
      // skip outer swath if you can't do it
      if (swath_idx == 1 && ! opt_outer_swath)
        continue;

      for (int i = 0; i < DIM; i++)
      {
        for (int j = 0; j < DIM; j++)
        {
          for (int k = 0; k < DIM; k++)
          {
            for (int l = 0; l < DIM; l++)
            {
              clear_tab[i][j][k][l] = 0.0;
              rain_tab[i][j][k][l] = 0.0;
              count_tab[i][j][k][l] =
                  (double)counts[swath_idx][0][i][j][k][l];
              if (counts[swath_idx][0][i][j][k][l] > 0)
              {
                double clear_prob = (double)counts[swath_idx][1][i][j][k][l] /
                  (double)counts[swath_idx][0][i][j][k][l];
                double rain_prob = (double)counts[swath_idx][2][i][j][k][l] /
                  (double)counts[swath_idx][0][i][j][k][l];

                clear_tab[i][j][k][l] = clear_prob;
                rain_tab[i][j][k][l] = rain_prob;
              }
              else
              {
                // mark as uncalculatable (2.0)
                clear_tab[i][j][k][l] = 2.0;
                rain_tab[i][j][k][l] = 2.0;
              }
            }
          }
        }
      }
      
      fwrite(clear_tab, sizeof(double), DIM * DIM * DIM * DIM, pcatab_ofp);
      fwrite(rain_tab, sizeof(double), DIM * DIM * DIM * DIM, pcatab_ofp);
      fwrite(count_tab, sizeof(double), DIM * DIM * DIM * DIM, pcatab_ofp);
    


      //-------------------------//
      // output statistics to xmgr file
      //------------------------//
      if(swath_idx==0){
	fprintf(efp0,"%% Swath Index %d (0/1)=(dual_beam/single_beam)\n",swath_idx);
	fprintf(efp0,"%% Prob_Threshold Percent_flagged Percent_false_alarm Percent_missed_detect\n");
      }
      else{
	fprintf(efp1,"%% Swath Index %d (0/1)=(dual_beam/single_beam)\n",swath_idx);
	fprintf(efp1,"%% Prob_Threshold Percent_flagged Percent_false_alarm Percent_missed_detect\n");
      }

      for(float t=0;t<0.2001;t+=0.001){

        double num_samples=0;
        double num_clear=0;
        double num_impacted=0;
        double num_flagged=0;
        double num_clear_flagged=0;
        double num_impacted_flagged=0;

	for(int i=0;i<DIM;i++){
	for(int j=0;j<DIM;j++){
	for(int k=0;k<DIM;k++){
	for(int m=0;m<DIM;m++){

	  num_samples+=count_tab[i][j][k][m];
          num_clear+=count_tab[i][j][k][m]*clear_tab[i][j][k][m];
          num_impacted+=count_tab[i][j][k][m]*rain_tab[i][j][k][m];
	  if(rain_tab[i][j][k][m]>t){
	    num_flagged+=count_tab[i][j][k][m];
	    num_impacted_flagged+=count_tab[i][j][k][m]*rain_tab[i][j][k][m];
	    num_clear_flagged+=count_tab[i][j][k][m]*clear_tab[i][j][k][m];
	  }
	}
	}
	}
	}
	double pf=100*num_flagged/num_samples;
	double pfa=100*num_clear_flagged/num_clear;
	double pmd=100-(100*num_impacted_flagged/num_impacted);

        if(swath_idx==0){
	  fprintf(efp0,"%g %g %g %g\n",t,pf,pfa,pmd);
	}
        else{
	  fprintf(efp1,"%g %g %g %g\n",t,pf,pfa,pmd);
	}
      }

    }

    //-------------//
    // close files //
    //-------------//
    fclose(efp0);
    fclose(efp1);
    fclose(pcatab_ofp);


    printf("%d revs %d total_samples %g percent impacted %g percent clear\n", rev_count,total_samples,
	   100.0*total_impacted/(float)total_samples,
	   100.0*total_clear/(float)total_samples
	   );

    return (0);
	}
