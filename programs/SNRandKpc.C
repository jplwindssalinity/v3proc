//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcs_id[] =
	"@(#) $Id$";

//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Misc.h"
#include "ConfigList.h"
#include "ConfigSim.h"
#include "Tracking.h"
#include "Tracking.C"
#include "Distributions.h"
#include "AngleInterval.h"

//-----------//
// TEMPLATES //
//-----------//
template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<long>;
template class List<OffsetList>;
template class List<AngleInterval>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;





//-----------//
// CONSTANTS //
//-----------//
#define A_VAL      0.0803419
#define B_VAL      0.120513
#define C_VAL      0.0607574
#define BANDWIDTH  8314.0
#define XKVAL_OUTER      2.7167E+06
#define ENSLICE_OUTER   1234.9
#define INC_ANGLE_OUTER  54.0*dtr
#define PULSEWIDTH 0.00149709
//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//
//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<sim_config_file>", "<chi>",0 };

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(	int		argc,
	char*	argv[])
{
	//------------------------//
	// parse the command line //
	//------------------------//

	const char* command = no_path(argv[0]);
	if (argc != 3)
		usage(command, usage_array, 1);
        int clidx=1;
        char* config_file  = argv[clidx++];
	float chi = atof(argv[clidx++])*dtr;

	//---------------------//
        // read in config file //
        //---------------------//

        ConfigList config_list;
        if (! config_list.Read(config_file))
        {
                fprintf(stderr, "%s: error reading sim config file %s\n",
                        command, config_file);
                exit(1);
        }

        //-------------------------------------//
        // read the geophysical model function //
        //-------------------------------------//
 
        GMF gmf;
        if (! ConfigGMF(&gmf, &config_list))
        {
                fprintf(stderr, "%s: error configuring GMF\n", command);
                exit(1);
        }     

	//--------------//
	// configure Kp //
	//--------------//

	Kp kp;

        if (! ConfigKp(&kp, &config_list))
        {
            fprintf(stderr, "%s: error configuring Kp\n", command);
            exit(1);
        }

 	Meas::MeasTypeE mtype[4];
	mtype[0]=Meas::HH_MEAS_TYPE;
	mtype[1]=Meas::HH_VH_CORR_MEAS_TYPE;
	mtype[2]=Meas::VV_MEAS_TYPE;
	mtype[3]=Meas::VV_HV_CORR_MEAS_TYPE;
        float sigma0;

        for(int ws=1; ws<31; ws++){
          float spd=(float)ws;
          printf("%g ",spd);
	  for(int m=0;m<4;m++){
	    gmf.GetInterpolatedValue(mtype[m],INC_ANGLE_OUTER,spd,
				     chi,&sigma0);
            Meas meas;
            meas.measType=mtype[m];
            meas.incidenceAngle=INC_ANGLE_OUTER;
            meas.XK=XKVAL_OUTER;
            meas.EnSlice = ENSLICE_OUTER;
            meas.bandwidth = BANDWIDTH;
            meas.txPulseWidth = PULSEWIDTH;
            meas.A=A_VAL;
            meas.B=B_VAL;
            meas.C=C_VAL;
            float var=gmf.GetVariance(&meas,spd,chi,sigma0,&kp);
	    float kpval=sqrt(var/sigma0/sigma0);
            kpval=10.0*log10(1+kpval);
            float sigma0_over_SNR= ENSLICE_OUTER/XKVAL_OUTER;
	    float SNR= sigma0/sigma0_over_SNR;
            SNR= 10.0*log10(SNR);
            printf("%s %g %g %g ",meas_type_map[mtype[m]],sigma0,kpval,
						 SNR);
	  }
          printf("\n");
	}
        exit(0);        
}

