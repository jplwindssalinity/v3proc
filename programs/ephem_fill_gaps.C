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
#include "ConfigList.h"
#include "L1A.h"
#include "ConfigSim.h"
#include "L1AToL1B.h"
#include "Tracking.h"
#include "Tracking.C"
#include "QscatConfig.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "SeaPac.h"
#include "AttenMap.h"
#include "ETime.h"

using namespace std;
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

#define RAD_TO_DEG(x) ((x)*180.0f/M_PI)
#define DEG_TO_RAD(x) ((x)*M_PI/180.0f)

int
main( int        argc, char*    argv[]) {
  const char*  command = no_path(argv[0]);
  char*        infile  = NULL;
  char*        outfile = NULL;
  
  int optind = 1;
  while ( (optind < argc) && (argv[optind][0]=='-') ) {
    std::string sw = argv[optind];
    if( sw=="-i" ) {
      infile=argv[++optind];
    } else if ( sw=="-o" ) {
      outfile=argv[++optind];
    } else {
      fprintf(stderr,"%s: -i ephem_input.dat -o ephem_output.dat\n",command);
      exit(1);
    }
    ++optind;
  }
  
  if( !infile || !outfile) {
    fprintf(stderr,"%s: -i ephem_input.dat -o ephem_output.dat\n",command);
    exit(1);
  }
  
  FILE* ifp = fopen(infile,"r");
  
  // Compute number of records in ephem file.
  fseek(ifp,0,SEEK_END);
  long int fsize = ftell(ifp);
  fseek(ifp,0,SEEK_SET);
  
  int n_ephem = fsize/(8*7);
  
  // Allocate storage for ephem vectors
  std::vector< double > time(n_ephem);
  std::vector< double > posx(n_ephem), posx_fwd(n_ephem), posx_bak(n_ephem), posx_int(n_ephem);
  std::vector< double > posy(n_ephem), posy_fwd(n_ephem), posy_bak(n_ephem), posy_int(n_ephem);
  std::vector< double > posz(n_ephem), posz_fwd(n_ephem), posz_bak(n_ephem), posz_int(n_ephem);
  std::vector< double > velx(n_ephem), velx_fwd(n_ephem), velx_bak(n_ephem), velx_int(n_ephem);
  std::vector< double > vely(n_ephem), vely_fwd(n_ephem), vely_bak(n_ephem), vely_int(n_ephem);
  std::vector< double > velz(n_ephem), velz_fwd(n_ephem), velz_bak(n_ephem), velz_int(n_ephem);
  std::vector< int    > missing(n_ephem);
  
  for( int ii=0; ii< n_ephem; ++ii ) {
    fread( &time[ii], sizeof(double), 1, ifp );
    fread( &posx[ii], sizeof(double), 1, ifp );
    fread( &posy[ii], sizeof(double), 1, ifp );
    fread( &posz[ii], sizeof(double), 1, ifp );
    fread( &velx[ii], sizeof(double), 1, ifp );
    fread( &vely[ii], sizeof(double), 1, ifp );
    fread( &velz[ii], sizeof(double), 1, ifp );
    
    // Check for missing ephem records
    missing[ii]=0;
    if( posx[ii]!=posx[ii] || posy[ii]!=posy[ii] || posz[ii]!=posz[ii] ) missing[ii]=1;
  }
  fclose(ifp);

  Spacecraft    spacecraft;
  SpacecraftSim spacecraft_sim;    
  OrbitState    os;
  
  double nodal_period, arg_lat, long_asc_node;
  double orb_inclination, orb_smaj_axis, orb_eccen;
  double arg_of_per, mean_anom;
  
  for( int ii=0; ii<n_ephem; ++ii ) {
    // If a good ephem point and next one missing, 
    // use this one to compute orbit elements
    // and define the spacecraft sim object.
    if( ii<n_ephem-1 && missing[ii]==0 && missing[ii+1]==1 ) {
      if(!compute_orbit_elements( posx[ii], posy[ii], posz[ii],
                                  velx[ii], vely[ii], velz[ii],
                                  &nodal_period,  &arg_lat, 
                                  &long_asc_node, &orb_inclination, 
                                  &orb_smaj_axis, &orb_eccen, 
                                  &arg_of_per,    &mean_anom ) ) {
        fprintf(stdout,"Error computing orbit elements!\n");
        exit(1);
      }
      
      spacecraft_sim.DefineOrbit( time[ii], orb_smaj_axis*1e-3, 
                                  orb_eccen, orb_inclination,
                                  long_asc_node, arg_of_per,
                                  mean_anom );
                                  
      posx_fwd[ii] = posx[ii];
      posy_fwd[ii] = posy[ii];
      posz_fwd[ii] = posz[ii];
      
      velx_fwd[ii] = velx[ii];
      vely_fwd[ii] = vely[ii];
      velz_fwd[ii] = velz[ii];
      
    // Use the spacecraft sim object to update the orbit if this ephem
    // data is missing
    } else if( missing[ii] ) {
      spacecraft_sim.UpdateOrbit( time[ii], &spacecraft );
      
      posx_fwd[ii] = spacecraft.orbitState.rsat.GetX()*1e3;
      posy_fwd[ii] = spacecraft.orbitState.rsat.GetY()*1e3;
      posz_fwd[ii] = spacecraft.orbitState.rsat.GetZ()*1e3;
      
      velx_fwd[ii] = spacecraft.orbitState.vsat.GetX()*1e3;
      vely_fwd[ii] = spacecraft.orbitState.vsat.GetY()*1e3;
      velz_fwd[ii] = spacecraft.orbitState.vsat.GetZ()*1e3;
    } else {
      posx_fwd[ii] = posx[ii];
      posy_fwd[ii] = posy[ii];
      posz_fwd[ii] = posz[ii];
      
      velx_fwd[ii] = velx[ii];
      vely_fwd[ii] = vely[ii];
      velz_fwd[ii] = velz[ii];
    }
  }
  
  for( int ii=n_ephem; ii>=0; --ii ) {
    // If a good ephem point and previous one missing, 
    // use this one to compute orbit elements
    // and define the spacecraft sim object.
    if( ii>0 && missing[ii]==0 && missing[ii-1]==1 ) {
      if(!compute_orbit_elements( posx[ii], posy[ii], posz[ii],
                                  velx[ii], vely[ii], velz[ii],
                                  &nodal_period,  &arg_lat, 
                                  &long_asc_node, &orb_inclination, 
                                  &orb_smaj_axis, &orb_eccen, 
                                  &arg_of_per,    &mean_anom ) ) {
        fprintf(stdout,"Error computing orbit elements!\n");
        exit(1);
      }
      
      spacecraft_sim.DefineOrbit( time[ii], orb_smaj_axis*1e-3, 
                                  orb_eccen, orb_inclination,
                                  long_asc_node, arg_of_per,
                                  mean_anom );
                                  
      posx_bak[ii] = posx[ii];
      posy_bak[ii] = posy[ii];
      posz_bak[ii] = posz[ii];
      
      velx_bak[ii] = velx[ii];
      vely_bak[ii] = vely[ii];
      velz_bak[ii] = velz[ii];
      
    // Use the spacecraft sim object to update the orbit if this ephem
    // data is missing
    } else if( missing[ii] ) {
      spacecraft_sim.UpdateOrbit( time[ii], &spacecraft );
      
      posx_bak[ii] = spacecraft.orbitState.rsat.GetX()*1e3;
      posy_bak[ii] = spacecraft.orbitState.rsat.GetY()*1e3;
      posz_bak[ii] = spacecraft.orbitState.rsat.GetZ()*1e3;
      
      velx_bak[ii] = spacecraft.orbitState.vsat.GetX()*1e3;
      vely_bak[ii] = spacecraft.orbitState.vsat.GetY()*1e3;
      velz_bak[ii] = spacecraft.orbitState.vsat.GetZ()*1e3;
    } else {
      posx_bak[ii] = posx[ii];
      posy_bak[ii] = posy[ii];
      posz_bak[ii] = posz[ii];
      
      velx_bak[ii] = velx[ii];
      vely_bak[ii] = vely[ii];
      velz_bak[ii] = velz[ii];
    }
  }
  
  // Linearly interpolate between _fwd emphem and _bak ephem propagations
  // over the missing data depending on how close to each end of
  // gap.
  int this_idx=0;
  while( this_idx<n_ephem ) {
    if( !missing[this_idx] ) {
      posx_int[this_idx] = posx[this_idx];
      posy_int[this_idx] = posy[this_idx];
      posz_int[this_idx] = posz[this_idx];
      velx_int[this_idx] = velx[this_idx];
      vely_int[this_idx] = vely[this_idx];
      velz_int[this_idx] = velz[this_idx];
      this_idx++;
    } else {
      // Search for end of this gap in ephem (that_idx);
      // this_idx-1 is last good ephem point before gap
      // that_idx is first good ephem point after gap.
      int that_idx = this_idx+1;
      while( that_idx<n_ephem && missing[that_idx] ) that_idx++;
      
      int idx0 = this_idx-1;
      int idx1 = that_idx;
      
      // Fill in the gap.
      while( this_idx < that_idx ) {
        // linearly mix posx_fwd and posx_bak between idx0 and idx1 
        // depending on how close to each boundary.
        double bak_weight = double(this_idx-idx0)/double(idx1-idx0);
        double fwd_weight = 1 - bak_weight;
        
        posx_int[this_idx] = fwd_weight*posx_fwd[this_idx] + bak_weight*posx_bak[this_idx];
        posy_int[this_idx] = fwd_weight*posy_fwd[this_idx] + bak_weight*posy_bak[this_idx];
        posz_int[this_idx] = fwd_weight*posz_fwd[this_idx] + bak_weight*posz_bak[this_idx];
        velx_int[this_idx] = fwd_weight*velx_fwd[this_idx] + bak_weight*velx_bak[this_idx];
        vely_int[this_idx] = fwd_weight*vely_fwd[this_idx] + bak_weight*vely_bak[this_idx];
        velz_int[this_idx] = fwd_weight*velz_fwd[this_idx] + bak_weight*velz_bak[this_idx];
        this_idx++;
      }
    }
  }
  
  FILE* ofp = fopen(outfile,"w");
  for( int ii=0; ii<n_ephem; ++ii ) {
    fwrite(&time[ii],    sizeof(double),1,ofp);
    fwrite(&posx_int[ii],sizeof(double),1,ofp);
    fwrite(&posy_int[ii],sizeof(double),1,ofp);
    fwrite(&posz_int[ii],sizeof(double),1,ofp);
    fwrite(&velx_int[ii],sizeof(double),1,ofp);
    fwrite(&vely_int[ii],sizeof(double),1,ofp);
    fwrite(&velz_int[ii],sizeof(double),1,ofp);
  }
  fclose(ofp);
  exit(0);
}

