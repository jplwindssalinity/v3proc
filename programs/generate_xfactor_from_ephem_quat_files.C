//==============================================================//
// Copyright (C) 2013, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    generate_xfactor_from_ephem_quat_files.C
//
// SYNOPSIS
//    generate_xfactor_from_ephem_quat_files [ -b ] <sim_config_file> <DTC_base>
//
// DESCRIPTION
//    Generates a kfactors for the given ephemeris, quaternion files, and 
//    config file parameters.
//
// OPTIONS
//
// OPERANDS
//
// EXAMPLES
//
// ENVIRONMENT
//    Not environment dependent.
//
// EXIT STATUS
//    The following exit values are returned:
//       0  Program executed successfully
//      >0  Program had an error
//
// NOTES
//    Base on generate_rgc_for_prescribed_attitude.C
//
// AUTHORS
//    Alex Fore
//----------------------------------------------------------------------

//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcs_id[] = "@(#) $Id$";

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
#include "AccurateGeom.h"
#include "XTable.h"

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

#define ORBIT_STEPS    32
#define AZIMUTH_STEPS  36

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

const char usage_string[] = "-c config_file -e ephem_file -q quats_file -o xfactor_file <-s start_rev> <-fbb spot_bbshift.dat>";

//--------------//
// MAIN PROGRAM //
//--------------//

int main( int argc, char* argv[] ) {
  //------------------------//
  // parse the command line //
  //------------------------//

  const char* command = no_path(argv[0]);
  char*       out_xfactor_file = NULL;
  char*       config_file      = NULL;
  char*       ephem_file       = NULL;
  char*       quat_file        = NULL;
  char*       fbbshift_file    = NULL;
  int         start_rev        = 0;
  
  int optind    = 1;
  while( (optind < argc) && (argv[optind][0]=='-') ) {
    std::string sw = argv[optind];
    if( sw == "-o" ) {
      out_xfactor_file = argv[++optind];
    } else if( sw == "-c" ) {
      config_file = argv[++optind];
    } else if( sw == "-e" ) {
      ephem_file = argv[++optind];
    } else if( sw == "-q" ) {
      quat_file = argv[++optind];
    } else if( sw == "-s" ) {
      start_rev = atoi(argv[++optind]);
    } else if( sw == "-fbb" ) {
      fbbshift_file = argv[++optind];
    } else {
      fprintf(stderr,"%s: %s\n",command,&usage_string[0]);
      exit(1);
    }
    ++optind;
  }

  // Must specify "-out_xfactor_file"
  if( !config_file || !ephem_file || !quat_file || !out_xfactor_file ) {
    fprintf(stderr,"%s: %s\n",command,&usage_string[0]);
    exit(1);
  }

  //--------------------------------//
  // read in simulation config file //
  //--------------------------------//

  ConfigList config_list;
  if (! config_list.Read(config_file)) {
    fprintf(stderr, "%s: error reading sim config file %s\n", command, config_file);
    exit(1);
  }

  //----------------------------------------------//
  // create a spacecraft and spacecraft simulator //
  //----------------------------------------------//

  Spacecraft spacecraft;
  if (! ConfigSpacecraft(&spacecraft, &config_list)) {
    fprintf(stderr, "%s: error configuring spacecraft simulator\n", command);
    exit(1);
  }

  SpacecraftSim spacecraft_sim;
  if (! ConfigSpacecraftSim(&spacecraft_sim, &config_list)) {
    fprintf(stderr, "%s: error configuring spacecraft simulator\n", command);
    exit(1);
  }
  spacecraft_sim.LocationToOrbit(0.0, 0.0, 1);

  //--------------------------------------//
  // create a QSCAT and a QSCAT simulator //
  //--------------------------------------//

  Qscat qscat;
  if (! ConfigQscat(&qscat, &config_list)) {
    fprintf(stderr, "%s: error configuring QSCAT\n", command);
    exit(1);
  }

  QscatSim qscat_sim;
  if (! ConfigQscatSim(&qscat_sim, &config_list)) {
    fprintf(stderr, "%s: error configuring instrument simulator\n", command);
    exit(1);
  }

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

  // Compute orbit period from average difference of consecutive ascending node times
  double orbit_period = 0;
  for( size_t ii=0;ii<asc_node_times.size()-1; ++ii) 
    orbit_period += asc_node_times[ii+1]-asc_node_times[ii];

  orbit_period /= double(asc_node_times.size()-1);
  //     printf("orbit_period: %f, revs used: %zd\n",orbit_period,asc_node_times.size()-1);

  //-----------//
  // variables //
  //-----------//

  OrbitState* orbit_state = &(spacecraft.orbitState);
  Attitude* attitude = &(spacecraft.attitude);

  double orbit_step_size   = orbit_period / (double)ORBIT_STEPS;
  double azimuth_step_size = two_pi / (double)AZIMUTH_STEPS;
  int    num_slices        = qscat.ses.GetTotalSliceCount();
  
  float fbb_shift[NUMBER_OF_QSCAT_BEAMS][ORBIT_STEPS][AZIMUTH_STEPS];
  
  XTable x_table( NUMBER_OF_QSCAT_BEAMS, AZIMUTH_STEPS, ORBIT_STEPS, 
                  qscat.ses.scienceSlicesPerSpot, qscat.ses.guardSlicesPerSide, 
                  qscat.ses.scienceSliceBandwidth, qscat.ses.guardSliceBandwidth );
  
  //----------------------------//
  // select encoder information //
  //----------------------------//

  unsigned int cds_encoder_offset_dn = 0;
  double sas_encoder_offset = 0.0;
  switch (qscat.sas.encoderElectronics) {
    case ENCODER_A:
      cds_encoder_offset_dn = qscat.cds.encoderAOffset;
      sas_encoder_offset = qscat.sas.encoderAOffset * dtr;
      break;
    case ENCODER_B:
      cds_encoder_offset_dn = qscat.cds.encoderBOffset;
      sas_encoder_offset = qscat.sas.encoderBOffset * dtr;
      break;
    default:
      fprintf(stderr, "%s: unknown encoder electronics\n", command);
      exit(1);
  }
  double cds_encoder_offset = (double)cds_encoder_offset_dn * two_pi / (double)ENCODER_N;

  //-------------------------------------------//
  // determine spin rate in radians per second //
  //-------------------------------------------//

  double assumed_spin_rate = qscat.cds.GetAssumedSpinRate();
  assumed_spin_rate *= rpm_to_radps;

  //-----------------//
  // loop over beams //
  //-----------------//

  for (int beam_idx = 0; beam_idx < NUMBER_OF_QSCAT_BEAMS; beam_idx++) {
    //---------------------------//
    // get the beam offset angle //
    //---------------------------//

    unsigned int cds_beam_offset_dn = 0;
    switch (beam_idx) {
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
    double cds_beam_offset = (double)cds_beam_offset_dn * two_pi / (double)ENCODER_N;

    //--------------------------//
    // allocate Doppler tracker //
    //--------------------------//

    qscat.cds.currentBeamIdx = beam_idx;
    Beam* beam = qscat.GetCurrentBeam();

    SesBeamInfo*    ses_beam_info   = qscat.GetCurrentSesBeamInfo();
    CdsBeamInfo*    cds_beam_info   = qscat.GetCurrentCdsBeamInfo();
    DopplerTracker* doppler_tracker = &(cds_beam_info->dopplerTracker);
    RangeTracker*   range_tracker   = &(cds_beam_info->rangeTracker);

    //------------------------------//
    // start at an equator crossing //
    //------------------------------//
    double start_time;
    if( start_rev == -1 ) {
      // use last node-to-node orbit in ephem / quat files
      start_time = asc_node_times[asc_node_times.size()-2];
    } else if ( start_rev< asc_node_times.size()-1) {
      start_time = asc_node_times[start_rev];
    } else {
      fprintf(stderr,"Error: bad value for start_rev\n");
      exit(1);
    }

    qscat.cds.SetEqxTime(start_time);

    //------------//
    // initialize //
    //------------//

    if (! qscat_sim.Initialize(&qscat)) {
      fprintf(stderr, "%s: error initializing the QSCAT simulator\n", command);
      exit(1);
    }

    if (! spacecraft_sim.Initialize(start_time)) {
      fprintf(stderr, "%s: error initializing spacecraft simulator\n", command);
      exit(1);
    }

    //--------------------//
    // loop through orbit //
    //--------------------//

    for (int orbit_step = 0; orbit_step < ORBIT_STEPS; orbit_step++ ) {
      //--------------------//
      // calculate the time //
      //--------------------//

      // addition of 0.5 centers on orbit_step
      double time = start_time + orbit_step_size * ((double)orbit_step + 0.5);
      qscat.cds.SetTime(time);
      
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
  
      // For adding extra pitch offsets for ISS/RapidSCAT
      //spacecraft.attitude.SetPitch(spacecraft.attitude.GetPitch()-3*dtr);
  
      // Interpolate ephem and overwrite that in spacecraft object
      ephem.GetOrbitState( time, EPHEMERIS_INTERP_ORDER, &(spacecraft.orbitState) );
      //--------------------------------------------------------------

      //----------------------//
      // step through azimuth //
      //----------------------//

      for (int azimuth_step = 0; azimuth_step < AZIMUTH_STEPS; azimuth_step++ ) {
        //--------------------------------//
        // calculate azimuth angle to use //
        //--------------------------------//

        // The table needs to be built for the CDS algorithm,
        // but we need to determine the actual antenna azimuth
        // angle in order to do the correct calculations.  The
        // following code starts from the CDS azimuth value,
        // backtracks to the original sampled encoder and then
        // calculates the actual antenna azimuth at the ground
        // impact time.  This method of doing the calculation will
        // allow for things like changes in the antenna spin rate
        // which will affect the actual antenna azimuth but not
        // the CDS estimation (which uses hardcoded spin rates).

        // start with the azimuth angle to be used by the CDS
        double cds_azimuth = azimuth_step_size * (double)azimuth_step;
        double azimuth = cds_azimuth;

        // subtract the beam offset
        azimuth -= cds_beam_offset;

        // subtract an estimate of the centering offset
        // uses an estimate of the round trip time as
        // the previous pulses round trip time
        qscat.sas.antenna.SetEncoderAzimuthAngle(cds_azimuth);
                      
        Antenna* antenna = &(qscat.sas.antenna);
        CoordinateSwitch antenna_frame_to_gc =
            AntennaFrameToGC(orbit_state, attitude, antenna, azimuth);
        double look, az;
        if (! GetPeakSpatialResponse2(&antenna_frame_to_gc,
          &spacecraft, beam, antenna->spinRate, &look, &az)) {
          fprintf(stderr, "%s: error finding peak spatial response\n", command);
          exit(1);
        }

        Vector3 vector;
        vector.SphericalSet(1.0, look, az);

        QscatTargetInfo qti;
        if (! qscat.TargetInfo(&antenna_frame_to_gc, &spacecraft, vector, &qti)) {
          fprintf(stderr, "%s: error finding round trip time\n", command);
          exit(1);
        }

        // then apply to the azimuth angle
        double delay = (qti.roundTripTime + qscat.ses.txPulseWidth) / 2.0;
        azimuth -= (delay * assumed_spin_rate);

        // subtract the cds encoder offset
        azimuth -= cds_encoder_offset;

        // subtract the internal (sampling) delay angle
        double cds_pri = MS_TO_S * (double)qscat.cds.priDn / 10.0;
        azimuth -= (cds_pri * assumed_spin_rate);

        // add the actual sampling delay angle
        azimuth += (qscat.ses.pri * qscat.sas.antenna.spinRate);

        // and get to the center of the transmit pulse so that
        // the two-way gain product is formed correctly
        azimuth += (qscat.ses.txPulseWidth * qscat.sas.antenna.spinRate / 2.0);

        // apply the sas encoder offset
        azimuth += sas_encoder_offset;

        //---------------------------//
        // set the Tx center azimuth //
        //---------------------------//

        qscat.sas.antenna.SetTxCenterAzimuthAngle(azimuth);

        //-------------------------//
        // set the antenna azimuth //
        //-------------------------//

        qscat.sas.antenna.SetEncoderAzimuthAngle(azimuth);

        //-----------------------------------------------------//
        // determine the encoder value to use in the algorithm //
        //-----------------------------------------------------//
        unsigned short encoder = qscat.sas.AzimuthToEncoder(cds_azimuth);
        unsigned short orbstep = qscat.cds.SetAndGetOrbitStep();
        
        //------------------------------//
        // calculate receiver gate info //
        //------------------------------//
      
        qscat.cds.rxGateDelayDn = 0;
        float rx_gate_delay_fdn = 0.0;
            
        if(qscat.cds.useRgc) {
          // tracking algorithm
          range_tracker->GetRxGateDelay( orbstep, encoder, 
            cds_beam_info->rxGateWidthDn, qscat.cds.txPulseWidthDn,
            &(qscat.cds.rxGateDelayDn), &rx_gate_delay_fdn );
          qscat.ses.CmdRxGateDelayDn(qscat.cds.rxGateDelayDn);
          //qscat.ses.CmdRxGateDelayFdn(rx_gate_delay_fdn);
        } else {
          // ideal delay
          float rtt   = qscat.IdealRtt(&spacecraft, 1);
          float delay = rtt + 
            (qscat.ses.txPulseWidth - ses_beam_info->rxGateWidth) / 2.0;
          qscat.ses.CmdRxGateDelayEu(delay);
        }
        //----------------------------//
        // calculate the tx frequency //
        //----------------------------//
        if(qscat.cds.useDtc) {
          doppler_tracker->GetCommandedDoppler( orbstep, encoder, 
            qscat.cds.rxGateDelayDn, rx_gate_delay_fdn, &(qscat.cds.txDopplerDn));
          qscat.ses.CmdTxDopplerDn(qscat.cds.txDopplerDn);
        } else {
          // 1 means to use spacecraft attitude to compute Doppler
          qscat.IdealCommandedDoppler(&spacecraft,NULL,1);
        }
        
        // Call Qscat::TargetInfo() again to get base-band frequency shift
        if (! qscat.TargetInfo(&antenna_frame_to_gc, &spacecraft, vector, &qti)) {
          fprintf(stderr, "%s: error finding round trip time\n", command);
          exit(1);
        }
        
        fbb_shift[beam_idx][orbit_step][azimuth_step] = qti.basebandFreq;
        
        // Make the slices for this pulse and geolocate them
        MeasSpot meas_spot;
        qscat.MakeSlices(&meas_spot);
        qscat.LocateSliceCentroids(&spacecraft,&meas_spot);
            
        // Loop over slices and compute the X-factors
        for( Meas* meas = meas_spot.GetHead(); meas; meas = meas_spot.GetNext() ) {
          // Set the measurement type for each meas
          Beam* beam     = qscat.GetCurrentBeam();
          meas->measType = PolToMeasType(beam->polarization);
          
          // Compute the Xfactor...
          float this_X, this_X_int;
          //qscat_sim.ComputeXfactor( &spacecraft, &qscat, meas, &this_X );
          
          IntegrateSlice( &spacecraft, &qscat, meas, qscat_sim.numLookStepsPerSlice,
                          qscat_sim.azimuthIntegrationRange, 
                          qscat_sim.azimuthStepSize, 
                          qscat_sim.rangeGateClipping, &this_X_int );
          
          // Remove peak gain from X_int
          this_X_int /= (beam->peakGain * beam->peakGain);

          fprintf(stdout,"%d %d %d %d %f %f %f\n", beam_idx, orbit_step, azimuth_step,
             meas->startSliceIdx, 10*log10(this_X_int), qscat.ses.txDoppler, 
             qscat.sas.antenna.txCenterAzimuthAngle );

          // Stick in output arrays
          int abs_idx;
          rel_to_abs_idx( meas->startSliceIdx, num_slices, &abs_idx );
          float orbit_position = ((double)(orbit_step)+0.5)/(double)ORBIT_STEPS;
          
          if(!x_table.AddEntry( this_X_int, beam_idx, azimuth, orbit_position, abs_idx)) {
            fprintf(stderr,"%s: Error adding to Xtable: %d %f %f %d\n", 
                    command, beam_idx, azimuth, orbit_position, abs_idx );
            exit(1);
          }
        }
      }
    }
  }
  
  if( fbbshift_file ) {
    FILE* ofp = fopen(fbbshift_file,"w");
    if( ofp==NULL ) {
      fprintf(stderr,"%s: Error opening base-band freq shift file: %s\n",command,fbbshift_file);
      fclose(ofp);
    } else {
      int n_beams = NUMBER_OF_QSCAT_BEAMS;
      int n_orb   = ORBIT_STEPS;
      int n_azi   = AZIMUTH_STEPS;
      
      size_t out_size = n_beams*n_orb*n_azi;
      if( fwrite(&n_beams,sizeof(int),1,ofp) != 1 ||
          fwrite(&n_orb,sizeof(int),1,ofp)   != 1 ||
          fwrite(&n_azi,sizeof(int),1,ofp)   != 1 ||
          fwrite(&fbb_shift[0][0][0],sizeof(float),out_size,ofp) != out_size ) {
        fprintf(stderr,"%s: Error writing base-band freqs to file: %s\n",command,fbbshift_file);
      }
      fclose(ofp);
    }
  }
  
  if(!x_table.SetFilename(out_xfactor_file) || !x_table.Write() ) {
    fprintf(stderr,"%s: Error writing XTable to: %s\n", command, out_xfactor_file );
    exit(1);
  }
  return (0);
}
