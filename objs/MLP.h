#ifndef MLP_H
#define MLP_H

#include "MLPData.h"

/***** Multi-Layer Perceptron Structure ***/
class MLP{
  
public:

  /**** variables ***/

  int nin; /*** number of inputs ***/
  int nout; /*** number of outputs ***/
  int hn; /*** number of hidden units ***/
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
  float* ierr; /*** derivative of Mean square error with respect to inputs **/
               /*** can be used to write a backprop routine to optimize    **/
               /*** habituation parameters                                 **/
  float moment;/*** momentum coefficient ***/
  float ssize; /*** training step size ****/

  /***** methods ****/
  MLP();
  ~MLP();

  /**** randomly initialize MLP weights between min and max ***/
  /**** Seeded by Clock if parameter seed is set to zero (or not assigned). **/
  int RandomInitialize(float min, float max, int num_inputs, 
		       int num_outputs, int num_hidden_units,
                       long int seed=0);

  /**** allocate MLP structure***/
  int Allocate();


  /**** function to perform one epoch of backprop with momentum on MLP ***/
  float Train(MLPData* pattern,  float moment_value, float ssize_value);

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


  /**** write an MLP weights file ****/
  int Write(FILE* ifp);
  int Write(char* filename);
  int WriteHeader(FILE* ofp);
  int WriteDelta(char* filename);

};
#endif






