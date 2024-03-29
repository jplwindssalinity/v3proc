#ifndef ANGLEINTERVAL_H
#define ANGLEINTERVAL_H
#include "List.h"
#include "Misc.h"
#include "Index.h"
#include<math.h>
#include"Constants.h"

class WindVectorPlus;

class AngleInterval{
 public:
  AngleInterval();
  ~AngleInterval();
  int Read(FILE* fp);
  int Write(FILE* fp);
  int WriteAscii(FILE* fp);
  int SetLeftRight(float left, float right);
  int GetEquallySpacedAngles(int num_angles, float* angles);
  float GetWidth(){return((left<right)?(right-left):(right+two_pi-left));}
  int InRange(float angle){ return(BETWEENANG(angle,left,right));}

  //variables

  float left;
  float right;
};

class AngleIntervalList : public List<AngleInterval>{
 public:
  AngleIntervalList();
  double GetNearestValue(double angle);
  ~AngleIntervalList();
  void FreeContents();
  int Read(FILE* fp);
  int Write(FILE* fp);
  int WriteAscii(FILE* fp);
  int Bisect();
  int GetPossiblePlacings(int num_angles, int* permutations, int*** num_placings);
 protected:
  int _GetPossiblePlacings(int num_angles_left, int* num_permutations,
			   int** num_placings, int interval_idx=0,
			   int* tmp_placings=NULL);
};

class AngleIntervalListPlus : public AngleIntervalList{

 public:
   AngleIntervalListPlus();
   ~AngleIntervalListPlus();
  int Read(FILE* fp);
  int Write(FILE* fp);
  int GetNearestVector(WindVectorPlus* wvp);
  int GetSampleDirection(int sample_idx, float* dir);
  float GetBestSpeed(double dir); 
  float GetBestObj(double dir);
  float EstimateMSE();
  
  //  variables
  Index dirIdx;
  float* bestSpd;
  float* bestObj;
};
#endif







