//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef QSCATSIMACCURATE_H
#define QSCATSIMACCURATE_H

static const char rcs_id_qscatsimaccurate_h[] =
	"@(#) $Id$";

#include "QscatSim.h"

//======================================================================
// CLASSES
//		QscatSimAccurate
//======================================================================

//======================================================================
// CLASS
//		QscatSimAccurate
//
// DESCRIPTION
//		A derived class of QscatSim which allows for more accurate but
//      slower QSCAT simulation.
//======================================================================

class QscatSimAccurate: public QscatSim
{
public:

	//-------------//
	// construction //
	//-------------//

	QscatSimAccurate();
	~QscatSimAccurate();

	//--------------------------//
	// scatterometer simulation //
	//--------------------------//

    int  SetMeasurements(Qscat* qscat, MeasSpot* meas_spot,
             WindField* windfield, GMF* gmf);

    int  ScatSim(Spacecraft* spacecraft, Qscat* qscat, WindField* windfield,
             GMF* gmf, L00Frame* l00_frame);
};

#endif
