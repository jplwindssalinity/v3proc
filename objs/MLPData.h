#ifndef API_DATASTRUCTS
#define API_DATASTRUCTS
#define MAXIMUM_LINE_LENGTH 200000

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

};

/********** Associated Functions ***************/

/*** skip through comment lines and blank lines and return 
     pointer to first token encountered ***/
char* skip_comments(FILE* ifp, char* line_from_file);

int get_floats_array(float* ptr, int size, FILE* ifp);

/**** takes a pattern length string such as "[9]" *****/
/*** and returns the integer i.e. 9               *****/
int get_patlen(char* k);

#endif




