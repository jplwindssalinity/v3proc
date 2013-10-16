//==============================================================//
// Copyright (C) 2013, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    generate_rgc
//
// SYNOPSIS
//    generate_rgc_from_ephem_quat_files [ -cf -s rev ] <config_file> <RGC_base>
//
// DESCRIPTION
//    Generates a set of Receiver Gate Constants for each beam based
//    upon the parameters in the simulation configuration file.
//
// OPTIONS
//      [ -c ]  Clean the constants.
//      [ -f ]  Make the range fixed over azimuth.
//      [ -s i ] start with rev i in ephem / quat files
//
// OPERANDS
//    The following operands are supported:
//      <config_file>  The config_file needed listing
//                     all input parameters.
//
//      <RGC_base>     The RGC output base.
//
// EXAMPLES
//    An example of a command line is:
//      % generate_constants sws1b.cfg rgc.dat
//
// ENVIRONMENT
//    Not environment dependent.
//
// EXIT STATUS
//    The following exit values are returned:
//       0   Program executed successfully
//      >0  Program had an error
//
// NOTES
//    None.
//
// AUTHOR
//    James N. Huddleston / Alex Fore
//    hudd@casket.jpl.nasa.gov
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
#include <unistd.h>
#include <vector>
#include "Misc.h"
#include "Ephemeris.h"
#include "Quat.h"
#include "ConfigList.h"
#include "QscatConfigDefs.h"
#include "ConfigSimDefs.h"
#include "Spacecraft.h"
#include "ConfigSim.h"
#include "QscatConfig.h"
#include "InstrumentGeom.h"
#include "Qscat.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"
#include "Constants.h"
#include "Interpolate.h"

//-----------//
// TEMPLATES //
//-----------//

// Class declarations needed for templates
// eliminates need to include the entire header file
class AngleInterval;

template class List<AngleInterval>;
template class BufferedList<OrbitState>;
template class BufferedList<QuatRec>;
template class List<OrbitState>;
template class List<QuatRec>;
template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<MeasSpot>;
template class List<WindVectorPlus>;
template class List<off_t>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class std::list<string>;
template class std::map<string,string,Options::ltstr>;


//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  "cfs:o:l:"

#define RANGE_ORBIT_STEPS    256
#define RANGE_AZIMUTH_STEPS  90    // used for fitting

#define EQX_TIME_TOLERANCE   0.1

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

const char* usage_array[] = { "[ -cf -s start_rev]", "<config_file>", "<RGC_base>",
    "<ephem_file>", "<quat_file>", 0 };

int opt_fixed = 0;     // by default, delay can change as a function of azimuth
int opt_clean = 0;

//--------------//
// MAIN PROGRAM //
//--------------//

void cost_function( double* rtt,
                    double* cp,
                    double  amp,
                    double  off,
                    double* J ) {
  double this_cost = 0;
  for( int ii=0;ii<RANGE_AZIMUTH_STEPS;++ii) {
    double residual = amp*cp[ii]+off-rtt[ii];
    double weight   = 1.0;
    this_cost      += weight * residual * residual;
  }
  *J = this_cost;
}

int line_search(  double* rtt,
                  double  rgc_limit,
                  double* cp,
                  double  x_init,
                  double  delta,
                  int     interp,
                  double* x_final,
                  double* J_final ) {
  
  int min_found = 0;
  int do_interp = 0;
  int do_center = 1;
  int do_plus   = 1;
  int do_minus  = 1;
  
  double x_center, x_plus, x_minus;
  double J_center, J_plus, J_minus;
  
  x_center = x_init;
  
  double amp, off;
  
  int iterations = 0;
  while ( !min_found && iterations < 10000 ) {
    if( do_center ) {
      amp = rgc_limit * x_center;
      off = rgc_limit * (1-fabs(x_center));
      cost_function( rtt, cp, amp, off, &J_center );
    }
    if( do_minus ) {
      x_minus = x_center - delta;
      if( x_minus < 0 ) x_minus = 0;
      amp = rgc_limit * x_minus;
      off = rgc_limit * (1-fabs(x_minus));
      cost_function( rtt, cp, amp, off, &J_minus );    
    }
    if( do_plus ) {
      x_plus = x_center + delta;
      if( x_plus > 1 ) x_plus = 1;
      amp = rgc_limit * x_plus;
      off = rgc_limit * (1-fabs(x_plus));
      cost_function( rtt, cp, amp, off, &J_plus );    
    }
    
    min_found = 1;
    do_interp = 1;
    
    if( J_plus < J_center && J_plus < J_minus ) {
      min_found = 0;
      J_minus  = J_center;
      x_minus  = x_center;
      J_center = J_plus;
      x_center = x_plus;
      
      if( x_plus >= 1 ) {
        x_center  = 1;
        min_found = 1;
        do_interp = 0;
      }
      do_minus  = 0;
      do_center = 0;
      do_plus   = 1;
    }

    if( J_minus < J_center && J_minus < J_plus ) {
      min_found = 0;
      J_plus   = J_center;
      x_plus   = x_center;
      J_center = J_minus;
      x_center = x_minus;
      
      if( x_minus <= 0 ) {
        x_center  = 0;
        min_found = 1;
        do_interp = 0;
      }
      do_minus  = 1;
      do_center = 0;
      do_plus   = 0;
    }
    ++iterations;
  }

  // Taylor series fit about extrema
  double dJ_dx   = (J_plus-J_minus)/(2*delta);
  double d2J_dx2 = (J_plus+J_minus-2*J_center)/(delta*delta);
  double x_off   =  -dJ_dx / d2J_dx2;
  
  if( interp && do_interp && d2J_dx2 != 0.0 && 
      x_center + x_off >= 0 && x_center + x_off <= 1 ) {
    // Solve for extrema of 2nd order Taylor series
    *x_final = x_center + x_off; 
    *J_final = J_center + dJ_dx * x_off + d2J_dx2 * x_off * x_off / 2.0;
  } else {
    *x_final = x_center;
    *J_final = J_center;
  }
  return(iterations);
}

int fit_rtt( double* rtt,     // Actual round-trip-time [ms]
             double  rgc_limit,
             double  amp0,    // from uncontrained fit
             double  pha0,    // from uncontrained fit
             double  off0,    // from uncontrained fit
             double* amp1,    // contrained fit amp
             double* off1 ) { // constrained fit constant
             
  double cp[RANGE_AZIMUTH_STEPS];
  
  // Check if inside valid solution region
  if( off0+amp0 <= rgc_limit ) {
    *amp1 = amp0;
    *off1 = off0;
    return(1);
  }
  
  // Outside valid solution region, find best solution on boundary
  for(int ii=0;ii<RANGE_AZIMUTH_STEPS;++ii) {
    double this_azi = two_pi * (double)ii / (double)RANGE_AZIMUTH_STEPS;
    cp[ii] = cos( this_azi + pha0 );
  }
  
  double x_init = 0.1;
  double x_coarse, x_fine;
  double J_coarse, J_fine;
  
  double delta_coarse = 1E-4;
  double delta_fine   = 1E-7;
  
  int iter_coarse = line_search( rtt, rgc_limit, &cp[0], x_init, 
                                 delta_coarse, (int)0, &x_coarse, &J_coarse );


  int iter_fine = line_search( rtt, rgc_limit, &cp[0], x_coarse, 
                               delta_fine,   (int)0, &x_fine,   &J_fine   );

  double amp_fine   = rgc_limit * x_fine;
  double off_fine   = rgc_limit * (1-fabs(x_fine));
  
  *amp1 = amp_fine;
  *off1 = off_fine;
  
  return(1);
}


const char usage_string[] = "-c config_file -e ephem.dat -q quats.dat -outbase outbase [-out_table out_rtt_table]";


int
main(
    int    argc,
    char*  argv[])
{
    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);
    char*       out_rtt_table = NULL;
    char*       config_file   = NULL;
    char*       ephem_file    = NULL;
    char*       quat_file     = NULL;
    char*       rgc_base      = NULL;
    
    double rgc_limit = -1;
    int    start_rev = 0;
    
    int optind = 1;
    while( (optind < argc) && (argv[optind][0]=='-') ) {
      std::string sw = argv[optind];
      if( sw == "-out_table" ) {
        out_rtt_table = argv[++optind];
      } else if( sw == "-start_rev" ) {
        start_rev = atoi(argv[++optind]);
      } else if( sw == "-out_base" ) {
        rgc_base = argv[++optind];
      } else if( sw == "-l" ) {
        rgc_limit = atof(argv[++optind]);
      } else if( sw == "-c" ) {
        config_file = argv[++optind];
      } else if( sw == "-e" ) {
        ephem_file = argv[++optind];
      } else if( sw == "-q" ) {
        quat_file = argv[++optind];
      } else {
        fprintf(stderr,"%s: %s\n",command,&usage_string[0]);
        exit(1);
      }
      ++optind;
    }

    if( !config_file || !rgc_base || !ephem_file || !quat_file ) {
      fprintf(stderr,"%s: %s\n",command,&usage_string[0]);
      exit(1);    
    }

    //--------------------------------//
    // read in simulation config file //
    //--------------------------------//

    ConfigList config_list;
    if (! config_list.Read(config_file))
    {
        fprintf(stderr, "%s: error reading configuration file %s\n",
            command, config_file);
        exit(1);
    }

    //----------------------------------//
    // force RGC and DTC to not be read //
    //----------------------------------//

    config_list.StompOrAppend(USE_RGC_KEYWORD, "0");
    config_list.StompOrAppend(USE_DTC_KEYWORD, "0");
    config_list.StompOrAppend(ORBIT_TICKS_PER_ORBIT_KEYWORD, "0");
    config_list.StompOrAppend(USE_KFACTOR_KEYWORD, "0");

    //----------------------------------------------//
    // create a spacecraft and spacecraft simulator //
    //----------------------------------------------//

    Spacecraft spacecraft;
    if (! ConfigSpacecraft(&spacecraft, &config_list))
    {
        fprintf(stderr, "%s: error configuring spacecraft\n", command);
        exit(1);
    }

    SpacecraftSim spacecraft_sim;
    if (! ConfigSpacecraftSim(&spacecraft_sim, &config_list))
    {
        fprintf(stderr, "%s: error configuring spacecraft simulator\n",
            command);
        exit(1);
    }
    spacecraft_sim.LocationToOrbit(0.0, 0.0, 1);

    //----------------------------------------//
    // select geodetic or geocentric attitude //
    //----------------------------------------//

    if (! ConfigAttitude(&config_list))
    {
        fprintf(stderr, "%s: using default attitude reference\n", command);
    }

    //--------------------------------------//
    // create a QSCAT and a QSCAT simulator //
    //--------------------------------------//

    Qscat qscat;
    if (! ConfigQscat(&qscat, &config_list))
    {
        fprintf(stderr, "%s: error configuring QSCAT\n", command);
        exit(1);
    }

    QscatSim qscat_sim;
    if (! ConfigQscatSim(&qscat_sim, &config_list))
    {
        fprintf(stderr, "%s: error configuring QSCAT simulator\n", command);
        exit(1);
    }

    //----------------//
    // allocate terms //
    //----------------//

    // terms are [0] = amplitude, [1] = phase, [2] = bias
    double** terms;
    terms = (double **)make_array(sizeof(double), 2, RANGE_ORBIT_STEPS, 3);
    
    // Open ephem and quat files
    Ephemeris ephem(ephem_file,10000000);
    QuatFile  quats(quat_file,10000000);
    
    // Determine period from ephem file and start time at Asc node
    std::vector< double > asc_node_times;    
    OrbitState this_os;
    OrbitState prev_os;
    
    ephem.GetNextOrbitState( &prev_os );
    while(ephem.GetNextOrbitState( &this_os )) {
      
      double this_z  = this_os.rsat.GetZ();
      double prev_z  = prev_os.rsat.GetZ();
      double this_vz = this_os.vsat.GetZ();
      
      if( this_z>=0 && prev_z < 0 && this_vz >= 0 ) {
        // linear interpolation to z==0 time;
        double asc_node_time = prev_os.time 
                             + ( this_os.time - prev_os.time ) 
                             * ( 0.0 - prev_z ) / ( this_z - prev_z );
        
        asc_node_times.push_back( asc_node_time );
      }
      prev_os = this_os;
    }
    
    double orbit_period = 0;
    for( size_t ii=0;ii<asc_node_times.size()-1; ++ii) 
      orbit_period += asc_node_times[ii+1]-asc_node_times[ii];
    
    orbit_period /= double(asc_node_times.size()-1);
    
    printf("orbit_period: %f, revs used: %zd\n",orbit_period,asc_node_times.size()-1);
    
    //------------//
    // initialize //
    //------------//

    //double orbit_period = spacecraft_sim.GetPeriod();
    double orbit_step_size = orbit_period / (double)RANGE_ORBIT_STEPS;
    double azimuth_step_size = two_pi / (double)RANGE_AZIMUTH_STEPS;
    unsigned int orbit_ticks_per_orbit =
        (unsigned int)(orbit_period * ORBIT_TICKS_PER_SECOND + 0.5);
    printf("%s %d\n", ORBIT_TICKS_PER_ORBIT_KEYWORD, orbit_ticks_per_orbit);
    qscat.cds.CmdOrbitTicksPerOrbit(orbit_ticks_per_orbit);

    //----------------------------//
    // select encoder information //
    //----------------------------//

    unsigned int encoder_offset_dn = 0;
    switch (qscat.sas.encoderElectronics)
    {
        case ENCODER_A:
            encoder_offset_dn = qscat.cds.encoderAOffset;
            break;
        case ENCODER_B:
            encoder_offset_dn = qscat.cds.encoderBOffset;
            break;
        default:
            fprintf(stderr, "%s: unknown encoder electronics\n", command);
            exit(1);
    }
    double encoder_offset = (double)encoder_offset_dn * two_pi /
        (double)ENCODER_N;

    //-------------------------------------------//
    // determine spin rate in radians per second //
    //-------------------------------------------//

    double assumed_spin_rate = qscat.cds.GetAssumedSpinRate();
    assumed_spin_rate *= rpm_to_radps;

    //--------------------//
    // step through beams //
    //--------------------//
    FILE* ofp_rtt_table = NULL;
    if( out_rtt_table ) {
      ofp_rtt_table = fopen(out_rtt_table,"w");
      int tmp = RANGE_ORBIT_STEPS;
      fwrite(&tmp,sizeof(int),1,ofp_rtt_table);
      tmp = RANGE_AZIMUTH_STEPS;
      fwrite(&tmp,sizeof(int),1,ofp_rtt_table);
    }

    
    for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++)
    {
        //---------------------------//
        // get the beam offset angle //
        //---------------------------//

        unsigned int cds_beam_offset_dn = 0;
        switch (beam_idx)
        {
            case 0:
                cds_beam_offset_dn = BEAM_A_OFFSET;
                break;
            case 1:
                cds_beam_offset_dn = BEAM_B_OFFSET;
                break;
            default:
                fprintf(stderr, "%s: too many beams\n", command);
                exit(1);
                break;
        }
        double cds_beam_offset = (double)cds_beam_offset_dn * two_pi /
            (double)ENCODER_N;

        //------------------------//
        // allocate range tracker //
        //------------------------//

        qscat.cds.currentBeamIdx = beam_idx;
        Beam* beam = qscat.GetCurrentBeam();
        CdsBeamInfo* cds_beam_info = qscat.GetCurrentCdsBeamInfo();
        if (! cds_beam_info->rangeTracker.Allocate(RANGE_ORBIT_STEPS))
        {
            fprintf(stderr, "%s: error allocating range tracker\n", command);
            exit(1);
        }

        //------------------------------//
        // start at an equator crossing //
        //------------------------------//

//        double start_time =
//            spacecraft_sim.FindNextArgOfLatTime(spacecraft_sim.GetEpoch(),
//            EQX_ARG_OF_LAT, EQX_TIME_TOLERANCE);

        // Check that there is enough ephem data for a whole rev.
        if( start_rev >= (int)asc_node_times.size()-1 ) {
          fprintf(stderr,"need more ephem data or start_rev too big\n");
          exit(1);
        }
        
        double start_time = asc_node_times[start_rev];
        qscat.cds.SetEqxTime(start_time);

        //------------//
        // initialize //
        //------------//

        if (! qscat_sim.Initialize(&qscat))
        {
            fprintf(stderr, "%s: error initializing QSCAT simulator\n",
                command);
            exit(1);
        }

        if (! spacecraft_sim.Initialize(start_time))
        {
            fprintf(stderr, "%s: error initializing spacecraft simulator\n",
                command);
            exit(1);
        }

        //--------------------//
        // loop through orbit //
        //--------------------//
        
        for (int orbit_step = 0; orbit_step < RANGE_ORBIT_STEPS; orbit_step++)
        {
            //--------------------//
            // calculate the time //
            //--------------------//

            // addition of 0.5 centers s/c on orbit_step
            double time = start_time +
                orbit_step_size * ((double)orbit_step + 0.5);

            //-----------------------//
            // locate the spacecraft //
            //-----------------------//
            spacecraft_sim.UpdateOrbit(time, &spacecraft);
            
            //---Modified to use input ephem and quaterion file------------
            // Overwrite spacecraft attitude and orbitstate with that
            // interpolated from ephem and quaterion files
            Quat this_quat;
            
            // Interpolate quaternion to this time
            quats.GetQuat( time, &this_quat );
            
            // Convert to attitude angles and overwrite those in spacecraft object
            this_quat.GetAttitude( &(spacecraft.attitude) );
            
            //spacecraft.attitude.SetPitch(spacecraft.attitude.GetPitch()-3*dtr);
            
            // Interpolate ephem and overwrite that in spacecraft object
            ephem.GetOrbitState( time, EPHEMERIS_INTERP_ORDER, &(spacecraft.orbitState) );
            //--------------------------------------------------------------
            
            
            //----------------------//
            // step through azimuth //
            //----------------------//
            
            double rtt[RANGE_AZIMUTH_STEPS];
            
            for (int azimuth_step = 0; azimuth_step < RANGE_AZIMUTH_STEPS;
                azimuth_step++)
            {
                //--------------------------------//
                // calculate azimuth angle to use //
                //--------------------------------//

                // The table needs to be built for the CDS algorithm,
                // but we need to determine the actual antenna azimuth
                // angle.  The following code starts from the CDS azimuth,
                // backtracks to the original sampled encoder and then
                // calculates the actual antenna azimuth at the ground
                // impact time.  This method of doing the calculation will
                // allow for things like changes in the antenna spin rate
                // which will affect the actual antenna azimuth but not
                // the CDS estimation (which uses hardcoded spin rates).

                // start with the azimuth angle to be used by the CDS
                double azimuth = azimuth_step_size * (double)azimuth_step;

                // set the antenna azimuth for calculating the centering offset
                qscat.sas.antenna.SetEncoderAzimuthAngle(azimuth);

                // subtract the beam offset
                azimuth -= cds_beam_offset;

                // subtract an estimate of the centering offset
                // uses an estimate of the round trip time as
                // the previous pulses round trip time
                OrbitState* orbit_state = &(spacecraft.orbitState);
                Attitude* attitude = &(spacecraft.attitude);
                Antenna* antenna = &(qscat.sas.antenna);
                CoordinateSwitch antenna_frame_to_gc =
                    AntennaFrameToGC(orbit_state, attitude, antenna, azimuth);
                double look, az;
                GetPeakSpatialResponse2(&antenna_frame_to_gc, &spacecraft,
                    beam, antenna->spinRate, &look, &az);
                Vector3 vector;
                vector.SphericalSet(1.0, look, az);
                QscatTargetInfo qti;
                if (! qscat.TargetInfo(&antenna_frame_to_gc, &spacecraft,
                    vector, &qti))
                {
                    fprintf(stderr, "%s: error finding round trip time\n",
                        command);
                    exit(1);
                }

                // then apply to the azimuth angle
                double delay = (qti.roundTripTime + qscat.ses.txPulseWidth) /
                    2.0;
                azimuth -= (delay * assumed_spin_rate);

                // subtract the encoder offset
                azimuth -= encoder_offset;

                // subtract the internal (sampling) delay angle
                double cds_pri = (double)qscat.cds.priDn / 10.0;
                azimuth -= (cds_pri * assumed_spin_rate);

                // add the actual sampling delay angle
                azimuth += (qscat.ses.pri * qscat.sas.antenna.spinRate);

                // and get to the center of the transmit pulse so that
                // the two-way gain product is formed correctly

                azimuth += (qscat.ses.txPulseWidth *
                    qscat.sas.antenna.spinRate / 2.0);

                //-------------------------//
                // set the antenna azimuth //
                //-------------------------//

                qscat.sas.antenna.SetEncoderAzimuthAngle(azimuth);
                qscat.SetOtherAzimuths(&spacecraft);

                //-------------------------------------------//
                // calculate the ideal round trip time in ms //
                //-------------------------------------------//
                // for an explanation of why T_GRID and T_RC are
                // here, check with Rod's memo
                
                // (not using IdealRtt because IdealRtt zeros the attitude)
                rtt[azimuth_step] = ( qti.roundTripTime + T_GRID +
                    T_RC) * S_TO_MS;
            }
            
            if(out_rtt_table) fwrite(&rtt,sizeof(double),RANGE_AZIMUTH_STEPS,ofp_rtt_table);
            
            //--------------------//
            // fit rtt parameters //
            //--------------------//

            double a, p, c;
            azimuth_fit(RANGE_AZIMUTH_STEPS, rtt, &a, &p, &c);

            if(rgc_limit>0) {
              double a1, c1;
              fit_rtt( &rtt[0], rgc_limit, a, p, c, &a1, &c1 );
              a = a1;
              c = c1;
            }

            if (opt_fixed) {
                // zero the amplitude and the phase
                a = 0.0;
                p = 0.0;
            }
            
            *(*(terms + orbit_step) + 0) = a;
            *(*(terms + orbit_step) + 1) = p;
            *(*(terms + orbit_step) + 2) = c;
        }

        //-----------//
        // set delay //
        //-----------//

        cds_beam_info->rangeTracker.SetRoundTripTime(terms);

        //-------//
        // clean //
        //-------//

        if (opt_clean) {
            float egw = (cds_beam_info->rxGateWidthDn -
                qscat.cds.txPulseWidthDn) * RANGE_GATE_NORMALIZER;
            cds_beam_info->rangeTracker.ClearAmpPhase();
            cds_beam_info->rangeTracker.QuantizeCenter(egw);
        }

        //---------------------------------------//
        // write out the receiver gate constants //
        //---------------------------------------//

        char filename[1024];
        sprintf(filename, "%s.%d", rgc_base, beam_idx + 1);
        if (! cds_beam_info->rangeTracker.WriteBinary(filename))
        {
            fprintf(stderr, "%s: error writing RGC file %s\n", command,
                filename);
            exit(1);
        }
    }
    
    if(out_rtt_table)  fclose(ofp_rtt_table);


    //----------------//
    // free the array //
    //----------------//

    free_array((void *)terms, 2, RANGE_ORBIT_STEPS, 3);

    return (0);
}