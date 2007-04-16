//==============================================================//
// Copyright (C) 1997-2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    make_cband_weight_map
//
// SYNOPSIS
//    make_cband_weight_map <cband_l2b_file> <kuband_l2b_file>  <weight_file>
//
// DESCRIPTION
//    Computes the proper weighting between C-band and Ku-band for each WVC
//    cell using the single frequency wind retrieval results.
//
// OPTIONS
//    None.
//
// OPERANDS
//    The following operands are supported:
//      <cband_l2b_file>     The input Level C BAND 2B wind field
//      <kuband_l2b_file>     The input Level Ku BAND 2B wind field
//      <weight_file>  The output weight file is a NATI x NCTI float
//                     array ( with two integer header values NCTI
//                     and   NATI
//                     A weight value of 1 uses C only and 0 uses Ku only
//
// EXAMPLES
//    An example of a command line is:
//    % make_cband_weight_map
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
//    Bryan W Stiles
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
#include "Misc.h"
#include "Wind.h"
#include "L2B.h"
#include "List.h"
#include "List.C"
#include "Array.h"

//-----------//
// TEMPLATES //
//-----------//

template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<AngleInterval>;

//-----------//
// CONSTANTS //
//-----------//

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

const char* usage_array[] = { "<cband_l2b_file>", "<kuband_l2b_file>",
			      "<weight_file>","[ds_thresh]","[nom_ratio]","[hdf_file]", 0};

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
    if (argc < 4 || argc > 7)
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* cband_file = argv[clidx++];
    const char* kuband_file = argv[clidx++];
    const char* weight_file = argv[clidx++];
    int hdf_flag = 0;
    float nom_ratio=0.5;
    float dsthresh=5.0;
    if (argc >= 5)
      dsthresh=atof(argv[clidx++]);
    if (argc >= 6)
      nom_ratio=atof(argv[clidx++]);
    if (argc == 7)
      hdf_flag=atoi(argv[clidx++]);

    //------------------//
    // read in l2b files //
    //------------------//

    L2B cl2b, kul2b;
    if (hdf_flag)
    {
        if (! cl2b.ReadPureHdf(cband_file))
        {
            fprintf(stderr, "%s: error reading HDF L2B file %s\n", command,
                cband_file);
            exit(1);
        }

        if (! kul2b.ReadPureHdf(kuband_file))
        {
            fprintf(stderr, "%s: error reading HDF L2B file %s\n", command,
                kuband_file);
            exit(1);
        }
    }
    else
    {
        if (! cl2b.OpenForReading(cband_file))
        {
            fprintf(stderr, "%s: error opening L2B file %s\n", command,
                cband_file);
            exit(1);
        }
        if (! cl2b.ReadHeader())
        {
            fprintf(stderr, "%s: error reading L2B header from file %s\n",
                command, cband_file);
            exit(1);
        }

        if (! cl2b.ReadDataRec())
        {
            fprintf(stderr, "%s: error reading L2B data record from file %s\n",
                command,cband_file);
            exit(1);
        }

        if (! kul2b.OpenForReading(kuband_file))
        {
            fprintf(stderr, "%s: error opening L2B file %s\n", command,
                kuband_file);
            exit(1);
        }
        if (! kul2b.ReadHeader())
        {
            fprintf(stderr, "%s: error reading L2B header from file %s\n",
                command, kuband_file);
            exit(1);
        }

        if (! kul2b.ReadDataRec())
        {
            fprintf(stderr, "%s: error reading L2B data record from file %s\n",
                command,kuband_file);
            exit(1);
        }
    }
    //-----------------------------------------//
    // use external rain flag file  if desired //
    //-----------------------------------------//
    // disabled for now
    // if(flag_file) l2b.frame.swath.ReadFlagFile(flag_file);


    // compute and write weights the algorithm is:
    // if Ku and C 5 x 5 average speeds agree within 2 m/s
    // and Ku speed < 25 m/s    --- Ku should be good enough case
    // set weight = 0
    //
    // else if Ku > 25 and C > 20 weight = 0.5 ---- high wind case
    //
    // else if Ku > C+2 weight = 1  --- rainy case
    //
    // else weight = 0.5   --- default case should be rare.


    int ncti_c=cl2b.frame.swath.GetCrossTrackBins();
    int nati_c=cl2b.frame.swath.GetAlongTrackBins();

    int ncti=kul2b.frame.swath.GetCrossTrackBins();
    int nati=kul2b.frame.swath.GetAlongTrackBins();

    if(ncti!=ncti_c || nati!=nati_c){
      fprintf(stderr,"Swath size mismatch between l2b files\n");
      exit(1);
    }

    float ** weights=(float**)make_array(sizeof(float),2,nati,ncti);
    float ** avespd_c=(float**)make_array(sizeof(float),2,nati,ncti);
    float ** avespd_ku=(float**)make_array(sizeof(float),2,nati,ncti);
    float ** true_spd=(float**)make_array(sizeof(float),2,nati,ncti);
    float ** spd_ku=(float**)make_array(sizeof(float),2,nati,ncti);
    float ** spd_c=(float**)make_array(sizeof(float),2,nati,ncti);
    int bothvalid=0, ngoodku=0, nhighspd=0, nrainy=0, ncvalid=0, nkuvalid=0,
      ndefault=0;

    int nrad=2;
    int first_valid_ati=-1;
    int nvalid_ati=0;
    for(int a=0;a<nati;a++){
      for(int c=0;c<ncti;c++){


	int ncfound=0, nkufound=0;
        avespd_c[a][c]=0;
        avespd_ku[a][c]=0;
	for(int a2=a-nrad;a2<a+nrad;a2++){
	  if(a2<0 || a2>nati-1) continue;
	  for(int c2=c-nrad;c2<c+nrad;c2++){
	    if(c2<0 || c2>ncti-1) continue;
	    WVC* cwvc=cl2b.frame.swath.GetWVC(c2,a2);
	    if(cwvc && cwvc->selected){
	      avespd_c[a][c]+=cwvc->selected->spd;
	      ncfound++;
	    }
	    WVC* kuwvc=kul2b.frame.swath.GetWVC(c2,a2);
	    if(kuwvc && kuwvc->selected){
	      avespd_ku[a][c]+=kuwvc->selected->spd;
	      nkufound++;
	    }
	  } // end c2 loop
	} // end a2 loop

        if(ncfound){
	  avespd_c[a][c]/=ncfound;
          WVC* wvc=cl2b.frame.swath.GetWVC(c,a);
          if(wvc && wvc->selected){
	    spd_c[a][c]= wvc->selected->spd;
            true_spd[a][c]=wvc->nudgeWV->spd;
	  }
          weights[a][c]=1;
	  ncvalid++;
          if(first_valid_ati<0) first_valid_ati=a;
	  if(a>=first_valid_ati+nvalid_ati){
	    nvalid_ati=a-first_valid_ati+1;
	  }
	}
        if(nkufound){
	  avespd_ku[a][c]/=nkufound;

          WVC* wvc=kul2b.frame.swath.GetWVC(c,a);
          if(wvc && wvc->selected){
	    spd_ku[a][c]= wvc->selected->spd;
	  }

	  weights[a][c]=0;
	  nkuvalid++;
          if(first_valid_ati<0) first_valid_ati=a;
	  if(a>=first_valid_ati+nvalid_ati){
	    nvalid_ati=a-first_valid_ati+1;
	  }
	}

	if(nkufound && ncfound){
	  bothvalid++;
	  float ds = avespd_c[a][c] - avespd_ku[a][c];
	  if(fabs(ds)<2.0 && avespd_ku[a][c]<25.0){
	    ngoodku++;
	    weights[a][c]=0;
	  }
	  else if( avespd_ku[a][c]>25.0 && fabs(ds)<dsthresh){
	    nhighspd++;
	    weights[a][c]=nom_ratio;
	  }
          else if( fabs(ds)>dsthresh ){
	    nrainy++;
	    weights[a][c]=1;
	  }
	  else{
	    ndefault++;
	    weights[a][c]=nom_ratio;
	  }
	}
      } // end c loop
    } // end a loop

    printf("ncvalid=%d nkuvalid=%d bothvalid=%d ngoodku=%d nhighspd=%d nrainy=%d ndefault=%d\n", ncvalid, nkuvalid,bothvalid,ngoodku,nhighspd,nrainy,ndefault);
    FILE* wfp=fopen(weight_file,"w");
    if(!wfp){
      fprintf(stderr,"Cannot open file %s for writing\n",weight_file);
      exit(1);
    }
    int f=first_valid_ati;
    int n=nvalid_ati;
    if(fwrite(&first_valid_ati,sizeof(int),1,wfp)!=1  ||
       fwrite(&nvalid_ati,sizeof(int),1,wfp)!=1  ||
       fwrite(&ncti,sizeof(int),1,wfp)!=1  ||
       !write_array(wfp,&weights[f],sizeof(float),2,n,ncti) ||
       !write_array(wfp,&avespd_c[f],sizeof(float),2,n,ncti) ||
       !write_array(wfp,&avespd_ku[f],sizeof(float),2,n,ncti) ||
       !write_array(wfp,&spd_c[f],sizeof(float),2,n,ncti) ||
       !write_array(wfp,&spd_ku[f],sizeof(float),2,n,ncti) ||
       !write_array(wfp,&true_spd[f],sizeof(float),2,n,ncti)){
      fprintf(stderr,"Error writing to file %s\n",weight_file);
      exit(1);
    }
    free_array(weights,2,nati,ncti);
    free_array(avespd_c,2,nati,ncti);
    free_array(avespd_ku,2,nati,ncti);
    free_array(spd_ku,2,nati,ncti);
    free_array(spd_c,2,nati,ncti);
    free_array(true_spd,2,nati,ncti);
    fclose(wfp);
    return (0);
}


