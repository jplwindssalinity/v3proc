//==============================================================//
// Copyright (C) 1998-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//     prob_3d
//
// SYNOPSIS
//     prob_3d [-f filter] [ -h ] [ -s ] [ -i ] sim_cfgfile
//       meas_file output_base
//
// DESCRIPTION
//     Calculates measurement value and its appropriate solution
//     curve
//
//
// OPTIONS
//     [ - filter ]  Use the specified filter.
//     [ -h ]        Help. Displays the list of filters.
//     [ -s ]        Use a l2a_s0 format measurement file.
//     [ -g ]        Use l2a_s0 file created from official processor L2A
//     [ -i ]        Plot probabilities due to individual measurements
//
//     Measurement File Format (Unless -s is set)
//       WIND_DIR(DEGREES) WIND_SPEED(m/s)
//       MEAS_TYPE AZIMUTH INC_ANGLE NUMBER_OF_MEASUREMENTS
//       MEAS_TYPE AZIMUTH INC_ANGLE NUMBER_OF_MEASUREMENTS
//       MEAS_TYPE AZIMUTH INC_ANGLE NUMBER_OF_MEASUREMENTS
//
// EXAMPLES
//     An example of a command line is:
//
// ENVIRONMENT
//     Not environment dependent.
//
// EXIT STATUS
//     The following exit values are returned:
//         1  Program executed successfully
//        >0  Program had an error
//
// NOTES
//     None.
//
// AUTHOR
//     Bryan Stiles (bstiles@acid.jpl.nasa.gov)
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
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Misc.h"
#include "ConfigList.h"
#include "ConfigSim.h"
#include "Tracking.h"
#include "Tracking.C"
#include "Distributions.h"
#include "AngleInterval.h"

//-----------//
// TEMPLATES //
//-----------//
template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<long>;
template class List<OffsetList>;
template class List<AngleInterval>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;

//-----------//
// CONSTANTS //
//-----------//

#define A_VAL          0.0803419
#define B_VAL          0.120513
#define C_VAL          0.0607574
#define XKVAL_INNER    2.5176E+06
#define ENSLICE_INNER  1235.6
#define XKVAL_OUTER    2.7167E+06
#define ENSLICE_OUTER  1234.9
#define BANDWIDTH      8314.0   // Hz
#define PULSEWIDTH     0.00149709
#define SPD_RES        0.1 // m/s
#define DIR_RES        1.0 // Degrees
#define OPTSTRING      "f:hsig"
#define MAX_SPEED      50 // m/s

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

const char* usage_array[] = { "[-f filter]", "[ -h ]", "[ -s ]", "[ -i ]",
    "[ -g ]", "<sim_config_file>", "<meas_file>", "<output_base>", 0};

int
ComputeProb(
    float**    prob,
    int        num_dirs,
    int        num_spds,
    MeasList*  meas_list,
    Kp*        kp,
    GMF*       gmf)
{
    float max_obj = -HUGE_VAL;

        //-------------- Determine total probabilities------------------//
    for(int d=0;d<num_dirs;d++){
      for(int s=0;s<num_spds;s++){
        float spd=s*SPD_RES;
            float dir=d*DIR_RES*dtr;
        prob[d][s]=gmf->_ObjectiveFunction(meas_list, spd,dir,kp);
        if(prob[d][s]>max_obj) max_obj=prob[d][s];
      }
    }
        float sum=0;
    for(int d=0;d<num_dirs;d++){
      for(int s=0;s<num_spds;s++){
        prob[d][s]=exp((prob[d][s]-max_obj)/2.0);
        sum+=prob[d][s];
          }
    }

    for(int d=0;d<num_dirs;d++){
      for(int s=0;s<num_spds;s++){
        prob[d][s]/=sum;
      }
    }

        return(1);
}

int WriteProb(char* filename, float** prob, int num_dirs, int num_spds){

  FILE* ofp=fopen(filename,"w");
  if(ofp==NULL){
    return(0);
  }

  unsigned char** cprob=(unsigned char**)make_array(sizeof(char),2,
                            num_dirs,num_spds);
  if(cprob==NULL) return(0);
  float max_prob=0;
  for(int d=0;d<num_dirs;d++){
    for(int s=0;s<num_spds;s++){
     if(prob[d][s]>max_prob) max_prob=prob[d][s];
    }
  }
  float scale=255.0/max_prob;
  if(fwrite((void*)&scale,sizeof(float),1,ofp)!=1){
    free_array((void*)cprob,2,num_dirs,num_spds);
    fclose(ofp);
    return(0);
  }
  for(int d=0;d<num_dirs;d++){
    for(int s=0;s<num_spds;s++){
      cprob[d][s]=(unsigned char)(scale*prob[d][s]+0.5);
    }
    if(fwrite((void*)&cprob[d][0],sizeof(char),num_spds,ofp)!=
           (unsigned int)num_spds){
      free_array((void*)cprob,2,num_dirs,num_spds);
      fclose(ofp);
      return(0);
    }
  }
  free_array((void*)cprob,2,num_dirs,num_spds);
  fclose(ofp);
  return(1);
}

int read_meas_file(FILE* mfp, GMF* gmf, Kp* kp,
           float true_spd, float true_dir,
           MeasList* meas_list){
          char line_from_file[200];
          char str1[20], str2[20], str3[20], str4[20];
          int finished=0;
          int look_count=0;
          int meas_idx=0;
      while(1){
        // skip comments
        line_from_file[0]='#';
        while(line_from_file[0]=='#'){
          fgets(line_from_file,200,mfp);
          if(feof(mfp)){
        finished=1;
        break;
          }
        }
        if(finished) break;

        // read in meas_type, azim, inc, and number of measurements
        sscanf(line_from_file,"%s %s %s %s",str1,str2,str3,str4);
        Meas::MeasTypeE mtype = Meas::NONE;
        if (strcasecmp(str1,"HH")==0)
          mtype=Meas::HH_MEAS_TYPE;
        else if (strcasecmp(str1,"VV")==0)
          mtype=Meas::VV_MEAS_TYPE;
        else if (strcasecmp(str1,"VVHV")==0)
          mtype=Meas::VV_HV_CORR_MEAS_TYPE;
        else if (strcasecmp(str1,"HHVH")==0)
          mtype=Meas::HH_VH_CORR_MEAS_TYPE;
        else{
          fprintf(stderr,"read_meas_file: Bad Measurement Type %s\n",str1);
          exit(1);
        }
        float azim=atof(str2)*dtr;
        float inc=atof(str3)*dtr;
        int num_meas=atoi(str4);

        // create measurements, append to meas_lis
        float chi=true_dir-azim+pi;
        float true_sigma0;
        gmf->GetInterpolatedValue(mtype, inc, true_spd, chi, &true_sigma0);

        Meas meas;
        meas.measType=mtype;
        meas.incidenceAngle=inc;
        meas.eastAzimuth=azim;
        if(inc>50*dtr){
          meas.XK=XKVAL_OUTER;
          meas.EnSlice = ENSLICE_OUTER;
        }
        else{
          meas.XK=XKVAL_INNER;
          meas.EnSlice = ENSLICE_INNER;
        }
        meas.bandwidth = BANDWIDTH;

        meas.txPulseWidth = PULSEWIDTH;
        meas.A=A_VAL;
        meas.B=B_VAL;
        meas.C=C_VAL;
        meas.value=true_sigma0;
        float var=gmf->GetVariance(&meas,true_spd,chi,true_sigma0,kp);
        Gaussian gauss(var,0.0);

        for(int c=0;c<num_meas;c++){
          Meas* new_meas = new Meas;
          *new_meas=meas;
          new_meas->value+=gauss.GetNumber();
          meas_list->Append(new_meas);
          meas_idx++;
          if(meas_idx>999){
        fprintf(stderr,"read_meas_file:Too many measurements!\n");
        exit(0);
          }
        }


        look_count++;

      }
      fclose(mfp);
      //-------- Check for empty measurement file----------- //
      if(look_count==0){
        fprintf(stderr,"read_meas_file: Empty measuremnet file \n");
        exit(1);
      }
      return(1);
}

int read_s0_meas_file(FILE* mfp, MeasList* meas_list, int gs_flag){
        char line[1024];
        char typestring[20];
        float inc, azi, s0, xk, en_slice, bandwidth, pulse_width;
        float kpa, kpb, kpc;
        for (int meas_idx = 1; ; meas_idx++)
      {
            if (fgets(line, 1024, mfp) != line)
          break;

            if (line[0] == '#')
          continue; // skip comments

            int bad_read = 0;
        if (sscanf(line, " %s %f %f %f %f %f %f %f %f %f %f",
               typestring, &inc, &azi, &s0, &xk, &en_slice, &bandwidth,
               &pulse_width, &kpa, &kpb, &kpc) != 11)
          {
        bad_read = 1;
          }
            if (bad_read)
          {
                if (meas_idx == 1){
          fprintf(stderr,"Empty Measurement File\n");
          exit(1);
        }
                else
          break;
          }
            Meas* new_meas = new Meas();
            if (strcasecmp(typestring, "VV") == 0)
          {
                new_meas->measType = Meas::VV_MEAS_TYPE;
          }
            else if (strcasecmp(typestring, "HH") == 0)
          {
                new_meas->measType = Meas::HH_MEAS_TYPE;
          }
            else if (strcasecmp(typestring, "VH") == 0)
          {
                new_meas->measType = Meas::VH_MEAS_TYPE;
          }
            else if (strcasecmp(typestring, "HV") == 0)
          {
                new_meas->measType = Meas::HV_MEAS_TYPE;
          }
            else if (strcasecmp(typestring, "VVHV") == 0)
          {
                new_meas->measType = Meas::VV_HV_CORR_MEAS_TYPE;
          }
            else if (strcasecmp(typestring, "HHVH") == 0)
          {
                new_meas->measType = Meas::HH_VH_CORR_MEAS_TYPE;
          }
            else
          {
                fprintf(stderr, "read_s0_meas_file: error parsing measurement type %s\n",typestring);
                exit(1);
          }
            new_meas->incidenceAngle = inc * dtr;
            new_meas->eastAzimuth = azi * dtr;
            if( new_meas->eastAzimuth < 0) new_meas->eastAzimuth+=two_pi;
            else if( new_meas->eastAzimuth > two_pi) new_meas->eastAzimuth-=two_pi;
            new_meas->value = s0;
        new_meas->XK = xk;
        new_meas->EnSlice = en_slice;
        new_meas->bandwidth = bandwidth;
        new_meas->txPulseWidth = pulse_width;
        new_meas->A = kpa;
        new_meas->B = kpb;
        new_meas->C = kpc;
            if(gs_flag) new_meas->numSlices=-1;
            else new_meas->numSlices=1;

            //-----------------//
            // add measurement //
            //-----------------//
            if(meas_idx>999){
          fprintf(stderr,"read_s0_meas_file: Too many measurements.\n");
              exit(0);
        }
            meas_list->Append(new_meas);
      }
    return(1);

}

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int        argc,
    char*    argv[])
{
        // Initialize
        int opt_no_correlation = 0;
    int opt_no_copol = 0;
    int opt_no_aft_look = 0;
    int opt_no_fore_look = 0;
    int opt_no_inner_beam = 0;
    int opt_no_outer_beam = 0;
    int opt_no_HHVH=0;
        int opt_no_VVHV=0;
        int s0_format=0;
        int gs_to_s0_format=0;
        int output_individuals=0;

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);
    extern int optind;
    extern char *optarg;
    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
      {
        switch(c)
          {
          case 'f':
        if (strcasecmp(optarg, "corr0") == 0)
          {
            opt_no_correlation = 1;
          }
        else if (strcasecmp(optarg, "copol0") == 0)
          {
            opt_no_copol = 1;
          }
        else if (strcasecmp(optarg, "aft0") == 0)
          {
            opt_no_aft_look = 1;
          }
        else if (strcasecmp(optarg, "fore0") == 0)
          {
            opt_no_fore_look = 1;
          }
        else if (strcasecmp(optarg, "outer0") == 0)
          {
            opt_no_outer_beam = 1;
          }
        else if (strcasecmp(optarg, "inner0") == 0)
          {
            opt_no_inner_beam = 1;
          }
        else if (strcasecmp(optarg, "HHVH0") == 0)
          {
            opt_no_HHVH = 1;
          }
        else if (strcasecmp(optarg, "VVHV0") == 0)
          {
            opt_no_VVHV = 1;
          }
        else
          {
            fprintf(stderr, "%s: error parsing filter %s\n", command,
                optarg);
            exit(1);
          }
        break;
          case 'h':
        printf("Filters:\n");
        printf("  corr0  : Remove correlation measurements\n");
        printf("  copol0 : Remove copolarization measurements\n");
        printf("  aft0   : Remove aft look measurements\n");
        printf("  fore0  : Remove fore look measurements\n");
        printf("  inner0 : Remove inner beam measurements\n");
        printf("  outer0 : Remove outer beam  measurements\n");
        printf("  HHVH0 : Remove HHVH  measurements\n");
        printf("  VVHV0 : Remove VVHV  measurements\n");
        exit(0);
        break;
          case 's':
        s0_format=1;
        break;
          case 'g':
        s0_format=1;
                gs_to_s0_format=1;
        break;
              case 'i':
                output_individuals=1;
        break;
          case '?':
        usage(command, usage_array, 1);
        break;
          }
      }
    if (argc != optind + 3)
      usage(command, usage_array, 1);

        char* config_file  = argv[optind++];
        char* meas_file = argv[optind++];
    char* output_base = argv[optind++];


    //---------------------//
        // read in config file //
        //---------------------//

        ConfigList config_list;
        if (! config_list.Read(config_file))
        {
                fprintf(stderr, "%s: error reading sim config file %s\n",
                        command, config_file);
                exit(1);
        }

        //-------------------------------------//
        // read the geophysical model function //
        //-------------------------------------//

        GMF gmf;
        if (! ConfigGMF(&gmf, &config_list))
        {
                fprintf(stderr, "%s: error configuring GMF\n", command);
                exit(1);
        }

    //--------------//
    // configure Kp //
    //--------------//

    Kp kp;

        if (! ConfigKp(&kp, &config_list))
        {
            fprintf(stderr, "%s: error configuring Kp\n", command);
            exit(1);
        }

    // Open measurement file read wind spd and direction
        FILE* mfp=fopen(meas_file,"r");
        char line_from_file[200];
        if(mfp==NULL){
      fprintf(stderr,"%s: Error opening %s for reading.\n",command,
          meas_file);
          exit(1);
    }

        // skip comments
        line_from_file[0]='#';
        while(line_from_file[0]=='#'){
      fgets(line_from_file,200,mfp);
      if(feof(mfp)){
        fprintf(stderr,"%s: Unexpected EOF in file %s\n",command,
            meas_file);
        exit(1);
      }
    }
        char str1[20], str2[20];
    sscanf(line_from_file,"%s %s\n",str1,str2);
    float true_spd, true_dir;
        true_dir=atof(str1)*dtr;
    true_spd=atof(str2);
    printf("## true_spd %g true_dir %g \n#\n",true_spd, true_dir*rtd);

        MeasList meas_list;

        if(!s0_format) read_meas_file(mfp,&gmf,&kp,true_spd,true_dir,&meas_list);

        else read_s0_meas_file(mfp,&meas_list,gs_to_s0_format);



        int num_dirs=int(360.0/DIR_RES+0.5);
        int num_spds=int(MAX_SPEED/SPD_RES+0.5);

        /********* Filter Measurement List **************/
        /*********  and write logfile      **************/
        int meas_idx=1;
        int remove=0;

        /**** Determine fore/aft boundaries in EastAzimuth ***/
        float sin_midazi=0.0;
        int num=0;
        for (Meas* meas = meas_list.GetHead(); meas; meas=meas_list.GetNext()){
           sin_midazi=sin_midazi+sin(meas->eastAzimuth);
       num++;
    }
        sin_midazi=sin_midazi/num;
        for (Meas* meas = meas_list.GetHead(); meas; )
        {
            remove = opt_no_correlation &&
                (meas->measType == Meas::VV_HV_CORR_MEAS_TYPE ||
                meas->measType == Meas::HH_VH_CORR_MEAS_TYPE);

            remove = remove || (opt_no_copol &&
                (meas->measType == Meas::VV_MEAS_TYPE ||
                meas->measType == Meas::HH_MEAS_TYPE));

        /****** Using eastAzimuth as a substitute for scanAngle
                    in determining fore or aft look *************/
            remove = remove || (opt_no_aft_look && sin(meas->eastAzimuth)<sin_midazi);

            remove = remove || (opt_no_fore_look && sin(meas->eastAzimuth)>sin_midazi);

            remove = remove || (opt_no_inner_beam && meas->incidenceAngle<50*dtr);

            remove = remove || (opt_no_outer_beam && meas->incidenceAngle>50*dtr);

            remove = remove || ( opt_no_HHVH &&
                (meas->measType == Meas::HH_VH_CORR_MEAS_TYPE));

            remove = remove || ( opt_no_VVHV &&
                (meas->measType == Meas::VV_HV_CORR_MEAS_TYPE));

            if(remove)
            {
                meas = meas_list.RemoveCurrent();
                delete meas;
                meas = meas_list.GetCurrent();
            }
            else
            {
          float chi=true_dir-meas->eastAzimuth+pi;
          float true_sigma0;
          gmf.GetInterpolatedValue(meas->measType,
                       meas->incidenceAngle,
                       true_spd, chi, &true_sigma0);
          float percent_error=
        fabs((meas->value - true_sigma0)/true_sigma0);
          printf("\n\n Meas # %d: model_s0 %g meas_s0 %g norm_err %g\n",
             meas_idx, true_sigma0, meas->value, percent_error);
              printf("%s Inc %g  Azim %g XK %g EnSlice %g\n",
             meas_type_map[meas->measType], meas->incidenceAngle * rtd,
             meas->eastAzimuth * rtd, meas->XK, meas->EnSlice);
              printf("Bandwidth %g  PulseWidth %g A %g B %g C %g\n",
             meas->bandwidth, meas->txPulseWidth, meas->A, meas->B,
             meas->C);
          meas = meas_list.GetNext();
          meas_idx++;
            }

        }

    float** prob = (float**)make_array(sizeof(float),2,num_dirs,num_spds);

        if(!ComputeProb(prob,num_dirs,num_spds,&meas_list,&kp,&gmf)){
      fprintf(stderr,"%s:Error computing probabilities\n",command);
      exit(1);
    }

        //--write out probabilities for combined measurements---//
    char filename[200];
    sprintf(filename,"%s.total",output_base);
    if(!WriteProb(filename,prob,num_dirs,num_spds)){
      fprintf(stderr,"%s:Error writing  probabilities to file %s\n",command,
          filename);
      exit(1);
    }

        if(output_individuals){
        //---Compute and Output individual measurement probabilities---//
        meas_idx=1;
        for(Meas* meas=meas_list.GetHead();meas;meas=meas_list.GetNext()){
      MeasList mlist;
          mlist.Append(meas);
      if(!ComputeProb(prob,num_dirs,num_spds,&mlist,&kp,&gmf)){
        fprintf(stderr,"%s:Error computing probabilities\n",command);
        exit(1);
      }

      //--write out probabilities for individual measurements---//
      char filename[200];
      sprintf(filename,"%s.%3.3d",output_base,meas_idx);
      if(!WriteProb(filename,prob,num_dirs,num_spds)){
        fprintf(stderr,"%s:Error writing  probabilities to file %s\n",
            command, filename);
        exit(1);
      }
          mlist.GetHead();
      meas=mlist.RemoveCurrent();
          meas_idx++;
    }
    }
        free_array((void*)prob,2,num_dirs,num_spds);
    exit(0);
}
