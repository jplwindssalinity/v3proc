//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef MEASUREMENT_H
#define MEASUREMENT_H

static const char rcs_id_measurement_h[] =
	"@(#) $Id$";

#include "List.h"
#include "Beam.h"


//======================================================================
// CLASSES
//		Measurement, MeasurementList
//======================================================================

//======================================================================
// CLASS
//		Measurement
//
// DESCRIPTION
//		The Measurement object is a general purpose object for
//		holding and manipulating sigma-0 and brightness temperature
//		data.
//======================================================================

enum BeamE { SCATTEROMETER_BEAM_A, SCATTEROMETER_BEAM_B };

class Measurement
{
public:

	//--------------//
	// construction //
	//--------------//

	Measurement();
	~Measurement();

	//--------------//
	// input/output //
	//--------------//

	int		WriteL17Format(int ofd);
	int		ReadL17Format(int ifd);

	//-----------//
	// variables //
	//-----------//

	double		time;
	PolE		pol;
	float		value;			// sigma-0 or temperature measurement
	float		incidenceAngle;
	float		scAzimuth;		// az. angle ccw from s/c velocity
	float		eastAzimuth;	// az. angle ccw from east

	float		centerLongitude;
	float		centerLatitude;

	float		estimatedKp;
};

//======================================================================
// CLASS
//		MeasurementList
//
// DESCRIPTION
//		The MeasurementList object is a list of Measurements.  This is
//		the output of the sigma-0 grouper and is fed into the wind
//		retriever.
//======================================================================

class MeasurementList : public List<Measurement>
{
public:

	//--------------//
	// construction //
	//--------------//

	MeasurementList();
	~MeasurementList();

	//--------------//
	// input/output //
	//--------------//

	int		WriteL17Format(int ofd);
	int		ReadL17Format(int ofd);
};

#endif
