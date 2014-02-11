//==============================================================//
// Copyright (C) 2014, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//
//----------------------------------------------------------------------
// NAME
//    ephem_quat_fill_gaps.C
//
// SYNOPSIS
//    ephem_quat_fill_gaps -ie in_ephem -iq in_quats -oe out_pehem -oq out_quats 
//    -p period -tlims t_start t_end
//
// DESCRIPTION
//    Fill gaps in ephem and quats files.  Requires a fairly accurate period to 
//    do sinusoidal fits for the quaternions.  Will extrapolate to contain
//    interval [t_start,t_end] if required.  
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
//       0   Program executed successfully
//      >0  Program had an error
//
// NOTES
//    None.
//
// AUTHOR
//    Alex Fore
//    alexander.fore@jpl.nasa.gov
//----------------------------------------------------------------------

static const char rcs_id[] = "$Id$";

//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "Misc.h"
#include "Ephemeris.h"
#include "Quat.h"
#include "ConfigList.h"
#include "ConfigSim.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "SeaPac.h" 

//-----------//
// TEMPLATES //
//-----------//

template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class BufferedList<QuatRec>;
template class List<EarthPosition>;
template class List<QuatRec>;

const char usage_string[] = "-ie in_ephem -iq in_quats -oe out_ephem -oq out_quats -p period -tlims t_start t_end";

int main( int argc, char* argv[] ) {
  
  const char*  command        = no_path(argv[0]);
  char*        in_ephem_file  = NULL;  // gap-ridden ephem file
  char*        in_quats_file  = NULL;  // gap-ridden quat file
  char*        out_ephem_file = NULL;  // out ephem file
  char*        out_quats_file = NULL;  // out quat file
  double       period_guess   = 0;
  double       t_start        = 0;
  double       t_end          = 0;
  
  // in_ephem_file and in_quat_file must have exactly overlapping time-tags!
  int optind = 1;
  while ( (optind < argc) && (argv[optind][0]=='-') ) {
    std::string sw = argv[optind];
    if( sw=="-ie" ) {
      in_ephem_file=argv[++optind];
    } else if ( sw=="-iq" ) {
      in_quats_file=argv[++optind];
    } else if( sw=="-oe" ) {
      out_ephem_file=argv[++optind];
    } else if ( sw=="-oq" ) {
      out_quats_file=argv[++optind];
    } else if ( sw=="-tlims" ) {
      t_start = atof(argv[++optind]);
      t_end = atof(argv[++optind]);
    } else if ( sw=="-p" ) {
      period_guess=atof(argv[++optind]);
    } else {
      fprintf(stderr,"%s: %s\n",command,&usage_string[0]);
      exit(1);
    }
    ++optind;
  }
  
  if( !in_ephem_file || !in_quats_file || !out_ephem_file || !out_quats_file ||
      period_guess==0 || t_start==0 || t_end==0 ) {
    fprintf(stderr,"%s: %s\n",command,&usage_string[0]);
    exit(1);
  }
  
  // Read in ephem and quaternion data
  FILE* ifp_ephem = fopen(in_ephem_file,"r");
  fseek(ifp_ephem,0,SEEK_END);
  long int fsize = ftell(ifp_ephem);
  fseek(ifp_ephem,0,SEEK_SET);
  int n_ephem = fsize/(8*7);
  
  std::vector< double > time(n_ephem), azi(n_ephem), fitvar(n_ephem);
  std::vector< double > posx(n_ephem), posy(n_ephem), posz(n_ephem);
  std::vector< double > velx(n_ephem), vely(n_ephem), velz(n_ephem);
  std::vector< double > qw(n_ephem), qx(n_ephem), qy(n_ephem), qz(n_ephem);
  
  FILE* ifp_quats = fopen(in_quats_file,"r");
  fseek(ifp_quats,0,SEEK_END);
  fsize = ftell(ifp_quats);
  fseek(ifp_quats,0,SEEK_SET);
  int n_quats = fsize/(8*5);
  
  if( n_ephem != n_quats ) {
    fprintf(stderr,"%s: ephem file, %s, and quats file, %s, do not match!\n",
            command, in_ephem_file, in_quats_file );
    fclose(ifp_ephem);
    fclose(ifp_quats);
    exit(1);
  }
  
  for( int ii=0; ii<n_ephem; ++ii ) {
    fread( &time[ii], sizeof(double), 1, ifp_ephem );
    fread( &posx[ii], sizeof(double), 1, ifp_ephem );
    fread( &posy[ii], sizeof(double), 1, ifp_ephem );
    fread( &posz[ii], sizeof(double), 1, ifp_ephem );
    fread( &velx[ii], sizeof(double), 1, ifp_ephem );
    fread( &vely[ii], sizeof(double), 1, ifp_ephem );
    fread( &velz[ii], sizeof(double), 1, ifp_ephem );
    
    double qtime;
    fread( &qtime,    sizeof(double), 1, ifp_quats );
    fread( &qw[ii],   sizeof(double), 1, ifp_quats );
    fread( &qx[ii],   sizeof(double), 1, ifp_quats );
    fread( &qy[ii],   sizeof(double), 1, ifp_quats );
    fread( &qz[ii],   sizeof(double), 1, ifp_quats );
    
    if( qtime != time[ii] ) {
      fprintf(stderr,"%s: ephem file, %s, and quats file, %s, do not match!\n",
              command, in_ephem_file, in_quats_file );
      fclose(ifp_ephem);
      fclose(ifp_quats);
      exit(1);
    }
    
    // Use for harmonic fitting
    azi[ii]    = time[ii]*2*M_PI/period_guess;
    fitvar[ii] = 1;
  }
  
  // Do the harmonic fit for missing quaternions
  // Other inputs to specfit function.
  int start_term   = 0;
  int end_term     = 5;
  
  // Outputs from specift function.
  double amp_www[end_term], pha_www[end_term];
  double amp_xxx[end_term], pha_xxx[end_term];
  double amp_yyy[end_term], pha_yyy[end_term];
  double amp_zzz[end_term], pha_zzz[end_term];
  
  int fit_size = (int)azi.size();
  
  specfit(&azi[0], &qw[0], &fitvar[0], fit_size, start_term, end_term, &amp_www[0], &pha_www[0]);
  specfit(&azi[0], &qx[0], &fitvar[0], fit_size, start_term, end_term, &amp_xxx[0], &pha_xxx[0]);
  specfit(&azi[0], &qy[0], &fitvar[0], fit_size, start_term, end_term, &amp_yyy[0], &pha_yyy[0]);
  specfit(&azi[0], &qz[0], &fitvar[0], fit_size, start_term, end_term, &amp_zzz[0], &pha_zzz[0]);
  
  
  // Determine where gaps are
  std::vector< double > time_gap_start;
  std::vector< double > time_gap_end;
  for( int ii=1; ii<n_ephem; ++ii ) {
    if( time[ii]-time[ii-1] > 2.0 ) {
      time_gap_start.push_back(time[ii-1]);
      time_gap_end.push_back(time[ii]);
    }
  }
  
  int n_ephem_out = (int)floor(t_end-t_start+0.5);
  std::vector< double > time_out(n_ephem_out), azi_out(n_ephem_out);
  std::vector< double > posx_out(n_ephem_out), posy_out(n_ephem_out), posz_out(n_ephem_out);
  std::vector< double > velx_out(n_ephem_out), vely_out(n_ephem_out), velz_out(n_ephem_out);
  std::vector< double > qw_out(n_ephem_out), qx_out(n_ephem_out), qy_out(n_ephem_out), qz_out(n_ephem_out);
  std::vector< int > is_gap(n_ephem_out);
  
  Ephemeris ephem(in_ephem_file,n_ephem);
  QuatFile  quats(in_quats_file,n_ephem);
  
  Spacecraft    spacecraft;
  SpacecraftSim spacecraft_sim;    
  
  for ( int ii=0; ii<n_ephem_out; ++ii ) {
    double this_time = t_start+(double)ii;
    
    time_out[ii] = this_time;
    azi_out[ii]  = this_time * 2 * M_PI / period_guess;
    
    // check if this_time is in a gap period
    is_gap[ii] = 0;
    for( size_t i_gap=0; i_gap<time_gap_start.size(); ++i_gap ) {
      if( this_time > time_gap_start[i_gap] && this_time < time_gap_end[i_gap] ) 
        is_gap[ii] = 1;
      if( this_time < time[0] || this_time > time[time.size()-1] )
        is_gap[ii] = 1;
    }
    
    if( is_gap[ii] ) {
      // Leave ephem data to zeros for now; Use harmonic fit to fill in quaternions
      qw_out[ii] = 0;
      qx_out[ii] = 0;
      qy_out[ii] = 0;
      qz_out[ii] = 0;
      
      for( int iterm = 0; iterm < end_term; ++iterm ) {
        qw_out[ii] += amp_www[iterm] * cos( azi_out[ii] * double(iterm)+pha_www[iterm]);
        qx_out[ii] += amp_xxx[iterm] * cos( azi_out[ii] * double(iterm)+pha_xxx[iterm]);
        qy_out[ii] += amp_yyy[iterm] * cos( azi_out[ii] * double(iterm)+pha_yyy[iterm]);
        qz_out[ii] += amp_zzz[iterm] * cos( azi_out[ii] * double(iterm)+pha_zzz[iterm]);
      }
      
      double norm = sqrt( qw_out[ii]*qw_out[ii] + qx_out[ii]*qx_out[ii] 
                        + qy_out[ii]*qy_out[ii] + qz_out[ii]*qz_out[ii] );
      qw_out[ii] /= norm;
      qx_out[ii] /= norm;
      qy_out[ii] /= norm;
      qz_out[ii] /= norm;
      
    } else {
      // Interpolate to this_time using methods from Ephemeris and QuatFile
      // objects.
      OrbitState this_os;
      Quat       this_quat;
      
      ephem.GetOrbitState( this_time, 1, &this_os );
      posx_out[ii] = this_os.rsat.GetX()*1e3;
      posy_out[ii] = this_os.rsat.GetY()*1e3;
      posz_out[ii] = this_os.rsat.GetZ()*1e3;
      velx_out[ii] = this_os.vsat.GetX()*1e3;
      vely_out[ii] = this_os.vsat.GetY()*1e3;
      velz_out[ii] = this_os.vsat.GetZ()*1e3;
      
      quats.GetQuat( this_time, &this_quat );
      qw_out[ii] = this_quat.w;
      qx_out[ii] = this_quat.x;
      qy_out[ii] = this_quat.y;
      qz_out[ii] = this_quat.z;
    }
  }
  
  // Step through again, filling in missing ephem data
  for( int ii = 0; ii < n_ephem_out; ++ii ) {
    if( is_gap[ii]==0 )
      continue;
    
    // Search for ephem records that bracket time_out[ii] in 
    // gappy ephem file (i.e. input ephem file)
    int idx_backward = 0;
    for( int i_ephem=0; i_ephem<n_ephem; ++i_ephem) {
      if( time[i_ephem] <= time_out[ii] ) {
        idx_backward = i_ephem;
      }
    }
    
    int idx_forward = n_ephem-1;
    for( int i_ephem=n_ephem-1; i_ephem>=0; --i_ephem) {
      if( time[i_ephem] > time_out[ii] ) {
        idx_forward = i_ephem;
      }
    }
    
    // Compute orbit elements from each bracketing ephem record,
    // use orbit propagator to progagate both to time_out[ii].
    double nodal_period;
    double arg_lat;
    double long_asc_node;
    double orb_inclination;
    double orb_smaj_axis;
    double orb_eccen;
    double arg_of_per;
    double mean_anom;

    // helper vars to reduce code duplication
    // index 0 is backward, index 1 is forward
    int    idx_ref[2]  = { idx_backward, idx_forward };
    double time_ref[2] = { time[idx_backward], time[idx_forward] };
    
    double posx_ref[2], posy_ref[2], posz_ref[2];
    double velx_ref[2], vely_ref[2], velz_ref[2];
    
    for( int i_ref = 0; i_ref < 2; ++i_ref ) {
      compute_orbit_elements( posx[idx_ref[i_ref]], posy[idx_ref[i_ref]],
                  posz[idx_ref[i_ref]], velx[idx_ref[i_ref]], 
                  vely[idx_ref[i_ref]], velz[idx_ref[i_ref]], 
                  &nodal_period, &arg_lat, &long_asc_node, &orb_inclination,
                  &orb_smaj_axis, &orb_eccen, &arg_of_per, &mean_anom );
        
      spacecraft_sim.DefineOrbit( time_ref[i_ref], orb_smaj_axis*1e-3, 
                  orb_eccen, orb_inclination,
                  long_asc_node, arg_of_per,
                  mean_anom );
        
      spacecraft_sim.UpdateOrbit( time_out[ii], &spacecraft );
      posx_ref[i_ref] = spacecraft.orbitState.rsat.GetX()*1e3;
      posy_ref[i_ref] = spacecraft.orbitState.rsat.GetY()*1e3;
      posz_ref[i_ref] = spacecraft.orbitState.rsat.GetZ()*1e3;
      
      velx_ref[i_ref] = spacecraft.orbitState.vsat.GetX()*1e3;
      vely_ref[i_ref] = spacecraft.orbitState.vsat.GetY()*1e3;
      velz_ref[i_ref] = spacecraft.orbitState.vsat.GetZ()*1e3;
    }
    // Finally, linearly mix the two ephem predictions as time_out[ii]
    double forward_weight = (time_out[ii]-time[idx_backward]) / 
                            (time[idx_forward]-time[idx_backward]);
    
    double backward_weight = 1 - forward_weight;
    
    posx_out[ii] = backward_weight*posx_ref[0] + forward_weight*posx_ref[1];
    posy_out[ii] = backward_weight*posy_ref[0] + forward_weight*posy_ref[1];
    posz_out[ii] = backward_weight*posz_ref[0] + forward_weight*posz_ref[1];

    velx_out[ii] = backward_weight*velx_ref[0] + forward_weight*velx_ref[1];
    vely_out[ii] = backward_weight*vely_ref[0] + forward_weight*vely_ref[1];
    velz_out[ii] = backward_weight*velz_ref[0] + forward_weight*velz_ref[1]; 
  }
  
  // Write em out
  FILE* ofp_ephem = fopen(out_ephem_file,"w");
  FILE* ofp_quats = fopen(out_quats_file,"w");
  
  for( size_t ii=0; ii<time_out.size(); ++ii ) {
    fwrite(&time_out[ii],sizeof(double),1,ofp_ephem);
    fwrite(&posx_out[ii],sizeof(double),1,ofp_ephem);
    fwrite(&posy_out[ii],sizeof(double),1,ofp_ephem);
    fwrite(&posz_out[ii],sizeof(double),1,ofp_ephem);
    fwrite(&velx_out[ii],sizeof(double),1,ofp_ephem);
    fwrite(&vely_out[ii],sizeof(double),1,ofp_ephem);
    fwrite(&velz_out[ii],sizeof(double),1,ofp_ephem);
    
    fwrite(&time_out[ii],sizeof(double),1,ofp_quats);
    fwrite(&qw_out[ii],  sizeof(double),1,ofp_quats);
    fwrite(&qx_out[ii],  sizeof(double),1,ofp_quats);
    fwrite(&qy_out[ii],  sizeof(double),1,ofp_quats);
    fwrite(&qz_out[ii],  sizeof(double),1,ofp_quats);
  }
  fclose(ofp_ephem);
  fclose(ofp_quats);
  exit(0);
}
