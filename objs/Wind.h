//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef WIND_H
#define WIND_H

static const char rcs_id_wind_h[] =
	"@(#) $Id$";

#include <stdio.h>
#include "Misc.h"
#include "List.h"
#include "LonLat.h"
#include "Index.h"

#include <mfhdf.h>
#include "Parameter.h"
#include "TlmHdfFile.h"

//======================================================================
// CLASSES
//		WindVector, WindVectorPlus, WVC, WindField, WindSwath
//======================================================================
//======================================================================
// CLASS
//		WindVector
//
// DESCRIPTION
//		The WindVector object represents a wind vector (speed and
//		direction).  The wind direction is counter-clockwise from East.
//======================================================================

class WindVector
{
public:

	//--------------//
	// construction //
	//--------------//

	WindVector();
	~WindVector();

	//---------//
	// setting //
	//---------//

	int		SetSpdDir(float speed, float direction);
	int		SetUV(float u, float v);
	int		GetUV(float* u, float* v);

	//-----------//
	// variables //
	//-----------//

	float		spd;
	float		dir;
};

//======================================================================
// CLASS
//		WindVectorPlus
//
// DESCRIPTION
//		The WindVectorPlus object is subclassed from a WindVector and
//		contains additional information generated by the wind
//		retrieval processing.
//======================================================================

class WindVectorPlus : public WindVector
{
public:

	//--------------//
	// construction //
	//--------------//

	WindVectorPlus();
	~WindVectorPlus();

	//--------------//
	// input/output //
	//--------------//

	int		WriteL2B(FILE* fp);
	int		WriteAscii(FILE* fp);
	int		ReadL2B(FILE* fp);

	//-----------//
	// variables //
	//-----------//

	float		obj;		// the objective function value
};

//======================================================================
// CLASS
//		WVC
//
// DESCRIPTION
//		The WVC object represents a wind vector cell.  It contains
//		a list of ambiguous solution WindVectorPlus.
//======================================================================

class WVC
{
public:

	//--------------//
	// construction //
	//--------------//

	WVC();
	~WVC();

	//--------------//
	// input/output //
	//--------------//

	int		WriteL2B(FILE* fp);
	int		ReadL2B(FILE* fp);
	int		WriteVctr(FILE* fp, const int rank);		// 0 = selected
	int		WriteAscii(FILE* fp);
	int		WriteFlower(FILE* fp);

	//--------------//
	// manipulation //
	//--------------//

	int					RemoveDuplicates();
	int					SortByObj();
	int					SortByDir();
	WindVectorPlus*		GetNearestToDirection(float dir, int max_rank = 0);

	//-------------//
	// GS routines //
	//-------------//

	int					Rank_Wind_Solutions();

	//---------//
	// freeing //
	//---------//

	void				FreeContents();

	//-----------//
	// variables //
	//-----------//

	LonLat					lonLat;
	WindVectorPlus*			selected;
	int                             selected_allocated;
	List<WindVectorPlus>	ambiguities;
};

//======================================================================
// CLASS
//		WindField
//
// DESCRIPTION
//		The WindField object hold a non-ambiguous wind field.
//======================================================================

#define VAP_LON_DIM				360
#define VAP_LAT_DIM				121
#define VAP_TYPE				"VAP"

#define ECMWF_HIRES_LON_DIM		640
#define ECMWF_HIRES_LAT_DIM		321
#define ECMWF_HIRES_TYPE		"ECMWF"

#define ECMWF_LORES_LON_DIM             360
#define ECMWF_LORES_LAT_DIM             181
#define ECMWF_LORES_TYPE                "ONE_DEG"
#define ECMWF_LORES_SCALE_FACTOR        100

#define NSCAT_LON_DIM                   720
#define NSCAT_LAT_DIM                   301
#define NSCAT_TYPE                      "NSCAT"
#define NSCAT_LAND_VALUE                -9999.0

class WindField
{
public:

	//--------------//
	// construction //
	//--------------//

	WindField();
	~WindField();

	//--------------//
	// input/output //
	//--------------//

	int		ReadVap(const char* filename);
	int		ReadEcmwfHiRes(const char* filename);
	int		WriteEcmwfHiRes(const char* filename, int extra_time_flag = 0);
        int             ReadEcmwfLoRes(const char* filename);
        int             ReadNSCAT(const char* filename);
	int		ReadType(const char* filename, const char* type);
	int		WriteVctr(const char* filename);

	int		NewRes(WindField* windfield, float lon_res, float lat_res);

	//--------//
	// access //
	//--------//

	int		NearestWindVector(LonLat lon_lat, WindVector* wv);
	int		InterpolatedWindVector(LonLat lon_lat, WindVector* wv);

	//----------//
	// tweaking //
	//----------//

    int  SetAllSpeeds(float speed);
    int  FakeEcmwfHiRes(float speed);

protected:

	//--------------//
	// construction //
	//--------------//

	int		_Allocate();
	int		_Deallocate();

	//-----------//
	// variables //
	//-----------//

	Index		_lon;
	Index		_lat;

	int			_wrap;		// flag for longitude wrapping

	WindVector***	_field;
};

//======================================================================
// ENUM  COMPONENT_TYPE
//
// DESCRIPTION
//              Used by the WindSwathClass to determine which set of
//              wind vector component to correlate in the
//              ComponentCovarianceVsCti routine.
//======================================================================

enum COMPONENT_TYPE{
  UTRUE,  // u component of true wind vector
  VTRUE,  // v component of true wind vector
  UMEAS,   // u component of measured wind vector
  VMEAS    // v component of measured wind vector
};

//======================================================================
// CLASS
//		WindSwath
//
// DESCRIPTION
//		The WindSwath object hold an ambiguous wind field gridded in
//		along track and cross track.
//======================================================================

class WindSwath
{
public:

	//--------------//
	// construction //
	//--------------//

	WindSwath();
	~WindSwath();

	int		Allocate(int cross_track_bins, int along_track_bins);

	//----------//
	// building //
	//----------//

	int		Add(int cti, int ati, WVC* wvc);

	//---------------------//
	// setting and getting //
	//---------------------//

	int		GetCrossTrackBins()		{ return(_crossTrackBins); };
	int		GetAlongTrackBins()		{ return(_alongTrackBins); };
	int		GetMaxAmbiguityCount();

	//---------//
	// freeing //
	//---------//

	int		DeleteWVCs();
	int		DeleteEntireSwath();
    int     DeleteLatitudesOutside(float low_lat, float high_lat);

	//--------------//
	// input/output //
	//--------------//

	int		WriteL2B(FILE* fp);
	int		ReadL2B(FILE* fp);
	int		ReadL2B(const char* filename);
	int		ReadHdfL2B(TlmHdfFile* tlmHdfFile);
	int		ReadHdfL2B(const char* filename);
	int		WriteVctr(const char* filename, const int rank);
	int		WriteFlower(const char* filename);
	int		WriteAscii(const char* filename);
	int		WriteAscii(FILE* fp);

	//-----------//
	// filtering //
	//-----------//

	int		InitWithRank(int rank);
	int		Nudge(WindField* nudge_field, int min_rank);
	int		SmartNudge(WindField* nudge_field);
	int		MedianFilter(int window_size, int max_passes, int weight_flag = 0);
	int		MedianFilterPass(int half_window, WindVectorPlus*** selected,
				char** change, int weight_flag = 0);

	//------------//
	// evaluation //
	//------------//

    void    operator-=(const WindSwath& w);
    int     DifferenceFromTruth(WindField* truth);
	int		CtdArray(float cross_track_res, float* ctd_array);
    int     DirArray(int number_of_bins, float* dir_array);

	float	RmsSpdErr(WindField* truth);
	float	RmsDirErr(WindField* truth);
	int     WriteDirErrMap(WindField* truth, FILE* ofp);
	float	Skill(WindField* truth);
	float	SpdBias(WindField* truth);
    int     DirectionDensity(WindField* truth,
                unsigned int* swath_density_array,
                unsigned int* field_density_array, float low_speed,
                float high_speed, int direction_count);

	int		SelectNearest(WindField* truth);
    int     GetProbabilityArray( WindField*  truth,
				float***  prob, int** num_samples, float** widths,
				int true_dir_bins, int delta_dir_bins);
	int		AvgNambigVsCti(float* avg_nambig);
	int		RmsSpdErrVsCti(WindField* truth, float* rms_spd_err_array,
				float* std_dev_array, float* std_err_array,
				float* spd_bias_array, int* count_array, float low_speed,
				float high_speed);
	int		RmsDirErrVsCti(WindField* truth, float* rms_dir_err_array,
				float* std_dev_array, float* std_err_array,
				float* dir_bias_array, int* count_array, float low_speed,
				float high_speed);
	int		SkillVsCti(WindField* truth, float* skill_array,
				int* count_array, float low_speed, float high_speed);
	int		WithinVsCti(WindField* truth, float* within_array,
				int* count_array, float low_speed, float high_speed,
				float within_angle);

    int     DirectionDensityVsCti(WindField* truth,
                unsigned int** swath_density_array,
                unsigned int** field_density_array, float low_speed,
                float high_speed, int direction_count);

    int     VectorCorrelationVsCti(WindField* truth, float* vc_array,
                int* count_array, float low_speed, float high_speed);
    int     ComponentCovarianceVsCti(WindField* truth, float* cc_array,
                int* count_array, float low_speed, float high_speed,
                COMPONENT_TYPE component1, COMPONENT_TYPE component2);

	//-----------//
	// variables //
	//-----------//

	WVC***		swath;

protected:

	//--------------//
	// construction //
	//--------------//

	int		_Allocate();
	int		_Deallocate();

	//--------------//
	// for HDF      //
	//--------------//
    int     _OpenOneHdfDataSet(TlmHdfFile*, SourceIdE, ParamIdE);
    int     _OpenHdfDataSets(TlmHdfFile*);
    void    _CloseHdfDataSets(void);

	//-----------//
	// variables //
	//-----------//

	int		_crossTrackBins;
	int		_alongTrackBins;
	int		_validCells;

	//--------------//
	// for HDF      //
	//--------------//
    int32   _lonSdsId;
    int32   _latSdsId;
    int32   _speedSdsId;
    int32   _dirSdsId;
    int32   _mleSdsId;
    int32   _selectSdsId;
};

#endif
