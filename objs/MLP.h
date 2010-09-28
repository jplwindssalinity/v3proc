#ifndef MLP_H
#define MLP_H

#include "MLPData.h"

/**** struct to store/ define the types of inputs ****/
#define IO_TYPE_STR_MAX_LENGTH          64
#define NUM_MLP_IO_TYPES         59
struct MLP_IOType {
	char str[IO_TYPE_STR_MAX_LENGTH];
	int id;
};

// longest the description of the training sets is allowed to be,
// in number of characters
#define TRAIN_SETS_DESC_MAX_LENGTH		1000

// defines for the argument taken by some functions; specifies whether to look in/ set
// the input or output types
#define MLP_IO_IN_TYPE      0
#define MLP_IO_OUT_TYPE     1



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
  float* inpt;  /*** array for holding inputs ***/
  float moment;/*** momentum coefficient ***/
  float ssize; /*** training step size ****/
  
  MLP_IOType *in_types; /*** array correlating input number to what
                               should be used for that input ***/
  MLP_IOType *out_types; /*** array correlating output number to what
                               is represented by that output ***/
  char train_set_str[TRAIN_SETS_DESC_MAX_LENGTH+1];  /*** string representing what the data was trained on;
                               for reference and writing to disk only- won't be
                               used by this class ***/

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
  
  /** function to locate the given type_str in the IO type defs, and set either in_types
    or out_types accordingly. must take in a pointer to the type defs and 
    the types buffer (either a pointer to in_types or out_types) **/
  int setIOTypeByString(char *type_str, int input_idx, int in_out);

  /** Set the inputs to be the specified string **/
  int setInputTypeByString(char *type_str, int input_idx);
  int setInputTypesByString(char type_strs[][IO_TYPE_STR_MAX_LENGTH]);
  /** Set the outputs to be the specified string **/
  int setOutputTypeByString(char *type_str, int input_idx);
  int setOutputTypesByString(char type_strs[][IO_TYPE_STR_MAX_LENGTH]);
  
  /** given an MLP_IOType string, find its index in the input or output **/
  int findIOTypeInd(char *mlp_io_type_str, int in_out);

  /** set a string describing what data the neural network was trained on **/
  int setTrainSetString(char *train_set_str_);

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
  int Forward(float* inpts);
  bool AssignInputs(float* inpts, bool* mask);
  int Forward();
  float ForwardMSE(float* inpts, float* doutx);

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






