//==============================================================//
// Copyright (C) 1997-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AngleInterval.h"
#include "List.h"
#include "Array.h"
#include "Misc.h"
#include "Wind.h"
#include "Constants.h"

//===============//
// AngleInterval //
//===============//

//----------------------------//
// Constructor and Destructor //
//----------------------------//

AngleInterval::AngleInterval()
:   left(0.0),right(0.0)
{
    return;
}

AngleInterval::~AngleInterval()
{
    return;
}

//-----------------------------//
// AngleInterval::SetLeftRight //
//-----------------------------//

int
AngleInterval::SetLeftRight(
    float  left_val,
    float  right_val)
{
    left = left_val;
    right = right_val;
    while (left < 0)
        left += two_pi;
    while (left > two_pi)
        left -= two_pi;
    while (right < 0)
        right += two_pi;
    while (right > two_pi)
        right -= two_pi;
    return(1);
}

//---------------------------------------//
// AngleInterval::GetEquallySpacedAngles //
//---------------------------------------//

int
AngleInterval::GetEquallySpacedAngles(
    int     num_angles,
    float*  angles)
{
    float width = GetWidth();
    float spacing = width / (num_angles + 1);
    for (int c = 0; c < num_angles; c++) {
        angles[c] = left + (c+1) * spacing;
        while(angles[c] < 0) {
            angles[c] += two_pi;
        }
        while(angles[c] > two_pi) {
            angles[c] -= two_pi;
        }
    }
    return(1);
}

//---------------------//
// AngleInterval::Read //
//---------------------//

int
AngleInterval::Read(
    FILE*  fp)
{
    if (fread((void*)&left, sizeof(float), 1, fp) != 1) {
        return(0);
    }
    if (fread((void*)&right, sizeof(float), 1, fp) != 1) {
        return(0);
    }
    return(1);
}

int
AngleInterval::Write(FILE* fp){
 if(fwrite((void*)&left,sizeof(float),1,fp)!=1) return(0);
 if(fwrite((void*)&right,sizeof(float),1,fp)!=1) return(0);
 return(1);
}
int
AngleInterval::WriteAscii(FILE* fp){
  fprintf(fp,"AngleInterval left=%g right=%g\n",left*rtd,right*rtd);
  return(1);
}

//------------------------------------//
// AngleIntervalList Methods          //
//------------------------------------//

//-------------------------------//
// Constructor and Destructor    //
//-------------------------------//
AngleIntervalList::AngleIntervalList(){
  return;
}

AngleIntervalList::~AngleIntervalList(){
  FreeContents();
  return;
}

void
AngleIntervalList::FreeContents(){
  AngleInterval* i;
  GotoHead();
  while((i = RemoveCurrent()) != NULL)
    delete i;
  return;
}

double
AngleIntervalList::GetNearestValue(double angle){
  double closest_dist=two_pi;
  double closest_angle=0;

  for(AngleInterval* ai=GetHead();ai;ai=GetNext()){
    if(ai->InRange(angle))return(angle);
    double tmp=ANGDIF(angle,ai->left);
    if(tmp<closest_dist){
      closest_dist=tmp;
      closest_angle=ai->left;
    }
    tmp=ANGDIF(angle,ai->right);
    if(tmp<closest_dist){
      closest_dist=tmp;
      closest_angle=ai->right;
    }
  }
  return(closest_angle);
}

int
AngleIntervalList::Write(FILE* fp){
  int node_count=NodeCount();
  if(node_count!=0){   // allows backward compatibility
    char magic[21]="Angle_Interval_Magic";
    if(fwrite((void *)&magic[0], sizeof(char), 21, fp) != 21)
        return(0);
    if(fwrite((void *)&node_count, sizeof(int),1, fp) != 1)
        return(0);
    for(AngleInterval* ai=GetHead();ai;ai=GetNext()){
      if(! ai->Write(fp)) return(0);
    }
    return(1);
  }
  else return(-1);
}

int
AngleIntervalList::WriteAscii(FILE* fp){
  int node_count=NodeCount();
  if(node_count!=0){   // allows backward compatibility
    fprintf(fp,"AngleIntervalList %d nodes\n",node_count);
    for(AngleInterval* ai=GetHead();ai;ai=GetNext()){
      if(! ai->WriteAscii(fp)) return(0);
    }
  }
  return(1);
}

// Returns 0 on I/O Error
// Returns -1 if No AngleIntervalList found
// Returns 1 if successful
int
AngleIntervalList::Read(FILE* fp){
  int node_count;
  char magic[21];
  fpos_t pos;
  if(fgetpos(fp,&pos)!=0)return(0);
  if(fread((void *)&magic[0], sizeof(char), 21, fp) != 21){
    if(feof(fp)) return(-1);
    else return(0);
  }
  if(strcmp(magic,"Angle_Interval_Magic")!=0){  // for backward compatibility
    if(fsetpos(fp,&pos)!=0)return(0);
    return(-1);
  }
  else{
    if(fread((void *)&node_count, sizeof(int),1, fp) != 1)
      return(0);
    for(int c=0;c<node_count;c++){
      AngleInterval* ai= new AngleInterval;
      if(! ai->Read(fp)) return(0);
      if(! Append(ai)) return(0);
    }
    return(1);
  }
}

//----------------------------------//
// Bisect All Intervals In List     //
//----------------------------------//

int AngleIntervalList::Bisect(){
 int original_num=NodeCount();
 AngleInterval* old_interval=GetHead();
 for(int i=0;i<original_num;i++,old_interval=GetNext()){
   float width=old_interval->GetWidth();
   AngleInterval* new_interval= new AngleInterval;
   *new_interval=*old_interval;
   float old_left=old_interval->left;
   float old_right=old_interval->right;
   float midpoint=old_left+width/2.0;
   old_interval->SetLeftRight(old_left, midpoint);
   new_interval->SetLeftRight(midpoint,old_right);
   Node<AngleInterval>* tmp=_current;
   if(!Append(new_interval)) return(0);
   _current=tmp;
 }
 return(1);
}

int
AngleIntervalList::GetPossiblePlacings(
       int     num_angles,
       int*    num_permutations,
       int***  num_placings){
  int num_intervals=NodeCount();
  // calculate upper bound on the number of permutations
  // (num_intervals + num_angles -1)!/(num_angles!*(num_intervals-1)!)
  int upper_bound=1;
  for(int c=num_intervals-1+num_angles;c>=num_intervals-1;c--) upper_bound*=c;
  for(int c=1;c<=num_angles;c++) upper_bound/=c;

  int** tmp_array=(int**)make_array(sizeof(float),2, upper_bound,
        num_intervals);
  if(!tmp_array){
    fprintf(stderr,"AngleIntervalList:GetPossiblePlacings:Unable to allocate tmp_array\n");
    exit(0);
  }
  *num_permutations=0;
  if(!_GetPossiblePlacings(num_angles,num_permutations,tmp_array))
    exit(0);
  *num_placings=(int**)make_array(sizeof(float),2, *num_permutations,
        num_intervals);
  if(!(*num_placings)){
    fprintf(stderr,"AngleIntervalList:GetPossiblePlacings:Unable to allocate array\n");
    exit(0);
  }
  for(int t=0;t<*num_permutations;t++){
    for(int i=0;i<num_intervals;i++){
      (*num_placings)[t][i]=tmp_array[t][i];
    }
  }
  free_array((void*)tmp_array,2,upper_bound,num_intervals);
  return(1);
}

//-----------------------------------------//
// AngleIntervalList::_GetPossiblePlacings //
//-----------------------------------------//

int
AngleIntervalList::_GetPossiblePlacings(
    int    num_angles_left,
    int*   num_permutations,
    int**  num_placings,
    int    interval_idx,
    int*   tmp_placings)
{
    int num_intervals = NodeCount();
    if (interval_idx == 0) {
        tmp_placings = new int[num_intervals];
        if (! tmp_placings)
            return(0);
    }
  if(interval_idx==num_intervals-1){
    for(int i=0;i<num_intervals-1;i++){
      num_placings[*num_permutations][i]=tmp_placings[i];
    }
    num_placings[*num_permutations][interval_idx]=num_angles_left;
    (*num_permutations)++;
  }

  else{
    for(int j=0;j<=num_angles_left;j++){
      tmp_placings[interval_idx]=j;
      _GetPossiblePlacings(num_angles_left-j,num_permutations,num_placings,
               interval_idx+1,tmp_placings);
    }
  }
  if(interval_idx==0) delete tmp_placings;
  return(1);
}

//--------------------------------------------//
// AngleIntervalListPlus Methods              //
//--------------------------------------------//

//-------------------------------//
// Constructor and Destructor    //
//-------------------------------//
AngleIntervalListPlus::AngleIntervalListPlus()
  : bestSpd(NULL), bestObj(NULL)
{
  return;
}

AngleIntervalListPlus::~AngleIntervalListPlus(){
  FreeContents();
  if(bestSpd!=NULL){
    free(bestSpd);
    bestSpd=NULL;
  }
  if(bestObj!=NULL) {
    free(bestObj);
    bestObj=NULL;
  }
  return;
}

int
AngleIntervalListPlus::Read(FILE* fp){
  int value=AngleIntervalList::Read(fp);
  if(value==0) return(0);
  else if(value==-1) return(-1);
  else if(!dirIdx.Read(fp)) return(0);
  unsigned int bins=dirIdx.GetBins();

  bestSpd=dirIdx.MakeFloatArray();
  if(!bestSpd) return(0);
  if(fread(&bestSpd[0],sizeof(float),bins,fp)!=bins) return(0);

  bestObj=dirIdx.MakeFloatArray();
  if(!bestObj) return(0);
  if(fread(&bestObj[0],sizeof(float),bins,fp)!=bins) return(0);

  return(1);
}

int
AngleIntervalListPlus::Write(FILE* fp){
  int value=AngleIntervalList::Write(fp);
  if(value!=1){
    if(bestSpd==NULL) return(value);
    else{
      char magic[21]="Angle_Interval_Magic";
      if(fwrite((void *)&magic[0], sizeof(char), 21, fp) != 21)
        return(0);
      int node_count=0;
      if(fwrite((void *)&node_count, sizeof(int),1, fp) != 1)
        return(0);
    }
  }
  if(!dirIdx.Write(fp)) return(0);
  unsigned int bins=dirIdx.GetBins();
  if(fwrite(&bestSpd[0],sizeof(float),bins,fp)!=bins) return(0);
  if(fwrite(&bestObj[0],sizeof(float),bins,fp)!=bins) return(0);
  return(1);
}

float
AngleIntervalListPlus::GetBestSpeed(double dir){
  int idx[2];
  float coeff[2];
  if(!bestSpd){
    fprintf(stderr,"AngleIntervalListPlus::Cannot get Best Speed\n");
    exit(1);
  }
  if(!dirIdx.GetLinearCoefsWrapped(dir,idx,coeff)){
    fprintf(stderr,"AngleIntervalListPlus::Cannot get Best Speed\n");
    exit(1);
  }
  return(coeff[0]*bestSpd[idx[0]]+coeff[1]*bestSpd[idx[1]]);
}

float
AngleIntervalListPlus::GetBestObj(double dir){
  int idx[2];
  float coeff[2];
  if(!bestObj){
    fprintf(stderr,"AngleIntervalListPlus::Cannot get Objective value\n");
    exit(1);
  }
  if(!dirIdx.GetLinearCoefsWrapped(dir,idx,coeff)){
    fprintf(stderr,"AngleIntervalListPlus::Cannot get Objective Value\n");
    exit(1);
  }
  return(coeff[0]*bestObj[idx[0]]+coeff[1]*bestObj[idx[1]]);
}


int
AngleIntervalListPlus::GetNearestVector(WindVectorPlus* wvp){
  //--------------------------------------------------------------//
  // Find closest sampled vector                                  //
  //--------------------------------------------------------------//
  float x,y, dirclose=0, trial_spd, trial_dir;
  float dx,dy;
  float sqrdist;
  float min_sqrdist=1000000.0;
  x=wvp->spd*cos(wvp->dir);
  y=wvp->spd*sin(wvp->dir);
  for(AngleInterval* ai=GetHead();ai;ai=GetNext()){
    // Consider all samples vectors in each interval (and endpoints)
    int sample_number=0;
    while(GetSampleDirection(sample_number,&trial_dir)){
      trial_spd=GetBestSpeed(trial_dir);
      dx=x-trial_spd*cos(trial_dir);
      dy=y-trial_spd*sin(trial_dir);
      sqrdist=dx*dx+dy*dy;
      if(sqrdist<min_sqrdist){
    dirclose=trial_dir;
        min_sqrdist=sqrdist;
      }
      sample_number++;
    }
  }
  // Assign the closest vector to wvp
  wvp->dir=dirclose;
  wvp->spd=GetBestSpeed(dirclose);
  wvp->obj=GetBestObj(dirclose);
  return(1);
}

//-------------------------------------------//
// AngleIntervalListPlus::GetSampleDirection //
//-------------------------------------------//

int
AngleIntervalListPlus::GetSampleDirection(
    int     samp_idx,
    float*  dir)
{
    // Get required constants
    AngleInterval* ai = GetCurrent();
    float step = dirIdx.GetStep();

    // Compute Width and Number of Samples
    float width;
    int num_samples;
    if (ai->left == ai->right)
    {
        width = 0;
        num_samples = 1;
    }
    else
    {
        width = ai->GetWidth();
        int first_idx = (int)(ai->left / step) + 1;
        int last_idx = (int)(ceil((ai->left+width) / step) - 1);
        int num_idx_in_range = last_idx - first_idx + 1;
        num_samples = 2 + num_idx_in_range;
    }

    // Compute Direction for various sample index values
    if (samp_idx < 0)
        return(0);
    else if (samp_idx == 0)
    {
        *dir = ai->left;
        return(1);
    }
    else if (samp_idx == num_samples - 1)
    {
        *dir = ai->right;
        return(1);
    }
    else if (samp_idx >= num_samples)
        return(0);
    else
    {
        int idx;
        dirIdx.GetNearestIndexWrapped(ai->left, &idx);
        float start = idx * step;
        if (! ai->InRange(start))
            start += step;
        *dir = start + samp_idx * step;
        if (*dir > two_pi)
            *dir -= two_pi;
        return(1);
    }
    return(0);    // should never get here
}

float
AngleIntervalListPlus::EstimateMSE(){
  Node<AngleInterval>* old = _current;
  float MSE=0;
  float step=dirIdx.GetStep();
  int bins=dirIdx.GetBins();

  for(int c=0;c<bins;c++){
    float dir=c*step;
    float dir2=GetNearestValue(dir);
    float tmp=ANGDIF(dir,dir2);
    MSE+=tmp*tmp*bestObj[c];
  }
  _current=old;
  return(MSE);
}


