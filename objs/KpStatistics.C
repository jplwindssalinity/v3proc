//==============================================================//
// Copyright (C) 2001, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
// Author:  Bryan Stiles  Bryan.W.Stiles@jpl.nasa.gov           //
//==============================================================//

static const char rcs_id_kp_c[] =
    "@(#) $Id$";

#include "KpStatistics.h"
#include "Array.h"

//==============//
// KpStatistics //
//==============//

KpStatistics::KpStatistics()
  : numMeas(NULL),numLook(NULL),sumVpcTheor(NULL),sumVpiEst(NULL),
    sumVpiWGEst(NULL),sumVpEst(NULL)
{
  return;
}

KpStatistics::~KpStatistics(){
  if(numMeas!=NULL) Deallocate();
  numMeas=NULL;
  return;
}

//------------------------------//
// KpStatistics::Allocate       //
//------------------------------//

int KpStatistics::Allocate(){
  if(numMeas!=NULL){
    return(0);
  }
  numMeas=(int******)make_array(sizeof(int),6,beamIdx.GetBins(),
				aTIdx.GetBins(),cTIdx.GetBins(),
				scanAngleIdx.GetBins(),spdIdx.GetBins(),
				chiIdx.GetBins());

  numLook=(int******)make_array(sizeof(int),6,beamIdx.GetBins(),
				aTIdx.GetBins(),cTIdx.GetBins(),
				scanAngleIdx.GetBins(),spdIdx.GetBins(),
				chiIdx.GetBins());

  sumVpcTheor=(float******)make_array(sizeof(float),6,beamIdx.GetBins(),
				aTIdx.GetBins(),cTIdx.GetBins(),
				scanAngleIdx.GetBins(),spdIdx.GetBins(),
				chiIdx.GetBins());

  sumVpiEst=(float******)make_array(sizeof(float),6,beamIdx.GetBins(),
				aTIdx.GetBins(),cTIdx.GetBins(),
				scanAngleIdx.GetBins(),spdIdx.GetBins(),
				chiIdx.GetBins());

  sumVpiWGEst=(float******)make_array(sizeof(float),6,beamIdx.GetBins(),
				aTIdx.GetBins(),cTIdx.GetBins(),
				scanAngleIdx.GetBins(),spdIdx.GetBins(),
				chiIdx.GetBins());

  sumVpEst=(float******)make_array(sizeof(float),6,beamIdx.GetBins(),
				aTIdx.GetBins(),cTIdx.GetBins(),
				scanAngleIdx.GetBins(),spdIdx.GetBins(),
				chiIdx.GetBins());
  if(!numMeas || !numLook || !sumVpcTheor || !sumVpiEst || !sumVpiWGEst ||
     !sumVpEst)
    return(0);
  
  /*** Initialize Arrays ***/
  for(int b=0;b<beamIdx.GetBins();b++){
    for(int ati=0;ati<aTIdx.GetBins();ati++){
      for(int cti=0;cti<cTIdx.GetBins();cti++){
	for(int scan=0;scan<scanAngleIdx.GetBins();scan++){
	  for(int spd=0;spd<spdIdx.GetBins();spd++){
	    for(int chi=0;chi<chiIdx.GetBins();chi++){
	      numMeas[b][ati][cti][scan][spd][chi]=0;
	      numLook[b][ati][cti][scan][spd][chi]=0;
	      sumVpcTheor[b][ati][cti][scan][spd][chi]=0;
	      sumVpiEst[b][ati][cti][scan][spd][chi]=0;
	      sumVpiWGEst[b][ati][cti][scan][spd][chi]=0;
	      sumVpEst[b][ati][cti][scan][spd][chi]=0;
	    }
	  }
	}
      }
    }
  }
  return(1);
}

//------------------------------//
// KpStatistics::Deallocate     //
//------------------------------//

int KpStatistics::Deallocate(){
  free_array((void*)numMeas,6,beamIdx.GetBins(),aTIdx.GetBins(),
	     cTIdx.GetBins(),scanAngleIdx.GetBins(),spdIdx.GetBins(),
	     chiIdx.GetBins());
  free_array((void*)numLook,6,beamIdx.GetBins(),aTIdx.GetBins(),
	     cTIdx.GetBins(),scanAngleIdx.GetBins(),spdIdx.GetBins(),
	     chiIdx.GetBins());
  free_array((void*)sumVpcTheor,6,beamIdx.GetBins(),aTIdx.GetBins(),
	     cTIdx.GetBins(),scanAngleIdx.GetBins(),spdIdx.GetBins(),
	     chiIdx.GetBins());
  free_array((void*)sumVpiEst,6,beamIdx.GetBins(),aTIdx.GetBins(),
	     cTIdx.GetBins(),scanAngleIdx.GetBins(),spdIdx.GetBins(),
	     chiIdx.GetBins());
  free_array((void*)sumVpiWGEst,6,beamIdx.GetBins(),aTIdx.GetBins(),
	     cTIdx.GetBins(),scanAngleIdx.GetBins(),spdIdx.GetBins(),
	     chiIdx.GetBins());
  free_array((void*)sumVpEst,6,beamIdx.GetBins(),aTIdx.GetBins(),
	     cTIdx.GetBins(),scanAngleIdx.GetBins(),spdIdx.GetBins(),
	     chiIdx.GetBins());

  return(1);
}

//--------------------//
// KpStatistics::Read //
//--------------------//
//--------------------------------------//
// Returns 0 for I/O Error              //
//         1 for good read              //
//         -1 for Index mismatch        //
//         (Indices do NOT match        //
//          previous non-zero settings) //
//--------------------------------------//
int
KpStatistics::Read(const char* filename){
  int retval=1;
  FILE* ifp=fopen(filename,"r");
  if(ifp==NULL) return(0);
  /*** If Indices have not been set, set them **/ 
  if(beamIdx.GetBins()==0){
    if(!beamIdx.Read(ifp)) return(0);
    if(!aTIdx.Read(ifp)) return(0);
    if(!cTIdx.Read(ifp)) return(0);
    if(!scanAngleIdx.Read(ifp)) return(0);
    if(!spdIdx.Read(ifp)) return(0);
    if(!chiIdx.Read(ifp)) return(0);
  }
  /**** Otherwise check to make sure they are the same ***/
  else{
    Index tmp;

    if(!tmp.Read(ifp)) return(0);
    if(tmp!=beamIdx) retval=-1;

    if(!tmp.Read(ifp)) return(0);
    if(tmp!=aTIdx) retval=-1;

    if(!tmp.Read(ifp)) return(0);
    if(tmp!=cTIdx) retval=-1;

    if(!tmp.Read(ifp)) return(0);
    if(tmp!=scanAngleIdx) retval=-1;

    if(!tmp.Read(ifp)) return(0);
    if(tmp!=spdIdx) retval=-1;

    if(!tmp.Read(ifp)) return(0);
    if(tmp!=chiIdx) retval=-1;
  }

  /**** Deallocate arrays if necessary ***/
  if(numMeas) Deallocate();
  
  /**** Allocate arrays ****/
  Allocate();
  
  /*** Read Arrays ***/
  for(int b=0;b<beamIdx.GetBins();b++){
    for(int ati=0;ati<aTIdx.GetBins();ati++){
      for(int cti=0;cti<cTIdx.GetBins();cti++){
	for(int scan=0;scan<scanAngleIdx.GetBins();scan++){
	  for(int spd=0;spd<spdIdx.GetBins();spd++){
	    int num_read=fread((void*)&(numMeas[b][ati][cti][scan][spd][0]),
			       sizeof(int),chiIdx.GetBins(),ifp);
	    if(num_read!=chiIdx.GetBins()) return(0);
	  }
	}
      }
    }
  }


  for(int b=0;b<beamIdx.GetBins();b++){
    for(int ati=0;ati<aTIdx.GetBins();ati++){
      for(int cti=0;cti<cTIdx.GetBins();cti++){
	for(int scan=0;scan<scanAngleIdx.GetBins();scan++){
	  for(int spd=0;spd<spdIdx.GetBins();spd++){
	    int num_read=fread((void*)&(numLook[b][ati][cti][scan][spd][0]),
			       sizeof(int),chiIdx.GetBins(),ifp);
	    if(num_read!=chiIdx.GetBins()) return(0);
	  }
	}
      }
    }
  }

  for(int b=0;b<beamIdx.GetBins();b++){
    for(int ati=0;ati<aTIdx.GetBins();ati++){
      for(int cti=0;cti<cTIdx.GetBins();cti++){
	for(int scan=0;scan<scanAngleIdx.GetBins();scan++){
	  for(int spd=0;spd<spdIdx.GetBins();spd++){
	   int num_read=fread((void*)&(sumVpcTheor[b][ati][cti][scan][spd][0]),
			       sizeof(float),chiIdx.GetBins(),ifp);
	    if(num_read!=chiIdx.GetBins()) return(0);
	  }
	}
      }
    }
  }

  for(int b=0;b<beamIdx.GetBins();b++){
    for(int ati=0;ati<aTIdx.GetBins();ati++){
      for(int cti=0;cti<cTIdx.GetBins();cti++){
	for(int scan=0;scan<scanAngleIdx.GetBins();scan++){
	  for(int spd=0;spd<spdIdx.GetBins();spd++){
	    int num_read=fread((void*)&(sumVpiEst[b][ati][cti][scan][spd][0]),
			       sizeof(float),chiIdx.GetBins(),ifp);
	    if(num_read!=chiIdx.GetBins()) return(0);
	  }
	}
      }
    }
  }

  for(int b=0;b<beamIdx.GetBins();b++){
    for(int ati=0;ati<aTIdx.GetBins();ati++){
      for(int cti=0;cti<cTIdx.GetBins();cti++){
	for(int scan=0;scan<scanAngleIdx.GetBins();scan++){
	  for(int spd=0;spd<spdIdx.GetBins();spd++){
	   int num_read=fread((void*)&(sumVpiWGEst[b][ati][cti][scan][spd][0]),
			       sizeof(float),chiIdx.GetBins(),ifp);
	    if(num_read!=chiIdx.GetBins()) return(0);
	  }
	}
      }
    }
  }

  for(int b=0;b<beamIdx.GetBins();b++){
    for(int ati=0;ati<aTIdx.GetBins();ati++){
      for(int cti=0;cti<cTIdx.GetBins();cti++){
	for(int scan=0;scan<scanAngleIdx.GetBins();scan++){
	  for(int spd=0;spd<spdIdx.GetBins();spd++){
	    int num_read=fread((void*)&(sumVpEst[b][ati][cti][scan][spd][0]),
			       sizeof(float),chiIdx.GetBins(),ifp);
	    if(num_read!=chiIdx.GetBins()) return(0);
	  }
	}
      }
    }
  }
  fclose(ifp);
  return(retval);
}

//---------------------//
// KpStatistics::Write //
//---------------------//

int
KpStatistics::Write(const char* filename){
  FILE* ofp=fopen(filename,"w");
  if(ofp==NULL) return(0);

  /*** Write Indices **/ 
  if(!beamIdx.Write(ofp)) return(0);
  if(!aTIdx.Write(ofp)) return(0);
  if(!cTIdx.Write(ofp)) return(0);
  if(!scanAngleIdx.Write(ofp)) return(0);
  if(!spdIdx.Write(ofp)) return(0);
  if(!chiIdx.Write(ofp)) return(0);

  /*** Write Arrays ***/
  if(!numMeas) return(0); // fails if arrays unallocated

  for(int b=0;b<beamIdx.GetBins();b++){
    for(int ati=0;ati<aTIdx.GetBins();ati++){
      for(int cti=0;cti<cTIdx.GetBins();cti++){
	for(int scan=0;scan<scanAngleIdx.GetBins();scan++){
	  for(int spd=0;spd<spdIdx.GetBins();spd++){
	    int num_wrote=fwrite((void*)&(numMeas[b][ati][cti][scan][spd][0]),
			       sizeof(int),chiIdx.GetBins(),ofp);
	    if(num_wrote!=chiIdx.GetBins()) return(0);
	  }
	}
      }
    }
  }

  for(int b=0;b<beamIdx.GetBins();b++){
    for(int ati=0;ati<aTIdx.GetBins();ati++){
      for(int cti=0;cti<cTIdx.GetBins();cti++){
	for(int scan=0;scan<scanAngleIdx.GetBins();scan++){
	  for(int spd=0;spd<spdIdx.GetBins();spd++){
	    int num_wrote=fwrite((void*)&(numLook[b][ati][cti][scan][spd][0]),
			       sizeof(int),chiIdx.GetBins(),ofp);
	    if(num_wrote!=chiIdx.GetBins()) return(0);
	  }
	}
      }
    }
  }

  for(int b=0;b<beamIdx.GetBins();b++){
    for(int ati=0;ati<aTIdx.GetBins();ati++){
      for(int cti=0;cti<cTIdx.GetBins();cti++){
	for(int scan=0;scan<scanAngleIdx.GetBins();scan++){
	  for(int spd=0;spd<spdIdx.GetBins();spd++){
	 int num_wrote=fwrite((void*)&(sumVpcTheor[b][ati][cti][scan][spd][0]),
			       sizeof(float),chiIdx.GetBins(),ofp);
	    if(num_wrote!=chiIdx.GetBins()) return(0);
	  }
	}
      }
    }
  }

  for(int b=0;b<beamIdx.GetBins();b++){
    for(int ati=0;ati<aTIdx.GetBins();ati++){
      for(int cti=0;cti<cTIdx.GetBins();cti++){
	for(int scan=0;scan<scanAngleIdx.GetBins();scan++){
	  for(int spd=0;spd<spdIdx.GetBins();spd++){
	   int num_wrote=fwrite((void*)&(sumVpiEst[b][ati][cti][scan][spd][0]),
			       sizeof(float),chiIdx.GetBins(),ofp);
	    if(num_wrote!=chiIdx.GetBins()) return(0);
	  }
	}
      }
    }
  }

  for(int b=0;b<beamIdx.GetBins();b++){
    for(int ati=0;ati<aTIdx.GetBins();ati++){
      for(int cti=0;cti<cTIdx.GetBins();cti++){
	for(int scan=0;scan<scanAngleIdx.GetBins();scan++){
	  for(int spd=0;spd<spdIdx.GetBins();spd++){
	 int num_wrote=fwrite((void*)&(sumVpiWGEst[b][ati][cti][scan][spd][0]),
			       sizeof(float),chiIdx.GetBins(),ofp);
	    if(num_wrote!=chiIdx.GetBins()) return(0);
	  }
	}
      }
    }
  }

  for(int b=0;b<beamIdx.GetBins();b++){
    for(int ati=0;ati<aTIdx.GetBins();ati++){
      for(int cti=0;cti<cTIdx.GetBins();cti++){
	for(int scan=0;scan<scanAngleIdx.GetBins();scan++){
	  for(int spd=0;spd<spdIdx.GetBins();spd++){
	    int num_wrote=fwrite((void*)&(sumVpEst[b][ati][cti][scan][spd][0]),
			       sizeof(float),chiIdx.GetBins(),ofp);
	    if(num_wrote!=chiIdx.GetBins()) return(0);
	  }
	}
      }
    }
  }
  fclose(ofp);
  return(1);
}

//--------------------------//
// KpStatistics::WriteXmgr //
//--------------------------//
int
KpStatistics::WriteXmgr(const char* filename){

  FILE* ofp=fopen(filename,"w");
  if(ofp==NULL) return(0);

  float aval,cval,lval,sval,xval;
  for(int b=0;b<beamIdx.GetBins();b++){
    for(int a=0;a<aTIdx.GetBins();a++){
      aTIdx.IndexToValue(a,&aval);
      for(int c=0;c<cTIdx.GetBins();c++){
	cTIdx.IndexToValue(c,&cval);
	for(int l=0;l<scanAngleIdx.GetBins();l++){
	  scanAngleIdx.IndexToValue(l,&lval);
	  for(int x=0;x<chiIdx.GetBins();x++){
	    chiIdx.IndexToValue(x,&xval);
	    for(int s=0;s<spdIdx.GetBins();s++){
	      spdIdx.IndexToValue(s,&sval);
	      ComputeKp(b,a,c,l,s,x);
	      fprintf(ofp,"%g %g %g %g %g %g %g %g %d %d %d %g %g %g %g\n",sval,_kp,_kpi,_kpiWG,_kpc,_kpr,_kpm,_kpWG,_n,_nl,b,aval,cval,lval, xval);
	    }
	    fprintf(ofp,"&\n");
	  }
	}
      }
    }
  }
  fclose(ofp);
  return(1);
}

//--------------------------//
// KpStatistics::WriteKpmu //
//--------------------------//
int
KpStatistics::WriteKpmu(const char* filename){
  FILE* ofp=fopen(filename,"w");
  if(ofp==NULL) return(0);
  if (! spdIdx.Write(ofp)) return(0);
  for (int i = 0; i < beamIdx.GetBins(); i++){
    for (int ispd=0; ispd<spdIdx.GetBins();ispd++){
      // Kpm Table is non-standard stored as OUTER,INNER
      ComputeKpmu(beamIdx.GetBins()-1-i,ispd); 
      if (fwrite((void *)&_kpmu, sizeof(float), 1, ofp) != 1)
        {
	  return(0);
        }
    }
  }
  fclose(ofp);
  return(1);
}

//--------------------------//
// KpStatistics::WriteAscii //
//--------------------------//
int
KpStatistics::WriteAscii(const char* filename){
  FILE* ofp=fopen(filename,"w");
  if(ofp==NULL) return(0);

  fprintf(ofp,"Beams: ");
  for(int c=0;c<beamIdx.GetBins();c++){
    float value;
    beamIdx.IndexToValue(c,&value);
    fprintf(ofp,"%d ",(int)value);
  }
  fprintf(ofp,"\n");

  fprintf(ofp,"ATI: ");
  for(int c=0;c<aTIdx.GetBins();c++){
    float value;
    aTIdx.IndexToValue(c,&value);
    fprintf(ofp,"%8.1f ",value);
  }
  fprintf(ofp,"\n");

  fprintf(ofp,"CTI: ");
  for(int c=0;c<cTIdx.GetBins();c++){
    float value;
    cTIdx.IndexToValue(c,&value);
    fprintf(ofp,"%6.1f ",value);
  }
  fprintf(ofp,"\n");

  fprintf(ofp,"Scan Angle(rads): ");
  for(int c=0;c<scanAngleIdx.GetBins();c++){
    float value;
    scanAngleIdx.IndexToValue(c,&value);
    fprintf(ofp,"%5.2f ",value);
  }
  fprintf(ofp,"\n");

  fprintf(ofp,"Speed (m/s): ");
  for(int c=0;c<spdIdx.GetBins();c++){
    float value;
    spdIdx.IndexToValue(c,&value);
    fprintf(ofp,"%6.1f ",value);
  }
  fprintf(ofp,"\n");

  fprintf(ofp,"Rel. Azimuth(rads): ");
  for(int c=0;c<chiIdx.GetBins();c++){
    float value;
    chiIdx.IndexToValue(c,&value);
    fprintf(ofp,"%5.2f ",value);
  }
  fprintf(ofp,"\n\n");

  float amin,amax,cmin,cmax,lmin,lmax,smin,smax,xmin,xmax;
  for(int b=0;b<beamIdx.GetBins();b++){
    for(int a=0;a<aTIdx.GetBins();a++){
      aTIdx.IndexToRange(a,&amin,&amax);
      for(int c=0;c<cTIdx.GetBins();c++){
	cTIdx.IndexToRange(c,&cmin,&cmax);
	for(int l=0;l<scanAngleIdx.GetBins();l++){
	  scanAngleIdx.IndexToRange(l,&lmin,&lmax);
	  for(int s=0;s<spdIdx.GetBins();s++){
	    spdIdx.IndexToRange(s,&smin,&smax);
	    for(int x=0;x<chiIdx.GetBins();x++){
	      chiIdx.IndexToRange(x,&xmin,&xmax);
	      fprintf(ofp,"\nBeam %d, ati %8.1f-%8.1f, cti %6.1f-%6.1f\n",b,amin,amax,cmin,cmax);
	      fprintf(ofp, "ScanAng %5.2f-%5.2f, Speed %6.1f-%6.1f, Chi %5.2f-%5.2f\n",lmin,lmax,smin,smax,xmin,xmax);

	      ComputeKp(b,a,c,l,s,x);
	      fprintf(ofp,"kpc=%g,kpr=%g,kpm=%g,kpWG=%g,kpiWG=%g,kpi=%g,kp=%g,num=%d,numLook=%d\n\n",_kpc,_kpr,_kpm,_kpWG,_kpiWG,_kpi,_kp,_n,_nl);
	    }
	  }
	}
      }
    }
  }
  fclose(ofp);
  return(1);
}


#define DEBUG 0
//----------------------//
// KpStatistics::Update //
//----------------------//

int
KpStatistics::Update( int ati,
	     int cti,
	     MeasList* meas_list,
	     WVC* wvc,
	     WGC* wgc,
	     GMF* gmf,
	     windTypeE wind_type=DIRTH)
{
  //--------------------------------------------------------//
  // Initialize look set dependent arrays                   //
  // Right now (2) Beams and (2) Scan Angle Bins (fore/aft) //
  // are presumed. This blantant hack is tested by making   //
  // sure the hacked values match the stored numbers        //
  //--------------------------------------------------------//

#define NUM_BEAMS  2
#define NUM_LOOKS  2

  if(beamIdx.GetBins()!=NUM_BEAMS || 
     scanAngleIdx.GetBins()!=NUM_LOOKS){
    
    fprintf(stderr,"KpStatistics::Update failed due to HACK\n");
    return(0);
  }

  float lookset_diff_var[NUM_BEAMS][NUM_LOOKS]={{0,0},{0,0}};
  float lookset_diff_mean[NUM_BEAMS][NUM_LOOKS]={{0,0},{0,0}};
  float lookset_diff_WG_var[NUM_BEAMS][NUM_LOOKS]={{0,0},{0,0}};
  float lookset_diff_WG_mean[NUM_BEAMS][NUM_LOOKS]={{0,0},{0,0}};
  float lookset_sum_vpc[NUM_BEAMS][NUM_LOOKS]={{0,0},{0,0}};
  float lookset_mean_chi_cos[NUM_BEAMS][NUM_LOOKS]={{0,0},{0,0}};
  float lookset_mean_chi_sin[NUM_BEAMS][NUM_LOOKS]={{0,0},{0,0}};
  int lookset_num[NUM_BEAMS][NUM_LOOKS]={{0,0},{0,0}};

  //--------------------------------------------//
  // Compute non-measurement independent indices//
  //--------------------------------------------//

  int ispd,ict,iat;



  WindVector* wv=NULL;  
  switch(wind_type){
  case NCEP:
    wv=wvc->nudgeWV;
    // check for bad NCEP data
    if(wv->spd==0 && wv->dir==0) return(1);
    break;
  case DIRTH:
    wv=wvc->specialVector;
    break;
  case SELECTED:
    wv=wvc->selected;
    break;
  case FIRSTRANK:
    wv=wvc->ambiguities.GetHead();
    break;
  }

  // If speed is outside of range exit normally
  float spd=wv->spd;
  if(!spdIdx.GetNearestIndexStrict(spd,&ispd)) return(1);

  // If CTI or ATI is outside of range something weird is going on ... //
  if(!cTIdx.GetNearestIndexStrict(cti,&ict) ||
     !aTIdx.GetNearestIndexStrict(ati,&iat)){
    fprintf(stderr,"KpStatistics::Update-- CTI or ATI Out of Bounds Huh??\n");
    exit(1);
  }

  //-----------------------------//
  // Loop through measurements   //
  //-----------------------------//

  for(Meas* meas=meas_list->GetHead();meas;meas=meas_list->GetNext()){
    // compute measurement dependent indices
    int ibeam,ilook;
    if(!beamIdx.GetNearestIndexStrict(meas->beamIdx,&ibeam) ||
     !scanAngleIdx.GetNearestIndexStrict(meas->scanAngle,&ilook)){
      fprintf(stderr,"KpStatistics::Update-- Beam No. or Scan Angle Out of Bounds Huh??\n");
      exit(1);
    }
    float phi= wv->dir;
    float chi= phi - meas->eastAzimuth + pi;

    LonLat pos;
    pos.Set(meas->centroid);
    wgc->GetWindVector(wv,pos);
    float mspd=wv->spd;
    float mphi=wv->dir;
    float mchi= mphi - meas->eastAzimuth + pi;
    
    
    //----------------------------------------------//
    // Calculate error associated with measurement  //
    //----------------------------------------------//

    float s0=meas->value;
    float s0_calc, s0_calc_WG;
    gmf->GetInterpolatedValue(meas->measType, meas->incidenceAngle, spd, chi,
            &s0_calc);
    gmf->GetInterpolatedValue(meas->measType, meas->incidenceAngle, mspd, mchi,
            &s0_calc_WG);
    float err = (s0-s0_calc)/s0_calc;
    float err_WG = (s0-s0_calc_WG)/s0_calc_WG;

    //-----------------------------------//
    // compute vpc                       //
    //-----------------------------------//

    // These 2 lines of code are fragile because it assumes
    // 1) the A B and C terms were obtained from a GDS L2A file
    // and 2) kpr used by the ground system was zero.
    float alpha = meas->A-1;
    float kpc2 = alpha  + meas->B/s0_calc + meas->C/s0_calc/s0_calc;


    //-----------------------------------//
    // Update look set arrays            //
    //-----------------------------------//
    
    lookset_diff_var[ibeam][ilook]+=err*err;
    lookset_diff_mean[ibeam][ilook]+=err;
    lookset_diff_WG_var[ibeam][ilook]+=err_WG*err_WG;
    lookset_diff_WG_mean[ibeam][ilook]+=err_WG;
    lookset_sum_vpc[ibeam][ilook]+=kpc2;
    lookset_mean_chi_cos[ibeam][ilook]+=cos(chi);
    lookset_mean_chi_sin[ibeam][ilook]+=sin(chi);
    lookset_num[ibeam][ilook]++;
  }
  //-----------------------------//
  // Loop through look sets      //
  //-----------------------------//
  for(int ibeam=0;ibeam<beamIdx.GetBins();ibeam++){
    for(int ilook=0;ilook<scanAngleIdx.GetBins();ilook++){

      // if Look Set is too small then skip it 
      int n=lookset_num[ibeam][ilook];
      if(n<3) continue;

      //----------------------------------------//
      // Complete look set array calculations   //
      //----------------------------------------//
      float diff_mean=lookset_diff_mean[ibeam][ilook]/n;
      float diff_var=(lookset_diff_var[ibeam][ilook]-n*diff_mean*diff_mean)
	/(n-1);
      float diff_WG_mean=lookset_diff_WG_mean[ibeam][ilook]/n;
      float diff_WG_var=(lookset_diff_WG_var[ibeam][ilook]-
		      n*diff_WG_mean*diff_WG_mean)
	/(n-1);

      if(diff_WG_var>3*diff_var) continue; // omit outliers 

      //-------- Start debugging stuff -------------------//
      // static int instance_no=0;
      // if(ispd==3){
      //	printf("%d %f %f\n",instance_no,diff_var,diff_WG_var);
      //  instance_no++;
      // }
      //--------- End Debugging Stuff --------------------//

      float chi_cos=lookset_mean_chi_cos[ibeam][ilook]/n;
      float chi_sin=lookset_mean_chi_sin[ibeam][ilook]/n;
      float chi=(float)atan2(chi_sin,chi_cos);

      // Get chi index
      int ichi;
      if(!chiIdx.GetNearestIndexWrapped(chi,&ichi)){
	fprintf(stderr,"KpStatistics::Update-- Get Chi Index failed Huh??\n");
	exit(1);
      } 
    


      //--------------------------------------//
      // Update Variance and Count arrays     //
      //--------------------------------------//
      sumVpcTheor[ibeam][iat][ict][ilook][ispd][ichi]+=
	lookset_sum_vpc[ibeam][ilook];
      sumVpEst[ibeam][iat][ict][ilook][ispd][ichi]+=
	lookset_diff_var[ibeam][ilook];
      numMeas[ibeam][iat][ict][ilook][ispd][ichi]+=n;
      numLook[ibeam][iat][ict][ilook][ispd][ichi]++;
      sumVpiEst[ibeam][iat][ict][ilook][ispd][ichi]+=diff_var;
      sumVpiWGEst[ibeam][iat][ict][ilook][ispd][ichi]+=diff_WG_var;
    }
  }

  return(1);
}
int
KpStatistics::ComputeKpmu(int beam,
 			int ispd)
{
  
  int n=0;
  int nl=0;
  float vpc=0.0;
  float vp=0.0;
  for(int iat=0;iat<aTIdx.GetBins();iat++){
      for(int ict=0;ict<cTIdx.GetBins();ict++){
	for(int ilook=0;ilook<scanAngleIdx.GetBins();ilook++){
	  for(int ichi=0;ichi<chiIdx.GetBins();ichi++){
	    n+=numMeas[beam][iat][ict][ilook][ispd][ichi];
	    nl+=numLook[beam][iat][ict][ilook][ispd][ichi];
            vpc+=sumVpcTheor[beam][iat][ict][ilook][ispd][ichi];
            vp+=sumVpEst[beam][iat][ict][ilook][ispd][ichi];
	  }
	}
      }
  }
  int degrees_of_freedom=2; // number of degrees of freedom in point-wise
                            // wind estimation

  int nWVC=nl/2; // for each beam there are two looks per WVC
  int total_num_in_WVC=0;
  for(int iat=0;iat<aTIdx.GetBins();iat++){
    for(int ict=0;ict<cTIdx.GetBins();ict++){
      for(int b=0;b<beamIdx.GetBins();b++){
	for(int l=0;l<scanAngleIdx.GetBins();l++){
	  for(int ichi=0;ichi<chiIdx.GetBins();ichi++){
	    total_num_in_WVC+=numMeas[b][iat][ict][l][ispd][ichi]; 
	  }
	}
      }
    }
  }

  float fraction_meas_in_beam=(float)n/(float)total_num_in_WVC;
  float coeff= degrees_of_freedom*fraction_meas_in_beam;
  vp=vp/(n-coeff*nWVC); 
  vpc=vpc/n;
  _kpmu=(1+vp)/(1+vpc)-1;
  _kpmu=sqrt(_kpmu);
  return(1);
}	
     
int
KpStatistics::ComputeKp(int beam,
			int iat,
			int ict,
			int ilook,
			int ispd,
			int ichi)
{
  float vpc=sumVpcTheor[beam][iat][ict][ilook][ispd][ichi];
  float vpi=sumVpiEst[beam][iat][ict][ilook][ispd][ichi];
  float vpiWG=sumVpiWGEst[beam][iat][ict][ilook][ispd][ichi];
  float vp=sumVpEst[beam][iat][ict][ilook][ispd][ichi];
  _n=numMeas[beam][iat][ict][ilook][ispd][ichi];
  _nl=numLook[beam][iat][ict][ilook][ispd][ichi];
  int degrees_of_freedom=2; // number of degrees of freedom in point-wise
                            // wind estimation

  int total_num_in_WVC=0;
  for(int b=0;b<beamIdx.GetBins();b++){
    for(int l=0;l<scanAngleIdx.GetBins();l++){
     total_num_in_WVC+=numMeas[b][iat][ict][l][ispd][ichi]; 
    }
  }
  float fraction_meas_in_lookset=(float)_n/(float)total_num_in_WVC;
  float coeff= degrees_of_freedom*fraction_meas_in_lookset;
  vpc=vpc/_n;
  vpi=vpi/_nl;
  vpiWG=vpiWG/_nl;
  vp=vp/(_n-coeff*_nl); 

  _kpi=sqrt(vpi);
  _kpi=10*log10(1+_kpi);

  _kpiWG=sqrt(vpiWG);
  _kpiWG=10*log10(1+_kpiWG);

  _kp=sqrt(vp);
  _kp=10*log10(1+_kp);

  _kpmu=(1+vp)/(1+vpc)-1;
  _kpmu=sqrt(_kpmu);

  _kpm=(1+vp)/(1+vpiWG)-1;
  if(_kpm>0) {
    _kpm=sqrt(_kpm);
    _kpm=10*log10(1+_kpm);
  }
  else{
    _kpm= sqrt(-_kpm);
    _kpm=-10*log10(1+_kpm);
  }
  _kpr=(1+vpiWG)/(1+vpc)-1;
  if(_kpr>0){
    _kpr=sqrt(_kpr);
    _kpr=10*log10(1+_kpr);
  }
  else{
    _kpr=(sqrt(-_kpr));
    _kpr=-10*log10(1+_kpr);
  }

  _kpWG=(1+vpi)/(1+vpiWG)-1;
  if(_kpWG>0){
    _kpWG=sqrt(_kpWG);
    _kpWG=10*log10(1+_kpWG);
  }
  else{
    _kpWG=(sqrt(-_kpWG));
    _kpWG=-10*log10(1+_kpWG);
  }

  _kpc=sqrt(vpc);
  _kpc=10*log10(1+_kpc);

  return(1);
}




