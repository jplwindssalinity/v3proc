#ifndef MLP_H
#define MLP_H

#include "MLPData.h"

/***** Multi-Layer Perceptron Structure ***/
class MLP{
  
public:

  /**** variables ***/

  int nin; /*** number of inputs ***/
  int nout; /*** number of outputs ***/
  int hn; /*** number of hidden units (must be a multiple of (nhlayers) ***/
  int nhlayers;
  int outputSigmoidFlag;
  float** htab; /** table used by VSS **/
  float** win;/*** weights between input units and hidden units ***/
  float** dwin;/*** previous weight changes ***/
       /***        (indices (hidden node number, input node number))***/


  float** whid;/*** weights between hidden and output units **/
  float** dwhid;/*** previous weight changes ***/
       /*** (indices (out# hidden #) ****/

  float* outp; /*** outputs  by output number***/ 
  float* hnout;/*** hidden unit outputs by hn number **/
  float* err;  /*** error at outputs ***/
  float* herr; /*** derivative of Mean Square Error with respect to hidden
                    unit output ***/
  float moment;/*** momentum coefficient ***/
  float ssize; /*** training step size ****/

  /***** methods ****/
  MLP();
  MLP(const MLP& m);
  MLP operator=(const MLP& m);

  ~MLP();

  /**** randomly initialize MLP weights between min and max ***/
  /**** Seeded by Clock if parameter seed is set to zero (or not assigned). **/
  int RandomInitialize(float min, float max, int num_inputs, 
		       int num_outputs, int num_hidden_units,
                       long int seed=0);

  /**** allocate MLP structure***/
  int Allocate();

 /**** deallocate MLP structure***/
  int Deallocate();

  // modify weights to work on unnormalized data
  int postproc(float* bias, float* std);

  // modify weights to work on normalized data
  int preproc(float* bias, float* std);

  /**** function to perform one epoch of backprop with momentum on MLP ***/
  float Train(MLPData* pattern,  float moment_value, float ssize_value);
  
  /** Variable Step Search Algorithm routines **/
  float TrainVSS(MLPData* pattern, int epochno);

 protected:
  int VSSPruneIfNecessary(MLPData* pattern, int hnum, float p, float d0);
  int VSSReinitNode(int hnum, float d0);
  int VSSInit(float d0, int num_patterns);
  int UpdateHiddenTable(MLPData* pattern, int hnum);
  float GetVSSError(MLPData* pattern);
  int VSSUpdateParam(MLPData* pattern,float* w, float* dw,int hnum,float d0,float c,float c2,float h,int nmax,int epochno);


 public:
  /***** Function to perform on test epoch (no backprop) and get results ***/
  float Test(MLPData* pattern, MLPData* results);

  /**** a single forward pass through the MLP ***/
  float Forward(float* dout, float* inpts);

  /*** a single backward pass ****/
  int Backward(float* inpts);


  /**** read an MLP weights file *****/
  int Read(FILE* ifp);
  int Read(char* filename);
  int ReadHeader(FILE* ifp);
  int ReadDelta(char* filename);
  int ReadBin(FILE* ofp);
  int ReadBin(char* filename);

  /**** write an MLP weights file ****/
  int Write(FILE* ofp);
  int Write(char* filename);
  int WriteHeader(FILE* ofp);
  int WriteDelta(char* filename);
  int WriteBin(FILE* ofp);
  int WriteBin(char* filename);
};
#endif






