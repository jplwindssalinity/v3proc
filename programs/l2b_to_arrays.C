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

const char* usage_array[] = { "<l2b_file>", "<out_file>", "[rank (-1=near, 0=sel, 5= nudge, 6= hdfdirth)]", 0};

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
    if (argc != 3 && argc !=4 )
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* in_file = argv[clidx++];
    const char* out_file = argv[clidx++];
    int rank=0;
    if(argc==4) rank=atoi(argv[clidx++]);
    //------------------//
    // read in l2b file //
    //------------------//

    L2B l2b;

    
    if (! l2b.SmartRead(in_file))
      {
	fprintf(stderr, "%s: error reading L2B data record from file %s\n",
                command,in_file);
	exit(1);
      }
    

    
    int ncti=l2b.frame.swath.GetCrossTrackBins();
    int nati=l2b.frame.swath.GetAlongTrackBins();


    float ** spd=(float**)make_array(sizeof(float),2,nati,ncti);
    float ** dir=(float**)make_array(sizeof(float),2,nati,ncti);
    float ** lat=(float**)make_array(sizeof(float),2,nati,ncti);
    float ** lon=(float**)make_array(sizeof(float),2,nati,ncti);

    int found_valid=0;
    int first_valid_ati=0, nvalid_ati=0;
    for(int a=0;a<nati;a++){
      for(int c=0;c<ncti;c++){
	    
	    WVC* wvc=l2b.frame.swath.GetWVC(c,a);

            bool good = true;
            if(wvc){
	      WindVectorPlus* wvp;
	      if(rank==-1 && wvc){
                if(wvc->nudgeWV)
		  wvp=wvc->GetNearestToDirection(wvc->nudgeWV->dir);
		else wvp=NULL;
	      }
	      if(rank<=0) wvp=wvc->selected;
              else if(rank<5) wvp=wvc->ambiguities.GetByIndex(rank-1);
	      else if(rank==5) wvp=wvc->nudgeWV;
	      else wvp=(WindVectorPlus*)wvc->specialVector;
	      if(wvp){
		spd[a][c]=wvp->spd;
		dir[a][c]=wvp->dir;
		lat[a][c]=wvc->lonLat.latitude;
		lon[a][c]=wvc->lonLat.longitude;
	      }
	      else good =false;
	      nvalid_ati=a+1-first_valid_ati;
	      if(!found_valid){
		found_valid=1;
		first_valid_ati=a;
	      }
	    }
	    else good =false;

	    if(good==false){
              spd[a][c]=-1;
              dir[a][c]=0;
	      lat[a][c]=0;
	      lon[a][c]=0;
	    }
      } // end c loop
    } // end a loop

    FILE* wfp=fopen(out_file,"w");
    if(!wfp){
      fprintf(stderr,"Cannot open file %s for writing\n",out_file);
      exit(1);
    }
    int f=first_valid_ati;
    int n=nvalid_ati;
    if(fwrite(&first_valid_ati,sizeof(int),1,wfp)!=1  ||
       fwrite(&nvalid_ati,sizeof(int),1,wfp)!=1  ||
       fwrite(&ncti,sizeof(int),1,wfp)!=1  ||
       !write_array(wfp,&spd[f],sizeof(float),2,n,ncti) ||
       !write_array(wfp,&dir[f],sizeof(float),2,n,ncti) ||
       !write_array(wfp,&lat[f],sizeof(float),2,n,ncti) ||
       !write_array(wfp,&lon[f],sizeof(float),2,n,ncti)){
      fprintf(stderr,"Error writing to file %s\n",out_file);
      exit(1);
    }
    free_array(spd,2,nati,ncti);
    free_array(dir,2,nati,ncti);
    free_array(lat,2,nati,ncti);
    free_array(lon,2,nati,ncti);
    fclose(wfp);
    return (0);
}


