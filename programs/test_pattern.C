//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		test_pattern
//
// SYNOPSIS
//		test_pattern cfg_file
//
// DESCRIPTION
//		Reads in the SeaWinds antenna patterns and generates
//		simulation patterns.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operand is supported:
//
// EXAMPLES
//		An example of a command line is:
//			% test_pattern sws1a.cfg
//
// ENVIRONMENT
//		Not environment dependent.
//
// EXIT STATUS
//		The following exit values are returned:
//		0	Program executed successfully
//		>0	Program had an error
//
// NOTES
//		None.
//
// AUTHOR
//		James N. Huddleston
//		hudd@acid.jpl.nasa.gov
//----------------------------------------------------------------------

//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcs_id[] =
	"@(#) $Id$";

//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include <stdlib.h>
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Misc.h"
#include "Constants.h"
#include "Attitude.h"
#include "CoordinateSwitch.h"
#include "Matrix3.h"
#include "Beam.h"
#include "Array.h"
#include "ConfigList.h"
#include "ConfigSim.h"
#include "QscatConfig.h"
#include "Antenna.h"
#include "Tracking.h"
#include "Tracking.C"
#include "Qscat.h"

//-----------//
// TEMPLATES //
//-----------//

// Class declarations needed for templates
// eliminates need to include the entire header file
class AngleInterval;

template class List<AngleInterval>;
template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<WindVectorPlus>;
template class List<long>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;

//-----------//
// CONSTANTS //
//-----------//

#define POINTS	260000
#define HPOL	0
#define VPOL	1

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

int read_pattern(const char* filename, float* el, float* az, float* gain,
		int pol, int* number_read);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<config file>", 0};

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
	int		argc,
	char*	argv[])
{
	//------------------------//
	// parse the command line //
	//------------------------//

	const char* command = no_path(argv[0]);
	if (argc != 2)
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* config_file = argv[clidx++];

	//--------------------------------//
	// read in simulation config file //
	//--------------------------------//

	ConfigList config_list;
	if (! config_list.Read(config_file))
	{
		fprintf(stderr, "%s: error reading sim config file %s\n",
			command, config_file);
		exit(1);
	}

	//-----------------------------------------------//
	// create QSCAT                                  //
	//-----------------------------------------------//

	Qscat qscat;
	if (! ConfigQscat(&qscat, &config_list))
	{
		fprintf(stderr, "%s: error configuring QSCAT\n", command);
		exit(1);
	}

	float G0H,G0V;
	G0H=10*log10(qscat.sas.antenna.beam[0].peakGain);
	G0V=10*log10(qscat.sas.antenna.beam[1].peakGain);
	printf("Max Gain values (dB H,V) = %g %g\n",G0H,G0V);
	double look,azimuth;
	float gain;

	Beam beam = qscat.sas.antenna.beam[1];
	beam.GetElectricalBoresight(&look,&azimuth);
	printf("Electrical Boresight V: (look,azi) %g %g\n",look*rtd,azimuth*rtd);
	beam.GetPowerGain(look,azimuth,&gain);
	printf("Electrical Boresight V: (gain dB) %g\n",10.0*log(gain)/log(10.0));
	beam.GetPowerGain(look,-0.00122173,&gain);
	printf("gain1 = %g\n",gain);
	beam.GetPowerGain(look,-0.00120428,&gain);
	printf("gain2 = %g\n",gain);
	// Write out cuts for plotting.
//	for (int i=-1000; i <= 1000; i++)
//	{
//		double azi = i/1000.0*dtr;
//		beam.GetPowerGain(look,azi,&gain);
//		printf("%g %g\n",azi,10.0*log(gain)/log(10.0));
//	}

	beam = qscat.sas.antenna.beam[0];
	beam.GetElectricalBoresight(&look,&azimuth);
	printf("Electrical Boresight H: (look,azi) %g %g\n",look*rtd,azimuth*rtd);
	beam.GetPowerGain(look,azimuth,&gain);
	printf("Electrical Boresight H: (gain dB) %g\n",10.0*log(gain)/log(10.0));
	// Write out cuts for plotting.
	for (int i=-1000; i <= 1000; i++)
	{
		double azi = i/1000.0*dtr;

	    Vector3 vector;
   		vector.SphericalSet(1.0, look, azi);
    	vector = beam._antennaFrameToBeamFrame.Forward(vector);
    	double r, theta, phi;
    	vector.SphericalGet(&r, &theta, &phi);
    	double Em = pi / 2.0 - theta;
    	double Am = phi;
	
		beam.GetPowerGain(look,azi,&gain);
		printf("%g %g %g %g\n",azi,10.0*log(gain)/log(10.0),Em,Am);
	}

	return (0);

	

}

//--------------//
// read_pattern //
//--------------//

int
read_pattern(
	const char*		filename,
	float*			el,
	float*			az,
	float*			gain,
	int				pol,
	int*			number_read)
{
	printf("Reading %s...\n", filename);
	FILE* fp = fopen(filename, "r");
	if (fp == NULL)
		return(0);

	char line[1024];

	// skip first two lines
	fgets(line, 1024, fp);
	fgets(line, 1024, fp);

	float elev, azim, efield[2];

	int idx = 0;
	while (fgets(line, 1024, fp) == line)
	{
		if (sscanf(line, " %g %g %g %g", &elev, &azim, &(efield[0]),
			&(efield[1])) != 4)
		{
			break;
		}
		el[idx] = elev * dtr;
		az[idx] = azim * dtr;
		gain[idx] = efield[pol];
		idx++;
	}

	*number_read = idx;

	int retval;
	if (feof(fp))
		retval = 1;
	else
		retval = 0;

	fclose(fp);

	printf("  Done.\n");

	return(retval);
}
