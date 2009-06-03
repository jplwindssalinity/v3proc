#ifndef API_DATASTRUCTS
#define API_DATASTRUCTS
#define MAXIMUM_LINE_LENGTH 200000
class MLP;
/*** STATIC Type Data Structure ***/
class MLPData{
public:
 
  /************ variables ****/
  int num_inpts; 
  int num_outpts; 
  int num_samps;
  float *start_inpt;
  float *start_outpt;
  float **inpt;
  float **outpt;

  /***####### Prototypes of MLPData Methods#####*****/

  /*** Constructor and Destructor***/
  ~MLPData();
  MLPData();

  int Normalize(float* bias, float* std);
  int Unnormalize(float* bias, float* std);

  /*** Read from File ***/
  int Read(char* filename);
  int ReadDelta(char* filename);
  int Read(FILE* ifp);

  /*** Write to File ***/
  int Write(FILE* ofp);
  int Write(char* filename);
  int WriteDelta(char* filename);

  /*** Write Header to File ***/
  int WriteHeader(FILE* ofp);


  
  /*** Allocate Structure: This function is useful for allocating 
    an data structure which is not read into a file. It takes the
    data set constants, allocates the inpt and outpt arrays, and
    fills them with zeroes****/
  int Allocate();
  int Deallocate();

  /*** Randomly shuffle samples in data set ***/
  int Shuffle(float** old_inpt_ptrs=NULL, float** old_outpt_ptrs=NULL);

  /*** Copy a data set ***/
  int Copy(MLPData* newptr);

  /*** Append one data set (newdata) to the end of another (store) **/
  int Append(MLPData* newdata);

  /*** randomly initialize an MLP ****/
  int  RandomInitialize(float range_min, float range_max, int num_inputs, 
	      int num_outputs, int num_hidden_units, long int seed=0);

  int Normalize(float* bias, float* std, int nvalidsamps);
};

class MLPDataArray{
 public:
  // Construction method for creating dataset and MLP array
  MLPDataArray(char* datfile, char* netfile, int dim1, float min1, float max1, int dim2, float min2, float max2, int MLPindim, int MLPoutdim, int num_samps, char* namestr, int hn=50);

  // Construction method for reading datasets one at a time from dataset file and reading MLP array
  // for MLP file
  MLPDataArray(char* datfile, char* netfile,char* netfilemode);

  // Constructor for making MLP array only for use in wind retrieval (reads MLPs from file)
  MLPDataArray(char* netfile);

  ~MLPDataArray();
  // Method to add one sample at a time to current data set
  // if DataSet position in array changes then trainMLPandWrite is called.
  // and DataSet is reinitialized for new position in array
  // Method is fragile samples must be added in dim1 dim2 numsamp order where numsamp is fastest
  // increasing index
  int addSampleInOrderAndWrite(float val1, float val2, float* inputs, float* outputs);
  int addSample(float val1, float val2, float* inputs, float* outputs);
  int Train();
  int reTrain();
  // Method for reading next DataSet from DataSetfile (goes in order)
  MLPData& readNextDataSet();
  
  // Get pointer to MLP
  MLP* getMLP(float val1, float val2);

  // Closes files and writes last DataSet
  int Flush();

  // Set in Direction PDF mode
  int SetDirPdf();
 protected:
  int readMLPArray();
  int readHeader();
  int readGenericHeader(FILE* ifp);
  int writeHeader();
  int allocateMLPArray();
  int readMLPHeader();
  int writeMLPHeader();
  int trainMLPAndWrite();
  int _trainMLP(int nvalidsamps);
  int balanceWeights(MLP* m);
  float trainDirPdf(MLP* m, MLPData* d, float* nrms);
  float testDirPdf(MLP* m, MLPData* d, float* nrms);
  float computeDirPdfErr(MLP* m, float* dout, float* nrms);
  int initializeDataSet();
  FILE* datfp;
  FILE* netfp;
  MLPData latestDataSet;
  MLP*** MLParray;

 public:
  int size1;
  int size2;
  int numsamps;
 protected:
  int idx1;
  int idx2;
  int sampno;
  float min1_;
  float max1_;
  float min2_;
  float max2_;
 public:
  int nMLPin;
  int nMLPout;
  int nMLPhn;
 protected:
  float* degdist;
  bool dirpdf;

 public:
  int nepochs;  
  int max_bad_epochs;
  char* name;
  float moment;
  float ssize;
  bool vss;
};

/********** Associated Functions ***************/

/*** skip through comment lines and blank lines and return 
     pointer to first token encountered ***/
char* skip_comments(FILE* ifp, char* line_from_file);

int get_floats_array(float* ptr, int size, FILE* ifp);

/**** takes a pattern length string such as "[9]" *****/
/*** and returns the integer i.e. 9               *****/
int get_patlen(char* k);

/** read single line inputs **/
int read_mlpint(FILE *ifp);
float read_mlpfloat(FILE *ifp);

#endif




