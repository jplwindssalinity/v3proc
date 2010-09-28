//==============================================================//
// Copyright (C) 1997-2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    l2a_to_ave
//
// SYNOPSIS
//    l2a_to_ave <input l2a file> <output l2a file> [optflg (-1 = linear averaging with negative sigma0 (default), 0 = linear averaging excluding negative sigma0, 1 = averaging of sigma0 in dB excluding negative sigma0)]
//
// DESCRIPTION
//    Composits all the measurements of a given flavor/look in a WVC. 
//    This is equivalent to the AVE algorithm on gridded L2A data when 
//    using an overlap gridding factor of 1 and a high resolution WVC 
//    posting.  Using a 2.5km WVC posting produces a product consistent
//    with BYU UHR wind processing.
//
//    The output data are put in an L2A file that can be processed by the standard
//    l1a_to_l2b processor
//
// OPTIONS
//   
//
// OPERANDS
//   
//
// EXAMPLES
//    An example of a command line is:
//      % l2a_to_ave l2a.dat l2aAVE.dat -1
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
//    Brent Williams (Brent.A.Williams@jpl.nasa.gov)
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
#include <stdlib.h>
#include <string.h>
#include <math.h>

//-----------//
// TEMPLATES //
//-----------//

// Class declarations needed for templates
// eliminates need to include the entire header file


//-----------//
// CONSTANTS //
//-----------//

//-------//
// HACKS //
//-------//

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


const char* usage_array[] = {"<l2a infile>","<l2a outfile>","[optflg (-1 = linear averaging with negative sigma0 (default), 0 = linear averaging excluding negative sigma0, 1 = averaging of sigma0 in dB excluding negative sigma0)]", 0};


//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int    argc,
    char*  argv[])
{
  char*  inputFilename;
  char*  outputFilename;
  FILE*  inputFp;
  FILE*  outputFp;
  FILE* fp;

  //header variables
  float crossTrackResolution,alongTrackResolution;
  int crossTrackBins,alongTrackBins;
  int zeroIndex;
  double startTime;


  //frame variables
  unsigned int rev;
  int ati;
  int cti;

  //MeasList variables
  int count;

  float value,XK, EnSlice,bandwidth,txPulseWidth;
  int landFlag;

  int count2;
  float lonO;//should be an array but neglect for now
  float latO;//should be an array but neglect for now

  float lon,lat;
  int measType;// just hard code measType as int for now
  float eastAzimuth,incidenceAngle;
  int beamIdx, startSliceIdx,numSlices;
  float scanAngle,A,B,C,azimuth_width,range_width;

  float scanAngleDeg;

  


  int count_ave2;//AVE 2 (outer swath) or 4 measurements per WVC
  float lat_ave,lon_ave,lonx_ave,lony_ave;
  float count_ave;

  //hard code 4 looks for now
  float count_aveFV,count_aveAV,count_aveFH,count_aveAH;
  float value_aveFV,value_aveAV,value_aveFH,value_aveAH;
  float inc_aveFV,inc_aveAV,inc_aveFH,inc_aveAH;
  float az_aveFV,az_aveAV,az_aveFH,az_aveAH;


  float A_aveFH, B_aveFH, C_aveFH;
  float A_aveAH, B_aveAH, C_aveAH;
  float A_aveFV, B_aveFV, C_aveFV;
  float A_aveAV, B_aveAV, C_aveAV;
  
  float ScanA_aveFH,ScanA_aveFV,ScanA_aveAH,ScanA_aveAV;

  float azx_aveFV,azy_aveFV,azx_aveFH,azy_aveFH,azx_aveAV,azy_aveAV,azx_aveAH,azy_aveAH;
 
  int measTypeFV,measTypeFH,measTypeAH,measTypeAV;

  int landFlag_ave;
  int writeflg;
  int optflg;

  
  //read in command line arguments
  if (argc<3)
    {
      fprintf(stderr,"Usage: l2a_to_ave %s %s %s\n",usage_array[0],usage_array[1],usage_array[2]);
      return(1);
    }
  else
    {
      inputFilename=argv[1];
      outputFilename=argv[2];
      printf("inputFilename: %s, outputFilename: %s \n",inputFilename, outputFilename);
      if (argc==4){
	sscanf(argv[3],"%d",&optflg);
	if (!(optflg==0||optflg==1)){
	  optflg=-1;//set to default if not a valid option
	}
      }else{
	optflg=-1;
      }
      printf("Processing Type is %d\n",optflg);
    }

  
  //read in l2a file and write out an l2a file that has been AVEed (i.e., makes composites of flavors or looks)
  
  // open l2a input file
  if (inputFilename == NULL){
    fprintf(stderr,"No Input File");
    return(1);
  }
  inputFp = fopen(inputFilename, "r");
  if (inputFp == NULL){
    fprintf(stderr,"Error opening input file %s for reading \n",inputFilename);
    return(1);
  }

  // open l2a output file
  if (outputFilename == NULL){
    fprintf(stderr,"No Output File");
    return(1);
  }
  outputFp = fopen(outputFilename, "w");
  if (outputFp == NULL){
    fprintf(stderr,"Error opening output file %s for writting \n",outputFilename);
    return(1);
  }

 
  //read and write header
  printf("reading and writing header\n");
  fp=inputFp;
  if (fread(&crossTrackResolution, sizeof(float), 1, fp) != 1 ||
        fread(&alongTrackResolution, sizeof(float), 1, fp) != 1 ||
        fread(&crossTrackBins, sizeof(int), 1, fp) != 1 ||
        fread(&alongTrackBins, sizeof(int), 1, fp) != 1 ||
        fread(&zeroIndex, sizeof(int), 1, fp) != 1 ||
        fread(&startTime, sizeof(double), 1, fp) != 1)
    {
      fprintf(stderr,"Error reading header in %s \n",inputFilename);
      return(1);
    }
  fp=outputFp;
  if (fwrite(&crossTrackResolution, sizeof(float), 1, fp) != 1 ||
        fwrite(&alongTrackResolution, sizeof(float), 1, fp) != 1 ||
        fwrite(&crossTrackBins, sizeof(int), 1, fp) != 1 ||
        fwrite(&alongTrackBins, sizeof(int), 1, fp) != 1 ||
        fwrite(&zeroIndex, sizeof(int), 1, fp) != 1 ||
        fwrite(&startTime, sizeof(double), 1, fp) != 1)
    {
      fprintf(stderr,"Error writting header in %s \n",outputFilename);
      return(1);
    }


  //read and write WVCs
   printf("reading and writing WVCs\n");
  //loop until reach end of file
  for(;;)
    {
      if(feof(inputFp))
	{
	  printf("REACHED END OF FILE");
	  break;
	}
      fp=inputFp;
      if (fread((void *)&rev, sizeof(unsigned int), 1, fp) != 1 ||
	  fread((void *)&ati, sizeof(int), 1, fp) != 1 ||
	  fread((void *)&cti, sizeof(int), 1, fp) != 1 ||
	  fread((void *)&count, sizeof(int), 1, fp) != 1 )
	{
	  if(feof(inputFp))
	    {
	      printf("REACHED END OF FILE\n ");
	      break;
	    }else
	    {
	      fprintf(stderr,"Error reading Frame in %s \n",inputFilename);
	      return(1);
	    }
	}

      
      lat_ave=0;
      lon_ave=0;
      lonx_ave=0;
      lony_ave=0;
      count_ave=0;
      value_aveFH=0;
      value_aveFV=0;
      value_aveAH=0;
      value_aveAV=0;
      count_aveFH=0;
      count_aveFV=0;
      count_aveAH=0;
      count_aveAV=0;
      inc_aveFH=0;
      inc_aveFV=0;
      inc_aveAH=0;
      inc_aveAV=0;
      az_aveFH=0;
      az_aveFV=0;
      az_aveAH=0;
      az_aveAV=0;
      A_aveFH=0;
      B_aveFH=0;
      C_aveFH=0;
      A_aveFV=0;
      B_aveFV=0;
      C_aveFV=0;
      A_aveAH=0;
      B_aveAH=0;
      C_aveAH=0;
      A_aveAV=0;
      B_aveAV=0;
      C_aveAV=0;

      ScanA_aveFH=0;
      ScanA_aveFV=0;
      ScanA_aveAH=0;
      ScanA_aveAV=0;

      azx_aveFV=0;
      azy_aveFV=0;
      azx_aveFH=0;
      azy_aveFH=0;
      azx_aveAV=0;
      azy_aveAV=0;
      azx_aveAH=0;
      azy_aveAH=0;

      landFlag_ave=0;

      //loop over measurements in this WVC
      for (int i=0;i<count;i++)
	{
	  
	  if (fread((void *)&value, sizeof(float), 1, fp) != 1 ||
	      fread((void *)&XK, sizeof(float), 1, fp) != 1 ||
	      fread((void *)&EnSlice, sizeof(float), 1, fp) != 1 ||
	      fread((void *)&bandwidth, sizeof(float), 1, fp) != 1 ||
	      fread((void *)&txPulseWidth, sizeof(float), 1, fp) != 1 ||
	      fread((void *)&landFlag, sizeof(int), 1, fp) != 1 ||
	      fread((void *)&count2, sizeof(int), 1, fp) != 1)
	    {
	      return(1);
	    }
	  if (count2>0)
	    {
	      for (int k=0 ; k<count2 ;k++)
		{//for now just neglect this
		  fread((void *)&lonO, sizeof(float), 1, fp);
		  fread((void *)&latO, sizeof(float), 1, fp);
		}
	    }
	  // outline.Read(fp) != 1 ||
	  //   centroid.ReadLonLat(fp) != 1 ||
	  if(fread((void *)&lon, sizeof(float), 1, fp) != 1 ||
	     fread((void *)&lat, sizeof(float), 1, fp) != 1 ||
	     fread((void *)&measType, sizeof(int), 1, fp) != 1 ||
	     fread((void *)&eastAzimuth, sizeof(float), 1, fp) != 1 ||
	     fread((void *)&incidenceAngle, sizeof(float), 1, fp) != 1 ||
	     fread((void *)&beamIdx, sizeof(int), 1, fp) != 1 ||
	     fread((void *)&startSliceIdx, sizeof(int), 1, fp) != 1 ||
	     fread((void *)&numSlices, sizeof(int), 1, fp) != 1 ||
	     fread((void *)&scanAngle, sizeof(float), 1, fp) != 1 ||
	     fread((void *)&A, sizeof(float), 1, fp) != 1 ||
	     fread((void *)&B, sizeof(float), 1, fp) != 1 ||
	     fread((void *)&C, sizeof(float), 1, fp) != 1 ||
	     fread((void *)&azimuth_width, sizeof(float), 1, fp) != 1 ||
	     fread((void *)&range_width, sizeof(float), 1, fp) != 1)
	    {
	      return(1);
	    }
	  if (value<0&&optflg>=0)
	    {
	      //do nothing i.e., exclude this measurement from the sum
	    }
	  else
	    {

	      if (optflg==1){//convert to dB if flag is set to 1
		value=10.0*log10(value);
	      }
	      scanAngleDeg=scanAngle*180.0/3.14159;
	      
	      //accumulate 
	      count_ave++;
	      lat_ave+=lat;
	      //lon_ave+=lon;
	      lonx_ave+=cos(lon);
	      lony_ave+=sin(lon);
	      //bitwise or the land flag
	      landFlag_ave=landFlag_ave | landFlag;
	      
	      if (beamIdx==0&&(scanAngleDeg<90||scanAngleDeg>=270))//Aft H-pol
		{
		  value_aveAH+=value;
		  inc_aveAH+=incidenceAngle;
		  azx_aveAH+=cos(eastAzimuth);
		  azy_aveAH+=sin(eastAzimuth);
		  //az_aveAH=eastAzimuth;
		  A_aveAH+=(A-1.0);
		  B_aveAH+=B;
		  C_aveAH+=C;
		  ScanA_aveAH+=scanAngle;
		  count_aveAH++;
		  measTypeAH=measType;
		  //printf("AH measType %d\n",measType);
		}
	      if (beamIdx==1&&(scanAngleDeg<90||scanAngleDeg>=270))//Aft V-pol
		{
		  value_aveAV+=value;
		  inc_aveAV+=incidenceAngle;
		  azx_aveAV+=cos(eastAzimuth);
		  azy_aveAV+=sin(eastAzimuth);
		  //	az_aveAV=eastAzimuth;
		  ScanA_aveAV+=scanAngle;
		  A_aveAV+=(A-1.0);
		  B_aveAV+=B;
		  C_aveAV+=C;
		  count_aveAV++;
		  measTypeAV=measType;
		  //printf("AV measType %d\n",measType);
		}
	      
	      if (beamIdx==0&&scanAngleDeg>=90&&scanAngleDeg<270)//fore H-pol
		{
		  value_aveFH+=value;
		  inc_aveFH+=incidenceAngle;
		  
		  azx_aveFH+=cos(eastAzimuth);
		  azy_aveFH+=sin(eastAzimuth);
		  //	az_aveFH=eastAzimuth;
		  
		  ScanA_aveFH+=scanAngle;
		  A_aveFH+=(A-1.0);
		  B_aveFH+=B;
		  C_aveFH+=C;
		  count_aveFH++;
		  measTypeFH=measType;
		  //printf("FH measType %d\n",measType);
		}
	      if (beamIdx==1&&scanAngleDeg>=90&&scanAngleDeg<270)//fore V-pol
		{
		  value_aveFV+=value;
		  inc_aveFV+=incidenceAngle;
		  
		  azx_aveFV+=cos(eastAzimuth);
		  azy_aveFV+=sin(eastAzimuth);
		  ScanA_aveFV+=scanAngle;
		  A_aveFV+=(A-1.0);
		  B_aveFV+=B;
		  C_aveFV+=C;
		  count_aveFV++;
		  measTypeFV=measType;
		  //printf("FV measType %d\n",measType);
		}
	    }//if value neg
	  
	}//loop over measurements in this WVC
      
      //get average values 
      if (count_ave>0)
	{
	  //lon_ave=lon_ave/(float)count_ave;//this dosn't handle wrapping
	  lon_ave=atan2(lony_ave,lonx_ave);//this handles wrapping
	  lat_ave=lat_ave/(float)count_ave;
	}

      count_ave2=0;
      if (count_aveFH>0)
	{
	  value_aveFH=value_aveFH/(float)count_aveFH;
	  az_aveFH=atan2(azy_aveFH,azx_aveFH);//this handles averaging angles that may have wrapping issues
	  //az_aveFH=az_aveFH/(float)count_aveFH;
	  inc_aveFH=inc_aveFH/count_aveFH;
	  ScanA_aveFH=ScanA_aveFH/count_aveFH;
	  A_aveFH=A_aveFH/(float)(count_aveFH*count_aveFH)+1.0;
	  B_aveFH=B_aveFH/(float)(count_aveFH*count_aveFH);
	  C_aveFH=C_aveFH/(float)(count_aveFH*count_aveFH);
	  count_ave2++;
	}
      if (count_aveFV>0)
	{
	  value_aveFV=value_aveFV/(float)count_aveFV;
	  az_aveFV=atan2(azy_aveFV,azx_aveFV);//this handles averaging angles that may have wrapping issues
	  //az_aveFV=az_aveFV/(float)count_aveFV;
	  inc_aveFV=inc_aveFV/(float)count_aveFV;
	  ScanA_aveFV=ScanA_aveFV/(float)count_aveFV;
	  A_aveFV=A_aveFV/(float)(count_aveFV*count_aveFV)+1.0;
	  B_aveFV=B_aveFV/(float)(count_aveFV*count_aveFV);
	  C_aveFV=C_aveFV/(float)(count_aveFV*count_aveFV);
	  count_ave2++;
	}
      if (count_aveAH>0)
	{
	  value_aveAH=value_aveAH/(float)count_aveAH;
	  az_aveAH=atan2(azy_aveAH,azx_aveAH);//this handles averaging angles that may have wrapping issues
	  //az_aveAH=az_aveAH/(float)count_aveAH;
	  inc_aveAH=inc_aveAH/(float)count_aveAH;
	  ScanA_aveAH=ScanA_aveAH/(float)count_aveAH;
	  A_aveAH=A_aveAH/(float)(count_aveAH*count_aveAH)+1.0;
	  B_aveAH=B_aveAH/(float)(count_aveAH*count_aveAH);
	  C_aveAH=C_aveAH/(float)(count_aveAH*count_aveAH);
	  count_ave2++;
	}
      if (count_aveAV>0)
	{
	  value_aveAV=value_aveAV/(float)count_aveAV;
	  az_aveAV=atan2(azy_aveAV,azx_aveAV);//this handles averaging angles that may have wrapping issues
	  //az_aveAV=az_aveAV/(float)count_aveAV;
	  inc_aveAV=inc_aveAV/(float)count_aveAV;
	  ScanA_aveAV=ScanA_aveAV/(float)count_aveAV;
	  A_aveAV=A_aveAV/(float)(count_aveAV*count_aveAV)+1.0;
	  B_aveAV=B_aveAV/(float)(count_aveAV*count_aveAV);
	  C_aveAV=C_aveAV/(float)(count_aveAV*count_aveAV);
	  count_ave2++;
	}
      
     

      //write output to file
      fp=outputFp;
      if(count_ave2>0)
	{
	  //only wrtie this WVC if it has measurements 
	  //(handles if all sigma0 in cell are negative and excluded when optflg>=0)
	if (fwrite((void *)&rev, sizeof(unsigned int), 1, fp) != 1 ||
	    fwrite((void *)&ati, sizeof(int), 1, fp) != 1 ||
	    fwrite((void *)&cti, sizeof(int), 1, fp) != 1 ||
	    fwrite((void *)&count_ave2, sizeof(int), 1, fp) != 1)
	  {
	    fprintf(stderr,"Error writting frame in %s \n",outputFilename);
	    return(1);
	  }
	}//if count_ave2>0

      //loop through 4 flavors/looks
      for (int k=0; k<4; k++)
	{
	  writeflg=0;
	  if (k==2&&count_aveFH>0)
	    {
	      value=value_aveFH;
	      incidenceAngle=inc_aveFH;
	      eastAzimuth=az_aveFH;
	      scanAngle=ScanA_aveFH;
	      A=A_aveFH;
	      B=B_aveFH;
	      C=C_aveFH;
	      writeflg=1;
	      measType=measTypeFH;
	      beamIdx=0;
	      //numSlices=count_aveFH;
	    }
	  if (k==0&&count_aveFV>0)
	    {
	      value=value_aveFV;
	      incidenceAngle=inc_aveFV;
	      eastAzimuth=az_aveFV;
	      scanAngle=ScanA_aveFV;
	      A=A_aveFV;
	      B=B_aveFV;
	      C=C_aveFV;
	      writeflg=1;
	      measType=measTypeFV;
	      beamIdx=1;
	      //numSlices=count_aveFV;
	    }
	  if (k==3&&count_aveAH>0)
	    {
	      value=value_aveAH;
	      incidenceAngle=inc_aveAH;
	      eastAzimuth=az_aveAH;
	      scanAngle=ScanA_aveAH;
	      A=A_aveAH;
	      B=B_aveAH;
	      C=C_aveAH;
	      writeflg=1;
	      measType=measTypeAH;
	      beamIdx=0;
	      //numSlices=count_aveAH;
	    }
	  if (k==1&&count_aveAV>0)
	    {
	      value=value_aveAV;
	      incidenceAngle=inc_aveAV;
	      eastAzimuth=az_aveAV;
	      scanAngle=ScanA_aveAV;
	      A=A_aveFV;
	      B=B_aveFV;
	      C=C_aveFV;
	      writeflg=1;
	      measType=measTypeAV;
	      beamIdx=1;
	      //numSlices=count_aveAV;
	    }
	 


	  if (optflg==1){//convert back form dB if flag is set to 1
	    value=(float) pow((double)10.0,(double)(value/10.0));
	  }

	  //put lon back in [0, 2*pi)
	  while(lon_ave<0){
	    lon_ave+=(6.28318);
	  }

	  count2=0;
	 
	  if (writeflg>0)
	    {
	      

	      if (fwrite((void *)&value, sizeof(float), 1, fp) != 1 ||
		  fwrite((void *)&XK, sizeof(float), 1, fp) != 1 ||
		  fwrite((void *)&EnSlice, sizeof(float), 1, fp) != 1 ||
		  fwrite((void *)&bandwidth, sizeof(float), 1, fp) != 1 ||
		  fwrite((void *)&txPulseWidth, sizeof(float), 1, fp) != 1 ||
		  fwrite((void *)&landFlag_ave, sizeof(int), 1, fp) != 1 ||
		  fwrite((void *)&count2, sizeof(int), 1, fp) != 1||
		  fwrite((void *)&lon_ave, sizeof(float), 1, fp) != 1 ||
		  fwrite((void *)&lat_ave, sizeof(float), 1, fp) != 1 ||
		  fwrite((void *)&measType, sizeof(int), 1, fp) != 1 ||
		  fwrite((void *)&eastAzimuth, sizeof(float), 1, fp) != 1 ||
		  fwrite((void *)&incidenceAngle, sizeof(float), 1, fp) != 1 ||
		  fwrite((void *)&beamIdx, sizeof(int), 1, fp) != 1 ||
		  fwrite((void *)&startSliceIdx, sizeof(int), 1, fp) != 1 ||
		  fwrite((void *)&numSlices, sizeof(int), 1, fp) != 1 ||
		  fwrite((void *)&scanAngle, sizeof(float), 1, fp) != 1 ||
		  fwrite((void *)&A, sizeof(float), 1, fp) != 1 ||
		  fwrite((void *)&B, sizeof(float), 1, fp) != 1 ||
		  fwrite((void *)&C, sizeof(float), 1, fp) != 1 ||
		  fwrite((void *)&azimuth_width, sizeof(float), 1, fp) != 1 ||
		  fwrite((void *)&range_width, sizeof(float), 1, fp) != 1)
		{
		  return(1);
		}



	    }
	}//loop over flavors

      
    }//loop over WVCs






  //close files
  fclose(inputFp);
  fclose(outputFp);
  
  
  return (0);
}
