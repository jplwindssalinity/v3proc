//==============================================================//
// Copyright (C) 2009-2012, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcs_id[] =
	"$Id$";
	
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Misc.h"
#include "ConfigList.h"
#include "ConfigSim.h"
#include "L1B.h"
#include "Meas.h"
#include "Tracking.h"
#include "Tracking.C"
#include "SpacecraftSim.h"
#include "AscatL1bReader.h"
#include "ETime.h"

using std::list;
using std::map; 

//-----------//
// TEMPLATES //
//-----------//

class AngleInterval;

template class List<AngleInterval>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
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

int main( int argc, char* argv[] ) {
  const char* command            = no_path(argv[0]);
  char*       ascat_szf_filename = NULL;
  char*       config_filename    = NULL;
  
  int config_entered      = 0;
  int debug_level_entered = 0;
  int debug_level         = 0;
  
  ETime          grid_start_time;
  ETime          grid_end_time;
  
  AscatFile      an_ascat_file;
  AscatSZFNode   an_ascat_szf_node;
  L1B            l1b;
  
  int            n_recs;
  int            n_nodes;
  int            ierr;
  
  // For converting between times...  
  ETime  QSCATSim_epoch;
  QSCATSim_epoch.FromCodeA("1970-01-01T00:00:00.000");

  ETime  ASCAT_epoch;
  ASCAT_epoch.FromCodeA("2000-01-01T00:00:00.000");

  double  ascat_epoch_to_qscatsim_epoch =  ASCAT_epoch.GetTime() - QSCATSim_epoch.GetTime();
  
  //------------------------//
  // parse the command line //
  //------------------------//
  
  const char* usage_string = "-c config_file.rdf [-d debug_level]\n";
  
  int optind = 1;
  while ( (optind < argc) && (argv[optind][0]=='-') ) {
    std::string sw = argv[optind];
    if( sw == "-c" )
      config_filename = argv[++optind]; 
    else if( sw == "-d" )
      debug_level = atoi(argv[++optind]); 
    else {
      printf("%s: unknown option %s\n",command,sw.c_str());
      printf("Usage: %s %s\n",command,usage_string);
      return (1);
    }
    ++optind;
  }
  
  if( !config_filename ) {
    fprintf(stderr,"%s: ERROR Must specifiy config RDF filename!\n", command);
    printf("Usage: %s %s\n",command,usage_string);
    return (1);
  }
  
  //---------------------//
  // read in config file //
  //---------------------//
  
  ConfigList config_list;
  if (! config_list.Read(config_filename)) {
    fprintf(stderr, "%s: error reading sim config file %s\n", command, config_filename);
    exit(1);
  }
  
  // Check for L1B_SZF_FILE keyword
  ascat_szf_filename = config_list.Get("L1B_SZF_FILE");
  if (ascat_szf_filename == NULL) {
    fprintf(stderr, "%s: config file must specify L1B_SZF_FILE\n", command);
    exit(1);
  }
  printf("%s: Using l1b SZF file: %s\n", command, ascat_szf_filename);
  
  if( ! ConfigL1B(&l1b, &config_list) ) {
    fprintf(stderr, "%s: error setting L1B filename from config file %s\n",
            command, config_filename);
    exit(1);
  }  
  
  // open ascat file
  if( an_ascat_file.open( ascat_szf_filename, &n_recs, &n_nodes ) != 0 ) { 
    fprintf(stderr, "%s: ERROR opening ascat szf file %s\n",command, ascat_szf_filename); 
    exit(1);
  }

  // open L1B file for writing
  if (! l1b.OpenForWriting()) {
    fprintf(stderr, "%s: ERROR opening L1B file %s for writing\n", command,
           l1b.GetOutputFilename());
    exit(1);
  } 

  
  if( debug_level ) {
    printf("ASC NODE time:    %30.8f\n",an_ascat_file.asc_node_time*24*60*60+ascat_epoch_to_qscatsim_epoch);
    printf("ASC NODE s/c R:   %12.6f %12.6f %12.6f\n",an_ascat_file.sc_pos_x_asc_node,
                                                      an_ascat_file.sc_pos_y_asc_node,
                                                      an_ascat_file.sc_pos_z_asc_node);
    printf("semi_major_axis:  %12.6f\n",an_ascat_file.semi_major_axis);
    printf("eccentricity:     %12.6f\n",an_ascat_file.eccentricity);
    printf("inclination:      %12.6f\n",an_ascat_file.inclination);
    printf("perigee_argument: %12.6f\n",an_ascat_file.perigee_argument);
    printf("ra_asc_node:      %12.6f\n",an_ascat_file.ra_asc_node);
    printf("mean_anomoly:     %12.6f\n",an_ascat_file.mean_anomoly);
  }
  
  
//--Set up the orbit propagator for the ASCAT orbit.
  Spacecraft     spacecraft;
  SpacecraftSim  spacecraft_sim;
  
  double lon_asc_node = atan2( an_ascat_file.sc_pos_y_asc_node, an_ascat_file.sc_pos_x_asc_node);
  
  lon_asc_node *= dtr;
  
//--Ascat orbital elements are in the MPHR header in each SZF file.
  spacecraft_sim.DefineOrbit( an_ascat_file.asc_node_time*24*60*60
                              +ascat_epoch_to_qscatsim_epoch,       // epoch time                  [sec]
                              an_ascat_file.semi_major_axis*1e-3,   // semi-major axis             [km]
                              an_ascat_file.eccentricity,           // eccentricity                [--]
                              an_ascat_file.inclination,            // inclination                 [deg]
                              lon_asc_node,            // longitude of ascending node [deg]
                              an_ascat_file.perigee_argument,       // argument of perigree        [deg]
                              an_ascat_file.mean_anomoly);          // mean anomoly at epoch       [deg]
  
//--get the s/c lon,lat @ asc node to use to set orbit location @ asc node.
  EarthPosition earth_pos_sc_asc_node;
  double        sc_lon_asc_node, sc_lat_asc_node, sc_alt_asc_node;

  earth_pos_sc_asc_node.SetPosition( an_ascat_file.sc_pos_x_asc_node * 1e-3,
                                     an_ascat_file.sc_pos_y_asc_node * 1e-3,
                                     an_ascat_file.sc_pos_z_asc_node * 1e-3 );
      
  earth_pos_sc_asc_node.GetAltLonGCLat( &sc_alt_asc_node, 
                                        &sc_lon_asc_node, 
                                        &sc_lat_asc_node );
      
  spacecraft_sim.LocationToOrbit( sc_lon_asc_node*rtd, 
                                  sc_lat_asc_node*rtd, 1 );
 
  if ( debug_level > 0 )
    printf("sc lon,lat,alt asc node: %f %f %f\n",
           sc_lon_asc_node*rtd, sc_lat_asc_node*rtd, sc_alt_asc_node);


  spacecraft_sim.Initialize( an_ascat_file.asc_node_time * 24.0 * 60.0 * 60.0 
                           + ascat_epoch_to_qscatsim_epoch );
  
  for( int i_rec = 0; i_rec < n_recs; ++i_rec ) {
    // Flush whatever is in the spotList object.
    l1b.frame.spotList.FreeContents();

    int isdummy;
    
    // read a data rec from szf file
    an_ascat_file.read_mdr(&isdummy);

    // Skip dummy records
    if(isdummy) continue;

    // Read a node from this record (need a time-tag to update ephemeris)
    an_ascat_file.get_node( 0, 0, &an_ascat_szf_node );
    
    double record_time_sec = an_ascat_szf_node.tm * 24.0 * 60.0 * 60.0+ascat_epoch_to_qscatsim_epoch;
    
    if( i_rec == 1 || i_rec == n_recs-1 ) {
      ETime t_bound;
      char  time_inst_str[CODE_A_TIME_LENGTH];
      char  time_grid_str[CODE_A_TIME_LENGTH];

      t_bound.SetTime( record_time_sec );
      t_bound.ToCodeA( &time_inst_str[0] );
      
      if( i_rec == 1 ) {
        t_bound.SetTime(record_time_sec-5*60);
        t_bound.ToCodeA( &time_grid_str[0] );
        
        grid_start_time = t_bound;
        
        fprintf( stdout, "INSTRUMENT_START_TIME = %s\n", time_inst_str );
        fprintf( stdout, "GRID_START_TIME       = %s\n", time_grid_str );
      } else {
        t_bound.SetTime(record_time_sec+5*60);
        t_bound.ToCodeA( &time_grid_str[0] );
        
        grid_end_time = t_bound;
        
        fprintf( stdout, "INSTRUMENT_END_TIME   = %s\n", time_inst_str );
        fprintf( stdout, "GRID_END_TIME         = %s\n", time_grid_str );
      }
    }
    
    // Update the ephemeris
    spacecraft_sim.UpdateOrbit( record_time_sec, &spacecraft );

    // I got the ASCAT yaw steering law from
    // http://oiswww.eumetsat.org/WEBOPS/eps-pg/Common/EPS-PG-AppB-MetopOrbit.htm
    float r_z   = spacecraft.orbitState.rsat.GetZ();
    float R_sc  = spacecraft.orbitState.rsat.Magnitude();
      
    float v_z   = spacecraft.orbitState.vsat.GetZ();
    float V_sc  = spacecraft.orbitState.vsat.Magnitude();
    
    float PSO   = atan2( r_z/R_sc, v_z/V_sc ); // orbit angle

    float yaw   = dtr *  3.9400 * cos(PSO) * (1-pow(dtr*3.9400*cos(PSO),2)/3.0 );
    float pitch = dtr *  0.1661 * sin(2*PSO);
    float roll  = dtr * -0.0508 * sin(PSO);
    
    spacecraft.attitude.SetRPY( roll, pitch, yaw );
    
    for( int i_beam = 0; i_beam < 6; ++i_beam ) {
      MeasSpot* new_meas_spot = new MeasSpot();
      
      new_meas_spot->time         = record_time_sec;
      new_meas_spot->scOrbitState = spacecraft.orbitState;
      new_meas_spot->scAttitude   = spacecraft.attitude;
      
      // Get the altitude from the orbit state and use for geolocating each measurement.
      double altitude, longitude, latitude;
      spacecraft.orbitState.rsat.GetAltLonGDLat( &altitude, &longitude, &latitude  );
                                                 
      // Needed for some geometry computations.
      EarthPosition    SC_nadir;                               
      CoordinateSwitch to_nadir_ENU;
      Vector3          look_vector, track_vector, surface_normal_at_nadir;
      double           sc_nadir_lon, sc_nadir_lat, sc_nadir_alt;
      
      // Get Nadir quantities for this orbit position...                    
      SC_nadir = spacecraft.orbitState.rsat.Nadir();
      SC_nadir.GetAltLonGDLat( &sc_nadir_alt, &sc_nadir_lon, &sc_nadir_lat );
      
      for( int i_node = 0; i_node < n_nodes; ++i_node ) {        
        an_ascat_file.get_node( i_node, i_beam, &an_ascat_szf_node );
        
        // Check that inc angle is in range for given beam
        // These are approximate; obtained from some documentation:
        //
        // EUMETSAT POLAR SYSTEM/MetOp Research Announcement of Opportunity
        // Scientific Exploitation of EPS Data & Products 9 June 2004
        if( i_beam == 1 || i_beam == 4 ) { // middle beams
          if( an_ascat_szf_node.t0 < 25.0 || an_ascat_szf_node.t0 > 55.0 ) continue;
        } else  { // forward or aft looking beams.
          if( an_ascat_szf_node.t0 < 33.7 || an_ascat_szf_node.t0 > 64.0 ) continue;
        }
        // Check that this node does not have odd values in and on some flags.
        if( an_ascat_szf_node.s0  < -2000 || 
            an_ascat_szf_node.a0  < -2000 || 
            an_ascat_szf_node.lon < -2000 ||
            an_ascat_szf_node.t0  > 600   ||
            an_ascat_szf_node.fgen1 >= 4  || // >= 4 when one of the aggregate non-nominal flags is set.
            an_ascat_szf_node.fgen2 == 1  || // solar array refl is bit 0 of fgen2
            an_ascat_szf_node.fgen2 == 3     )
          continue;        

        if( an_ascat_szf_node.s0 > 30 ) {
          int dummy = 0; // break point for debugging...
        }

        Meas* new_meas = new Meas();
        
        new_meas->value = pow( 10.0, 0.1 * an_ascat_szf_node.s0 ); // linear units for simga-0
                
        if( an_ascat_szf_node.fgen2 >= 2 )
          new_meas->landFlag = 1;
        else
          new_meas->landFlag = 0;
                
        new_meas->centroid.SetAltLonGDLat( 0.0,
                                           an_ascat_szf_node.lon * dtr,
                                           an_ascat_szf_node.lat * dtr );
        
        new_meas->measType       = Meas::C_BAND_VV_MEAS_TYPE;
        new_meas->eastAzimuth    = gs_deg_to_pe_rad( an_ascat_szf_node.a0 );
        new_meas->incidenceAngle = an_ascat_szf_node.t0 * dtr;
        new_meas->beamIdx        = i_beam;
        
        new_meas->startSliceIdx  = i_node;
        new_meas->numSlices      = -1;
        
        
        // Compute track angle, scan angle, scan angle relative to track angle.
        surface_normal_at_nadir = SC_nadir.Normal();
        
        look_vector             = new_meas->centroid - spacecraft.orbitState.rsat;
        track_vector            = spacecraft.orbitState.vsat;
        
        // project look_vector and s/c velocity vector onto surface of earth.
        look_vector             = look_vector - surface_normal_at_nadir 
                                * ( surface_normal_at_nadir % look_vector );

        track_vector            = track_vector - surface_normal_at_nadir 
                                * ( surface_normal_at_nadir % track_vector );
        
        // Compute the coordinate transformation to ENU @ s/c nadir point.
        to_nadir_ENU = SC_nadir.SurfaceCoordinateSystem();
        
        // Transform to ENU system
        look_vector  = to_nadir_ENU.Forward( look_vector );
        track_vector = to_nadir_ENU.Forward( track_vector );

        // Compute track and scan angles (relative east, measured counter-clockwise).
        double scan_ang  = atan2( look_vector.GetY(),  look_vector.GetX()  )*rtd; // E is X, N is Y...
        double track_ang = atan2( track_vector.GetY(), track_vector.GetX() )*rtd;

        
        // scanAngle := scan angle relative to track angle, measured with same handedness
        // as scan_ang and track_ang.
        if( scan_ang-track_ang < 0 ) 
          new_meas->scanAngle = (scan_ang-track_ang+360) * dtr;
        else if ( scan_ang-track_ang > 360 )
          new_meas->scanAngle = (scan_ang-track_ang-360) * dtr;
        else
          new_meas->scanAngle = (scan_ang-track_ang)     * dtr;
        
        
        double n_looks, sigma_nesz_dB, inc_deg, sigma_nesz;
        
        inc_deg = new_meas->incidenceAngle * rtd;
        
        // these values are not totally refined; definitely there is some room for improvement...
        // 12-9-2009 AGF
        n_looks = 2.89*pow(10,-4)*pow(inc_deg,3) 
                - 2.11*pow(10,-2)*pow(inc_deg,2) 
                + 4.83*pow(10,-2)*inc_deg 
                + 2.16*pow(10,1);
        
        sigma_nesz_dB = 1.21*pow(10,-2)*pow(inc_deg,2) 
                      - 1.47*pow(10,0)*inc_deg 
                      + 1.13*pow(10,1) - 2;
        
        sigma_nesz = pow(10,0.1*sigma_nesz_dB);

        new_meas->XK = 1.0;
        new_meas->EnSlice = new_meas->XK * sigma_nesz;

        
        new_meas->A = 1.0 + 1.0 / n_looks;
        new_meas->B = 2 * sigma_nesz / n_looks;
        new_meas->C = pow(sigma_nesz,2) / n_looks;
        
        new_meas->azimuth_width   = 20; // [km]; from documentation
        new_meas->range_width     = 10; // [km]; from documentation

        new_meas_spot->Append(new_meas);
      }
      
      l1b.frame.spotList.Append(new_meas_spot);
    }
    
    // write data rec
    if( ! l1b.WriteDataRec() ) {
      fprintf(stderr, "%s: writing to %s failed.\n", command, l1b.GetOutputFilename() );
      exit(1);
    }
  }
  
  l1b.Close();
  
  char* ephemeris_filename = config_list.Get(EPHEMERIS_FILE_KEYWORD);
  if (ephemeris_filename == NULL) {
    fprintf(stderr, "%s: ERROR getting ephemeris filename from config file.\n", command);
    exit(1);
  }
  // open ephem file for writing.
  FILE* eph_fp = fopen(ephemeris_filename, "w");
  if (eph_fp == NULL) {
    fprintf(stderr, "%s: ERROR opening ephemeris file %s\n", command,ephemeris_filename);
    exit(1);
  }  
  
  spacecraft_sim.LocationToOrbit( sc_lon_asc_node*rtd, sc_lat_asc_node*rtd, 1 );
  
  spacecraft_sim.Initialize( an_ascat_file.asc_node_time * 24.0 * 60.0 * 60.0 
                           + ascat_epoch_to_qscatsim_epoch );
  
  
  // Write out an ephemeris file for subsequent processing.
  int NUM_MIN = ceil( (grid_end_time.GetTime()-grid_start_time.GetTime())/60);
  
  // Add 20 min to both ends
  for( int i_min = -20; i_min <= NUM_MIN+20; ++i_min )  {
    double curr_time_sec = grid_start_time.GetTime() + 60*double(i_min);
    
    spacecraft_sim.UpdateOrbit( curr_time_sec, &spacecraft );

    // I got the ASCAT yaw steering law from
    // http://oiswww.eumetsat.org/WEBOPS/eps-pg/Common/EPS-PG-AppB-MetopOrbit.htm
    float r_z   = spacecraft.orbitState.rsat.GetZ();
    float R_sc  = spacecraft.orbitState.rsat.Magnitude();
      
    float v_z   = spacecraft.orbitState.vsat.GetZ();
    float V_sc  = spacecraft.orbitState.vsat.Magnitude();
    
    float PSO   = atan2( r_z/R_sc, v_z/V_sc ); // orbit angle

    float yaw   = dtr *  3.9400 * cos(PSO) * (1-pow(dtr*3.9400*cos(PSO),2)/3.0 );
    float pitch = dtr *  0.1661 * sin(2*PSO);
    float roll  = dtr * -0.0508 * sin(PSO);
    
    spacecraft.attitude.SetRPY( roll, pitch, yaw );
    
    spacecraft.orbitState.Write(eph_fp);
    //spacecraft.attitude.Write(att_fp);
    
  }
  
  an_ascat_file.close();
  fclose(eph_fp);
  return (0);
}	
