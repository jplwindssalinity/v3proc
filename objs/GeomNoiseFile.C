#include "GeomNoiseFile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Constants.h"

GeomNoiseFile::GeomNoiseFile(const char* in_file, int need_all_looks)
  : infile(in_file), needAllLooks(need_all_looks)
{
    lineno=0;
    ifp=fopen(in_file,"r");
    if(!ifp){
      fprintf(stderr,"Cannot open %s\n",in_file);
      exit(1);
    }
    FirstPass();
    fclose(ifp);
    ifp=fopen(in_file,"r");
    if(!ifp){
      fprintf(stderr,"Cannot open %s\n",in_file);
      exit(1);
    }
    lineno=0;
    ReadHeader();
}
void
GeomNoiseFile::Rewind(){
    fclose(ifp);
    ifp=fopen(infile,"r");
    if(!ifp){
      fprintf(stderr,"Cannot open %s\n",infile);
      exit(1);
    }
    ReadHeader();
}

void
GeomNoiseFile::ReadHeader(){
    char line[4096];
    char* str;

    while(1){
     if (fgets(line, 4096, ifp) != line){
       fprintf(stderr,"Empty GeomNoiseFile\n");
       exit(1);
     }
     lineno++;
     if (line[0]=='#') continue; // skip comments
     str=strtok(line," \t\n");
     nbeams=atoi(str);
     
     
     nlooks=2*nbeams;
     if(nlooks>MAX_NUM_LOOKS || nlooks<1){
       fprintf(stderr,"Bad number of beams %d and thus looks %d\n",nbeams,nlooks);
       fprintf(stderr,"If you need more beams edit MAX_NUM_LOOKS in quick_winds.C and recompile\n");
       exit(1);
     }
     str=strtok(NULL," \t\n");
     gridres=atof(str);
     
     str=strtok(NULL," \t\n");
     if(str!=NULL){
       bias[0]=pow(10,0.1*atof(str))-1;
       for(int c=1;c<nlooks;c++){
	 str=strtok(NULL," \t\n");
	 bias[c]=pow(10,0.1*atof(str))-1;
       }
     }
     break;
    }
}

void
GeomNoiseFile::FirstPass(){
 
    ReadHeader();
    max_ctd=0;
    min_ctd=0;
    ncti=0;
    int firstline=1;
    int n1=0,n2=0;
    float azi1=0,azi2=0,ctd1,ctd2;
    while(1){
      if(!ReadLine()) break;
      int totnmeas=0;
      int missing_look=0;
      for(int k=0;k<nlooks;k++){
	totnmeas+=nmeas[k];
        if(nmeas[k]==0) missing_look=1;
      }
      if(totnmeas==0) continue;
      if(needAllLooks && missing_look) continue;
      if(firstline) min_ctd=ctd-0.5*gridres;
      firstline=0;
      max_ctd=ctd+0.5*gridres;

      if(ctd<=0 && ctd > -gridres){
        azi1=0;
        n1=0;
        ctd1=ctd;
	for(int j=0;j<nlooks;j++){
	  if(nmeas[j]>0){
	    n1++;
	    azi1+=azim[j];
	  }
	}
      }

      if(ctd >= 0 && ctd< gridres){
        azi2=0;
        n2=0;
	ctd2=ctd;
	for(int j=0;j<nlooks;j++){
	  if(nmeas[j]>0){
	    n2++;
	    azi2+=azim[j];
	  }
	}
	forward_direction=fabs(ctd2/gridres)*azi1/n1+fabs(ctd1/gridres)*azi2/n2;
      }
    }
    ncti=int((max_ctd-min_ctd)/gridres+0.5);
}

int
GeomNoiseFile::ReadLine(){
  char line[4096];
  char* str;

  int retval=1;
  while(1){
    if (fgets(line, 4096, ifp) != line){
      retval=0;
      break;
    }
    lineno++;
    if(line[0]=='\n') continue;    
    if (line[0]=='#') continue; // skip comments
    if(line[0]==' '){
      fprintf(stderr,"Error line %d in geomnoise file starts with a space character\n",lineno);
      exit(1);
    }
    else{
      str=strtok(line," \t\n");
      if(str==NULL) continue;  // skips blank line
      break; // finished reading line
    }

  } // end while loop

  // check for end of input file
  if(!retval) return(retval);

 

  //-----------------------------
  // Parse line
  //-----------------------------
  ctd=atof(str);
  for(int i=0;i<nlooks;i++){
    str=strtok(NULL," \t\n");
    nmeas[i]=atoi(str);

    str=strtok(NULL," \t\n");
    s0ne[i]=atof(str);
    s0ne[i]=pow(10,0.1*s0ne[i]);
    
    str=strtok(NULL," \t\n");
    nlpm[i]=atoi(str);

    str=strtok(NULL," \t\n");
    azim[i]=atof(str)*dtr;	
    
    str=strtok(NULL," \t\n");
    inc[i]=atof(str)*dtr;

    str=strtok(NULL," \t\n");
    pol[i]=str[0];

    str=strtok(NULL," \t\n");
    sambrat[i]=atof(str);
    sambrat[i]=pow(10,0.1*sambrat[i]);
	
  }
  return(1);
}

GeomNoiseFile::~GeomNoiseFile()
{
  fclose(ifp);
}
