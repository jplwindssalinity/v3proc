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
main( int      argc, 
      char*    argv[]) {
      
  const char*  command       = no_path(argv[0]);
  char*        in_ephem_file = NULL;  // gap-filled ephem file, with exactly overlapping records
  char*        in_quat_file  = NULL;  // gap-ridden quat file
  char*        outfile       = NULL;  // gap-filled output quat file.
  
  int optind = 1;
  while ( (optind < argc) && (argv[optind][0]=='-') ) {
    std::string sw = argv[optind];
    if( sw=="-ie" ) {
      in_ephem_file=argv[++optind];
    } else if ( sw=="-iq" ) {
      in_quat_file=argv[++optind];
    } else if ( sw=="-o" ) {
      outfile=argv[++optind];
    } else {
      fprintf(stderr,"%s: -ie ephem_input.dat -iq quat_input.dat -o quat_output.dat\n",command);
      exit(1);
    }
    ++optind;
  }
  
  if( !in_ephem_file || !in_quat_file || !outfile) {
    fprintf(stderr,"%s: -ie ephem_input.dat -iq quat_input.dat -o quat_output.dat\n",command);
    exit(1);
  }
  
  FILE* ifp_ephem = fopen(in_ephem_file,"r");
  FILE* ifp_quats = fopen(in_quat_file,"r");

  // Compute number of records in ephem file.
  fseek(ifp_ephem,0,SEEK_END);
  long int ephem_fsize = ftell(ifp_ephem);
  fseek(ifp_ephem,0,SEEK_SET);
  int n_ephem = ephem_fsize/(8*7);

  // Compute number of records in quat file.
  fseek(ifp_quats,0,SEEK_END);
  long int quats_fsize = ftell(ifp_quats);
  fseek(ifp_quats,0,SEEK_SET);
  int n_quats = quats_fsize/(8*5);
  
  // Require ephem file and quat file to have same number of records
  if( n_ephem != n_quats ) {
    fprintf(stderr,"Ephem file and Quat file sizes do not match: %d %d\n",n_ephem,n_quats);
    exit(1);
  }
  
  // Allocate storage for ephem and quat vectors
  std::vector< double > time(n_ephem), qtime(n_ephem);
  std::vector< double > posx(n_ephem);
  std::vector< double > posy(n_ephem);
  std::vector< double > posz(n_ephem);
  std::vector< double > velx(n_ephem);
  std::vector< double > vely(n_ephem);
  std::vector< double > velz(n_ephem);
  std::vector< double > qw(n_ephem), qw_fit(n_ephem);
  std::vector< double > qx(n_ephem), qx_fit(n_ephem);
  std::vector< double > qy(n_ephem), qy_fit(n_ephem);
  std::vector< double > qz(n_ephem), qz_fit(n_ephem);
  std::vector< int    > missing(n_ephem);
  
  // Read em in.
  for( int ii=0; ii< n_ephem; ++ii ) {
    fread( &time[ii], sizeof(double), 1, ifp_ephem );
    fread( &posx[ii], sizeof(double), 1, ifp_ephem );
    fread( &posy[ii], sizeof(double), 1, ifp_ephem );
    fread( &posz[ii], sizeof(double), 1, ifp_ephem );
    fread( &velx[ii], sizeof(double), 1, ifp_ephem );
    fread( &vely[ii], sizeof(double), 1, ifp_ephem );
    fread( &velz[ii], sizeof(double), 1, ifp_ephem );
    
    fread( &qtime[ii], sizeof(double), 1, ifp_quats );
    fread( &qw[ii],    sizeof(double), 1, ifp_quats );
    fread( &qx[ii],    sizeof(double), 1, ifp_quats );
    fread( &qy[ii],    sizeof(double), 1, ifp_quats );
    fread( &qz[ii],    sizeof(double), 1, ifp_quats );
    
    // Check for missing quaternion records
    missing[ii]=0;
    if( qw[ii]!=qw[ii] || qx[ii]!=qx[ii] || qy[ii]!=qy[ii] || qz[ii]!=qz[ii] ) missing[ii]=1;
    
    // Set output quaterions when no gaps
    qw_fit[ii] = qw[ii];
    qx_fit[ii] = qx[ii];
    qy_fit[ii] = qy[ii];
    qz_fit[ii] = qz[ii];
  }
  fclose(ifp_ephem);
  fclose(ifp_quats);
  
  // Estimate orbit period for harmonic fit
  std::vector< double > asc_node_times;
  
  for( int ii=0;ii<n_ephem-1;++ii)
    if( posz[ii+1]>0 && posz[ii]<=0 )  // check if ascending node.
      asc_node_times.push_back(time[ii]);
  
  double mean_period = 0;
  for( int ii=0;ii<asc_node_times.size()-1; ++ii) 
    mean_period += asc_node_times[ii+1]-asc_node_times[ii];
  
  mean_period /= double(asc_node_times.size()-1);
  fprintf(stdout,"Mean Period: %f %d\n",mean_period, asc_node_times.size());
  
  // Generate list of start indicies and times for gaps.
  std::vector< int >    idx_gap_start;
  std::vector< int >    idx_fit_start;
  std::vector< double > t_gap_start;
  
  int this_idx = 0;
  while(this_idx<n_ephem) {
    if(!missing[this_idx]) { 
      ++this_idx;
    } else {
      idx_gap_start.push_back(this_idx);
      t_gap_start.push_back(time[this_idx]);
      while( this_idx<n_ephem && missing[this_idx] ) ++this_idx;
    }
  }
  
  // Set harmonic fit times (azimuth) and variance (1 use it, -1 ignore it)
  std::vector< double > fitvar(n_ephem), fitazi(n_ephem);
  for( int ii=0; ii<n_ephem; ++ii ) {
    fitazi[ii] = 2*M_PI*time[ii]/mean_period;
    fitvar[ii] = 1.0;
    if( missing[ii] ) fitvar[ii] = -1;
  }
  
  // Set spectral fit start indicies
  for( int i_gap = 0; i_gap < idx_gap_start.size(); ++i_gap ) {
    
    // Use preceding 1 period of data for harmonic fit
    int idx_fit_start = idx_gap_start[i_gap];
    while( idx_fit_start>=0 && t_gap_start[i_gap] - time[idx_fit_start] < mean_period ) 
      --idx_fit_start;
    
    if( idx_fit_start<0 ) idx_fit_start=0;
    
    // Get pointers to data to be used for fitting.
    double* p_azi = &fitazi[idx_fit_start];
    double* p_var = &fitvar[idx_fit_start];
    double* p_www = &qw[idx_fit_start];
    double* p_xxx = &qx[idx_fit_start];
    double* p_yyy = &qy[idx_fit_start];
    double* p_zzz = &qz[idx_fit_start];
    
    // Other inputs to specfit function.
    int sample_count = idx_gap_start[i_gap] - idx_fit_start;
    int start_term   = 0;
    int end_term     = 3;
    
    // Outputs from specift function.
    double amp_www[end_term], pha_www[end_term];
    double amp_xxx[end_term], pha_xxx[end_term];
    double amp_yyy[end_term], pha_yyy[end_term];
    double amp_zzz[end_term], pha_zzz[end_term];
    
    // Do the harmonic fit
    specfit( p_azi, p_www, p_var, sample_count, start_term, end_term, &amp_www[0], &pha_www[0] );
    specfit( p_azi, p_xxx, p_var, sample_count, start_term, end_term, &amp_xxx[0], &pha_xxx[0] );
    specfit( p_azi, p_yyy, p_var, sample_count, start_term, end_term, &amp_yyy[0], &pha_yyy[0] );
    specfit( p_azi, p_zzz, p_var, sample_count, start_term, end_term, &amp_zzz[0], &pha_zzz[0] );
    
    this_idx = idx_gap_start[i_gap];
    while( this_idx<n_ephem && missing[this_idx] ) {
      double www_fit = 0; double xxx_fit = 0;
      double yyy_fit = 0; double zzz_fit = 0;
      
      // Compute fitted quaternions for missing records.
      for( int iterm=0;iterm<end_term;++iterm) {
        www_fit += amp_www[iterm]*cos(fitazi[this_idx]*double(iterm)+pha_www[iterm]);
        xxx_fit += amp_xxx[iterm]*cos(fitazi[this_idx]*double(iterm)+pha_xxx[iterm]);
        yyy_fit += amp_yyy[iterm]*cos(fitazi[this_idx]*double(iterm)+pha_yyy[iterm]);
        zzz_fit += amp_zzz[iterm]*cos(fitazi[this_idx]*double(iterm)+pha_zzz[iterm]);
      }
      
      // Normalize the quaterions
      double norm = sqrt(www_fit*www_fit+xxx_fit*xxx_fit+yyy_fit*yyy_fit+zzz_fit*zzz_fit);
      qw_fit[this_idx] = www_fit / norm;
      qx_fit[this_idx] = xxx_fit / norm;
      qy_fit[this_idx] = yyy_fit / norm;
      qz_fit[this_idx] = zzz_fit / norm;
      
      ++this_idx;
    }
  }
  
  // write out gap filled quats
  FILE* ofp = fopen(outfile,"w");
  for( int ii=0; ii<n_ephem; ++ii ) {
    fwrite(&time[ii],  sizeof(double),1,ofp);
    fwrite(&qw_fit[ii],sizeof(double),1,ofp);
    fwrite(&qx_fit[ii],sizeof(double),1,ofp);
    fwrite(&qy_fit[ii],sizeof(double),1,ofp);
    fwrite(&qz_fit[ii],sizeof(double),1,ofp);
  }
  fclose(ofp);
  exit(0);
}

