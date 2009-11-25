//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef MEAS_H
#define MEAS_H

static const char rcs_id_meas_h[] =
    "@(#) $Id$";

#include "mfhdf.h"

#include "Antenna.h"
#include "LonLat.h"
#include "Ephemeris.h"
#include "Attitude.h"
#include "CoastalMaps.h"
#include "List.h"
#include "math.h"
#include "Sds.h"
#include "ETime.h"

#define GET_HDF_VAR(TYPE, VAR, START, EDGES, SCALE) \
    TYPE VAR##_tmp; \
    int32 VAR##_sds_id = SDnametoid(sd_id, #VAR); \
    failed = failed || (SDreaddata(VAR##_sds_id, START, NULL, \
    		EDGES, (VOIDP)&VAR##_tmp) == FAIL); \
	double VAR = (double)VAR##_tmp * (SCALE);




//======================================================================
// CLASSES
//    Meas, MeasList, OffsetList, OffsetListList, MeasSpot,
//    MeasSpotList
//======================================================================

class MeasList;
class L1BHdf;
class L2AHdf;

//======================================================================
// CLASS
//    Meas
//
// DESCRIPTION
//    The Meas object is a general purpose object for holding and
//    manipulating sigma-0 and brightness temperature measurements.
//======================================================================

extern const char* meas_type_map[];

class Meas
{
public:

    friend int operator==(const Meas&, const Meas&);
    friend int operator>=(const Meas&, const Meas&);
    friend int operator< (const Meas&, const Meas&);

    //-------//
    // enums //
    //-------//

    enum MeasTypeE { NONE, VV_MEAS_TYPE, HH_MEAS_TYPE, VH_MEAS_TYPE,
		     HV_MEAS_TYPE, VV_HV_CORR_MEAS_TYPE, HH_VH_CORR_MEAS_TYPE,
                     C_BAND_VV_MEAS_TYPE, C_BAND_HH_MEAS_TYPE};

    //--------------//
    // construction //
    //--------------//

    Meas();
    virtual ~Meas();

    //-------------//
    // compositing //
    //-------------//

    //--------------------------------------------------//
    // If N is nonzero we                               //
    // compositing N consecutive meas objects starting  //
    // at meas_list->current                            //
    // If N is zero (Default case) whole MeasList is    //
    // composited.                                      //
    //--------------------------------------------------//

    int  Composite(MeasList* meas_list, int n = 0);

    //--------------//
    // input/output //
    //--------------//

    int  Write(FILE* fp);
    int  Read(FILE* fp);
    int  WriteAscii(FILE* fp);
    // current version //
    int UnpackL1BHdf(int32 sd_id, int *start, int *edges);
    // obsolete verison
    int  UnpackL1BHdf(L1BHdf* l1bHdf, int32 pulseIndex, int32 sliceIndex);
    int  ReadL2AHdfCell(L2AHdf* l2aHdf, int dataIndex, int arrayIndex);

    //---------//
    // freeing //
    //---------//

    void  FreeContents();

    //--------------//
    // sigma0 stuff //
    //--------------//

    float  EstimatedKp(float sigma0);

    //-----------//
    // variables //
    //-----------//

    float          value;
    float          XK;
    float          EnSlice;
    float          bandwidth;
    float          txPulseWidth;
    int            landFlag;
    Outline        outline;
    EarthPosition  centroid;

    MeasTypeE  measType;
    float      eastAzimuth;    // azimuth angle ccw from east
    float      incidenceAngle;
    int        beamIdx;
    int        startSliceIdx;
    int        numSlices;
    float      scanAngle;
    float      A, B, C;    // Kpc coefficients
    float      azimuth_width;
    float      range_width;


    //------------------------//
    // not to be written out! //
    //------------------------//

    off_t  offset;    // byte offset in file
};

//======================================================================
// CLASS
//    MeasList
//
// DESCRIPTION
//    The MeasList object is a list of Meas objects.  Used for
//    gridding.
//======================================================================

class MeasList : public List<Meas>
{
public:

    //--------------//
    // construction //
    //--------------//

    MeasList();
    ~MeasList();

    //--------------//
    // input/output //
    //--------------//

    int  Write(FILE* fp);
    int  Read(FILE* fp);
    int  WriteAscii(FILE* fp);

    int  ReadL2AHdfCell(L2AHdf* l2aHdf, int rowNo, int cellNo);
    //int  AppendL2AHdf(Meas* newMeas);
    //Meas*  lastL2AHdfMeas;

    //------//
    // info //
    //------//

    LonLat  AverageLonLat(int wtbyXK=0);

    //---------//
    // freeing //
    //---------//

    void  FreeContents();
};

//======================================================================
// CLASS
//    OffsetList
//
// DESCRIPTION
//    The OffsetList object is a list of byte offsets into a file.
//    It is used to index Meas objects in a file.
//======================================================================

class OffsetList : public List<off_t>
{
public:

    //--------------//
    // construction //
    //--------------//

	OffsetList();
	~OffsetList();

	int		MakeMeasList(FILE* fp, MeasList* meas_list);

	//---------//
	// freeing //
	//---------//

	void	FreeContents();

	//-----------//
	// variables //
	//-----------//

	long	spotId;
};

//======================================================================
// CLASS
//		OffsetListList
//
// DESCRIPTION
//		The OffsetListList object is a list of offset lists.
//======================================================================

class OffsetListList : public List<OffsetList>
{
public:

	//--------------//
	// construction //
	//--------------//

	OffsetListList();
	~OffsetListList();

	//---------//
	// freeing //
	//---------//

	void	FreeContents();
};

//======================================================================
// CLASS
//		MeasSpot
//
// DESCRIPTION
//		The MeasSpot object contains information about a spot.  It
//		contains spacecraft information as well as a list of Meas for
//		each slice.
//======================================================================

class MeasSpot : public MeasList
{
public:

	//--------------//
	// construction //
	//--------------//

	MeasSpot();
	~MeasSpot();

	//--------------//
	// input/output //
	//--------------//

    int  Write(FILE* fp);
    int  WriteAscii(FILE* fp);
    int  Read(FILE* fp);

    float NominalQuikScatBaseBandFreq(Vector3 pos);
    int  UnpackL1BHdf(int32    sds_id,
    		int32	h_id,
		    int *start,		// array of 3 indexes, indicating where to start in the hdf file
		    int *edges);		// array of the size of each dimension to read
	// obsolete version of the function, only here so that code compiles
    int  UnpackL1BHdf(L1BHdf*     l1bHdf,
                      int32       hdfIndex,    // index in the HDF
                      int32       pulseIndex); //index of the pulses(100)

 
	//-----------//
	// variables //
	//-----------//

	double		time;
	OrbitState	scOrbitState;
	Attitude	scAttitude;
};

//======================================================================
// CLASS
//		MeasSpotList
//
// DESCRIPTION
//		The MeasSpotList object contains a list of MeasSpot.  It is
//		used for storing and manipulating a set of measurements.
//======================================================================

class MeasSpotList : public List<MeasSpot>
{
public:

	//--------------//
	// construction //
	//--------------//

	MeasSpotList();
	~MeasSpotList();

	//--------------//
	// input/output //
	//--------------//

    int  Write(FILE* fp, FILE* ephemeris_fp = NULL);
    int  WriteAscii(FILE* fp);
    int  Read(FILE* fp);

    int  UnpackL1BHdf(int32    sds_id,
    		int32	h_id,
		    int *start,		// array of 3 indexes, indicating where to start in the hdf file
		    int *edges);		// array of the size of each dimension to read
	// obsolete version of the function, only here so that code compiles
    int  UnpackL1BHdf(L1BHdf*     l1bHdf, int32 hdfIndex)
    	{ fprintf(stderr, "ERROR: MeasSpotList::UnpackL1BHdf: this function is obsolete "
    		"and should never be reached. exiting\n"); abort(); }

    int  UnpackL1BHdfCoastal(L1BHdf*     l1bHdf,
			    int32       hdfIndex,
			     CoastalMaps* lmap,
			     Antenna* ant);  // index in the HDF

    //---------//
    // freeing //
    //---------//

    void  FreeContents();
};

#endif
