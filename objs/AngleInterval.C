//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#include<stdio.h>
#include"AngleInterval.h"
#include"List.h"
#include"Array.h"
#include"Misc.h"
#include"Constants.h"

//------------------------------------//
// AngleInterval Methods              //
//------------------------------------//

//-------------------------------//
// Constructor and Destructor    //
//-------------------------------//

AngleInterval::AngleInterval()
  : left(0.0),right(0.0){
  return;
}

AngleInterval::~AngleInterval(){
  return;
}

//--- AngleInterval::SetLeftRight ----//
int AngleInterval::SetLeftRight(
            float       left_val,
            float       right_val){
  left=left_val;
  right=right_val;
  while(left<0) left+=two_pi;
  while(left>two_pi) left-=two_pi;
  while(right<0) right+=two_pi;
  while(right>two_pi) right-=two_pi;
  return(1);
}

//--- AngleInterval::GetEquallySpacedAngles ----//
int
AngleInterval::GetEquallySpacedAngles(
       int num_angles,
       float* angles){
  float width=GetWidth();
  float spacing=width/(num_angles+1);
  for(int c=0;c<num_angles;c++){
    angles[c]=left+(c+1)*spacing;
    while(angles[c]<0) angles[c]+=two_pi;
    while(angles[c]>two_pi) angles[c]-=two_pi;    
  }
  return(1);
}

int
AngleInterval::Read(FILE* fp){
 if(fread((void*)&left,sizeof(float),1,fp)!=1) return(0);
 if(fread((void*)&right,sizeof(float),1,fp)!=1) return(0);
 return(1);
}
int
AngleInterval::Write(FILE* fp){
 if(fwrite((void*)&left,sizeof(float),1,fp)!=1) return(0);
 if(fwrite((void*)&right,sizeof(float),1,fp)!=1) return(0);
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
  }
  return(1);
}

int 
AngleIntervalList::Read(FILE* fp){
  int node_count;
  char magic[21];
  fpos_t pos;
  if(fgetpos(fp,&pos)!=0)return(0);
  if(fread((void *)&magic[0], sizeof(char), 21, fp) != 21){
    if(feof(fp)) return(1);
    else return(0);
  }
  if(strcmp(magic,"Angle_Interval_Magic")!=0){  // for backward compatibility
    if(fsetpos(fp,&pos)!=0)return(0);
  }
  else{
    if(fread((void *)&node_count, sizeof(int),1, fp) != 1)
      return(0);
    for(int c=0;c<node_count;c++){
      AngleInterval* ai= new AngleInterval;
      if(! ai->Read(fp)) return(0);
      if(! Append(ai)) return(0);
    }
  }
  return(1);
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
  int upper_bound= num_intervals*num_intervals*num_intervals 
    -num_intervals*num_intervals + num_intervals;
  int tmp_int=1;
  for(int i=0;i<num_angles-2;i++) tmp_int*=num_intervals;
  upper_bound+=tmp_int;
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

int
AngleIntervalList::_GetPossiblePlacings(
	int    num_angles_left,
        int*   num_permutations,
	int**  num_placings,
	int    interval_idx=0,
	int*   tmp_placings=NULL){
  int num_intervals=NodeCount();
  if(interval_idx==0){
    tmp_placings=new int[num_intervals];
    if(!tmp_placings) return(0);
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
