//======================================================================
// CLASSES
//		InstrumentSimAccurate
//======================================================================

//======================================================================
// CLASS
//		InstrumentSimAccurate
//
// DESCRIPTION
//		A derived class of InstrumentSim which allows for more
//              accurate but slower Instrument Simulation.
//======================================================================


#include"InstrumentSim.h"
class InstrumentSimAccurate: public InstrumentSim
{
public:

	//-------------//
	// contruction //
	//-------------//

	InstrumentSimAccurate();
	~InstrumentSimAccurate();

	//--------------------------//
	// scatterometer simulation //
	//--------------------------//

	int		SetMeasurements(Instrument* instrument, 
				MeasSpot* meas_spot, WindField* windfield,
				GMF* gmf);

	int		ScatSim(double time, Spacecraft* spacecraft,
				Instrument* instrument, WindField* windfield, GMF* gmf,
				L00Frame* l00_frame);

};
