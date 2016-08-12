//==============================================================//
// Copyright (C) 2014, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <string>
#include "Constants.h"
#include "ETime.h"
#include "Wind.h"
#include "Array.h"
#include "List.h"
#include "BufferedList.h"
#include "Ephemeris.h"

class AngleInterval;
template class List<AngleInterval>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<WindVectorPlus>;
template class List<EarthPosition>;

#define RAD_TO_DEG(x) ((x)*180.0f/M_PI)
#define DEG_TO_RAD(x) ((x)*M_PI/180.0f)

typedef struct {
    float lambda_0;
    float inclination;
    float rev_period;
    int xt_steps;
    double at_res;
    double xt_res;
} latlon_config;

int lin_interp( double x1, double  x2,   // independant variable
                double f1, double  f2,   // dependant variable at x1, x2
                double x,  double* f ) { // interpolate to x, return value into f

// Sanity check for NANs and x in range [x1,x2]
  if( x1 != x1 || x2 != x2 || f1 != f1 || f2 != f2 || x != x || x < x1 || x > x2 ) {
    fprintf( stderr, "lin_interp: Inputs not in range!\n" );
    fprintf( stderr, "x1,x2,f1,f2,x: %f %f %f %f %f\n",x1,x2,f1,f2,x);
    return(0);
  }
  
  if( x1 != x2 )
    *f = f1 + (f2-f1)*(x-x1)/(x2-x1);
  else if( x1 == x2 && f1 == f2 )
    *f = f1;
  else {
    fprintf( stderr, "lin_interp: Expected f1 == f2 if x1 == x2!\n");
    return(0);
  }
  return(1);
}                  

int bilinear_interp( double x1,  double x2,
                     double y1,  double y2,
                     double f11, double f12, // dependant variable at x=x1, y=y1; x=x1, y=y2
                     double f21, double f22, // dependant variable at x=x2, y=y1; x=x2, y=y2
                     double x,   double y, double *f ) {
  
  double fx1; // dependant variable at x=x, y=y1
  double fx2; // dependant variable at x=x, y=y2
  
  if( !lin_interp( x1, x2, f11, f21, x, &fx1 ) ||
      !lin_interp( x1, x2, f12, f22, x, &fx2 ) ||
      !lin_interp( y1, y2, fx1, fx2, y, f    ) )
    return(0);
  else
    return(1);
}

void bin_to_latlon(int at_ind, int ct_ind,
        const latlon_config *config, float *lat, float *lon) {

    /* Utilizes e2, r1_earth from Constants.h */
    const static double P1 = 60*1440.0f;

    const static double P2 = config->rev_period;
    const double inc = DEG_TO_RAD(config->inclination);
    const int    r_n_xt_bins = config->xt_steps;
    const double at_res = config->at_res;
    const double xt_res = config->xt_res;

    const double lambda_0 = DEG_TO_RAD(config->lambda_0);

    const double r_n_at_bins = 1624.0 * 25.0 / at_res;
    const double atrack_bin_const = two_pi/r_n_at_bins;
    const double xtrack_bin_const = xt_res/r1_earth;

    double lambda, lambda_t, lambda_pp;
    double phi, phi_pp;
    double Q, U, V, V1, V2;

    double sin_phi_pp, sin_lambda_pp;
    double sin_phi_pp2, sin_lambda_pp2;
    double sini, cosi;

    sini = sinf(inc);
    cosi = cosf(inc);

    lambda_pp = (at_ind + 0.5)*atrack_bin_const - pi_over_two;
    phi_pp = -(ct_ind - (r_n_xt_bins/2 - 0.5))*xtrack_bin_const;

    sin_phi_pp = sinf(phi_pp);
    sin_phi_pp2 = sin_phi_pp*sin_phi_pp;
    sin_lambda_pp = sinf(lambda_pp);
    sin_lambda_pp2 = sin_lambda_pp*sin_lambda_pp;
    
    Q = e2*sini*sini/(1 - e2);
    U = e2*cosi*cosi/(1 - e2);

    V1 = (1 - sin_phi_pp2/(1 - e2))*cosi*sin_lambda_pp;
    V2 = (sini*sin_phi_pp*sqrtf((1 + Q*sin_lambda_pp2)*(1 - 
                    sin_phi_pp2) - U*sin_phi_pp2));

    V = (V1 - V2)/(1 - sin_phi_pp2*(1 + U));

    lambda_t = atan2f(V, cosf(lambda_pp));
    lambda = lambda_t - (P2/P1)*lambda_pp + lambda_0;

    lambda += (lambda < 0)       ?  two_pi : 
              (lambda >= two_pi) ? -two_pi :
                                    0.0f;
    phi = atanf((tanf(lambda_pp)*cosf(lambda_t) - 
                cosf(inc)*sinf(lambda_t))/((1 - e2)*
                sinf(inc)));

    *lon = lambda;
    *lat = phi;
}

const char usage_string[] = 
    "-e ecmwf_dir -o out_file -period period_seconds -t_start CODEB_string -long_asc long_asc_node -orbit_inc orbit_inclination [-n]";

int main(int argc, char* argv[]){
    const char* command = argv[0];
    char* ecmwf_dir = NULL;
    char* out_file = NULL;
    char* t_start_str = NULL;
    float wvc_size = 25.0;
    int n_cti = 76;
    double orbit_inc = -999;
    double orbit_period = 0;
    double long_asc_node = -999;
    int file_hour_interval = 6;
    int do_neutral = 0;
    int do_ncep = 0;
    int use_bigE = 0;
    int for_ascat = 0;

    int optind=1;
    while(optind<argc && (argv[optind][0]=='-')){
        std::string sw = argv[optind];
        if(sw == "-e") {
            ecmwf_dir = argv[++optind];

        } else if(sw == "-o"){
            out_file = argv[++optind];

        } else if(sw == "-t_start"){
            t_start_str = argv[++optind];

        } else if(sw == "-n_cti"){
            n_cti = atoi(argv[++optind]);

        } else if(sw == "-wvc_size"){
            wvc_size = atof(argv[++optind]);

        } else if(sw == "-period"){
            orbit_period = atof(argv[++optind]);

        } else if(sw == "-long_asc"){
            long_asc_node = atof(argv[++optind]);

        } else if(sw == "-inc"){
            orbit_inc = atof(argv[++optind]);

        } else if(sw == "-n"){
            do_neutral = 1;
            file_hour_interval = 3;

        } else if(sw == "-nwp1"){
            do_ncep = 1;

        } else if(sw == "-bigE"){
            use_bigE = 1;

        } else if(sw == "-ascat"){
            for_ascat = 1;

        } else {
            fprintf(stderr,"%s: Unknown option: %s\n", command, sw.c_str());
            exit(1);
        }
        ++optind;
    }

    // validate inputs
    if( !ecmwf_dir || !out_file || !t_start_str || orbit_period==0 || 
        long_asc_node==-999 || orbit_inc==-999) {
        fprintf(stderr, "%s: Usage: %s\n", command,&usage_string[0]);
        exit(1);
    }

    latlon_config orbit_config;
    orbit_config.lambda_0    = long_asc_node;
    orbit_config.inclination = orbit_inc;
    orbit_config.rev_period  = orbit_period;
    orbit_config.xt_steps    = n_cti;
    orbit_config.at_res      = wvc_size;
    orbit_config.xt_res      = wvc_size;
    
    // Keep swath grid consistent with historical grids
    // possible sizes:
    // 50km - 812, 25km -- 1624, 12.5km -- 3248, 10km -- 4060,
    // 8km - 5075, 7km -- 5800, 6.25km -- 6496, ...etc
    double f_nati = 40600 / wvc_size;
    if( f_nati != round(f_nati)) {
        fprintf(stderr, "wvc_size must divide 40600 exactly\n");
        exit(1);
    }
    int n_ati = (int)(40600/wvc_size);
    
    ETime etime_start, etime_curr;
    etime_start.FromCodeB(t_start_str);
    
    char ecmwf_file_1[1024],      ecmwf_file_2[1024];
    char ecmwf_file_1_last[1024], ecmwf_file_2_last[1024];

    WindField nwp_1, nwp_2;

    float ** output_lat = (float**)make_array(sizeof(float), 2, n_ati, n_cti);
    float ** output_lon = (float**)make_array(sizeof(float), 2, n_ati, n_cti);
    float ** output_spd = (float**)make_array(sizeof(float), 2, n_ati, n_cti);
    float ** output_dir = (float**)make_array(sizeof(float), 2, n_ati, n_cti);
    
    double t_start = etime_start.GetTime();

    for (int ati=0; ati<n_ati; ++ati){
        double t_curr = t_start + orbit_period*double(ati)/double(n_ati);
        etime_curr.SetTime( t_curr );
        char code_B_str_curr[CODE_B_TIME_LENGTH];

        etime_curr.ToCodeB( code_B_str_curr );

        int year_curr  = atoi( strtok( code_B_str_curr, "-" ) );
        int  doy_curr  = atoi( strtok( NULL,            "T" ) );
        int hour_curr  = atoi( strtok( NULL,            ":" ) );
        int  min_curr  = atoi( strtok( NULL,            ":" ) );
        float sec_curr = atof( strtok( NULL,            ":" ) );

        int year_1, year_2, doy_1, doy_2, hour_1, hour_2;

        year_1 = year_curr;
        doy_1  = doy_curr;
        hour_1 = hour_curr;
        year_2 = year_1;
        doy_2  = doy_1;
        hour_2 = hour_1;

        double t_1   = -1;
        double t_2   = -1;
        double t_now = -1;

        // t_now is hours of day
        t_now = double( hour_curr ) + ( double( min_curr ) 
              + double( sec_curr ) / 60.0 ) / 60.0;

        for(int file_hour_start = 0; file_hour_start<24;
            file_hour_start += file_hour_interval){
            int file_hour_stop = file_hour_start + file_hour_interval;
            if(hour_curr>= file_hour_start && hour_curr < file_hour_stop){
                hour_1 = file_hour_start;
                hour_2 = file_hour_stop;
                t_1 = (double)file_hour_start;
                t_2 = (double)file_hour_stop;

                if(file_hour_stop==24) {
                    hour_2 = 0;
                    int is_leap_year = 0;
                    if( year_curr % 400 == 0 )
                        is_leap_year = 1;
                    else if( year_curr % 100 == 0 )
                        is_leap_year = 0;
                    else if( year_curr % 4 == 0 )
                        is_leap_year = 1;
                    else
                        is_leap_year = 0;

                    if((doy_curr == 365 && is_leap_year == 0) || 
                        doy_curr == 366){
                        doy_2  = 1;
                        year_2 = year_1 + 1;
                    } else {
                        doy_2  = doy_1 + 1;
                        year_2 = year_1;
                    }
                }
            }
        }

        char nwp_char = '3';
        if(do_neutral) nwp_char = '4';
        if(do_ncep) nwp_char = '1';
        sprintf(ecmwf_file_1, "%s/SNWP%c%4.4d%3.3d%2.2d", ecmwf_dir, nwp_char,
            year_1, doy_1, hour_1);
        sprintf(ecmwf_file_2, "%s/SNWP%c%4.4d%3.3d%2.2d", ecmwf_dir, nwp_char,
            year_2, doy_2, hour_2);
        
        // Test if need to load new files.
        if( strcmp( ecmwf_file_1, ecmwf_file_1_last ) != 0 ||
            strcmp( ecmwf_file_2, ecmwf_file_2_last ) != 0 ) {
            printf("need to load new files!\n");
            // load files
            if( !nwp_1.ReadNCEP1( ecmwf_file_1, use_bigE ) ||
                !nwp_2.ReadNCEP1( ecmwf_file_2, use_bigE ) ) {
                fprintf( stderr, "Error loading nwp files!\n");
                exit(1);
            }
        }

        // Copy to _last string variables so we don't reload every time when 
        // files are the same.
        strcpy( ecmwf_file_1_last, ecmwf_file_1 );
        strcpy( ecmwf_file_2_last, ecmwf_file_2 );

        for (int cti = 0; cti < n_cti; ++cti ) {
            // Get lat lon for this wvc
            float wvc_lat, wvc_lon;

            int bin_ati = ati;
            if(for_ascat)
                bin_ati -= n_ati/2;

            bin_to_latlon( bin_ati, cti, &orbit_config, &wvc_lat, &wvc_lon );

            double     wvc_lon_deg = double( RAD_TO_DEG(wvc_lon) );
            double     wvc_lat_deg = double( RAD_TO_DEG(wvc_lat) );      

            if( wvc_lon_deg < 0    ) wvc_lon_deg += 360;
            if( wvc_lon_deg >= 360 ) wvc_lon_deg -= 360;


            double     lat[2];     // time index
            double     lon[2];     // time index
            float      u[2][2][2]; // time, lon, lat index order
            float      v[2][2][2]; // time, lon, lat index order
            double     s[2][2][2]; // time, lon, lat index order
            double     uu[2];      // bilinearly interpolated u at t1,t2
            double     vv[2];      // bilinearly interpolated v at t1,t2
            double     ss[2];      // bilinearly interpolated s at t1,t2
            double     uuu;        // trilinearly interpolated u
            double     vvv;        // trilinearly interpolated v
            double     sss;        // trilinearly interpolated speed


            // Corner of the "box" for bilinear interpolation
            // Assuming nwp is posted at integer lat lon (in degrees!)
            lat[0] = floor( wvc_lat_deg );
            lat[1] =  ceil( wvc_lat_deg );

            lon[0] = floor( wvc_lon_deg );
            lon[1] =  ceil( wvc_lon_deg );

            // Get windvector objects at the 4 corners
            for( int i_lon = 0; i_lon < 2; ++i_lon ) {
                for( int i_lat = 0; i_lat < 2; ++i_lat ) {

                    LonLat     tmp_ll;
                    WindVector tmp_wv[2];

                    tmp_ll.Set(
                        DEG_TO_RAD( lon[i_lon] ), DEG_TO_RAD( lat[i_lat] ) );

                    if( !nwp_1.NearestWindVector( tmp_ll, &tmp_wv[0] ) ||
                        !nwp_2.NearestWindVector( tmp_ll, &tmp_wv[1] ) ) {
                        fprintf( stderr, 
                            "Error getting nearest wind vectors @ corners!\n");
                        exit(1);
                    }

                    if( !tmp_wv[0].GetUV( &u[0][i_lon][i_lat], &v[0][i_lon][i_lat] ) ||
                        !tmp_wv[1].GetUV( &u[1][i_lon][i_lat], &v[1][i_lon][i_lat] ) ) {
                        fprintf( stderr, "Error getting u,v from WindVector!\n");
                        exit(1);
                    }

                    s[0][i_lon][i_lat] = tmp_wv[0].spd;
                    s[1][i_lon][i_lat] = tmp_wv[1].spd;
                }
            }

            // Interpolate!
            for ( int tt = 0; tt < 2; ++tt ) {
                // interpolate for u, v, and speed.
                if( !bilinear_interp( 
                        lon[0],      lon[1],      lat[0],      lat[1], 
                        u[tt][0][0], u[tt][0][1], u[tt][1][0], u[tt][1][1], 
                        wvc_lon_deg, wvc_lat_deg, &uu[tt] ) ||
                    !bilinear_interp( 
                        lon[0],      lon[1],      lat[0],      lat[1], 
                        v[tt][0][0], v[tt][0][1], v[tt][1][0], v[tt][1][1], 
                        wvc_lon_deg, wvc_lat_deg, &vv[tt] ) ||
                    !bilinear_interp( 
                        lon[0],      lon[1],      lat[0],      lat[1], 
                        s[tt][0][0], s[tt][0][1], s[tt][1][0], s[tt][1][1], 
                        wvc_lon_deg, wvc_lat_deg, &ss[tt] ) ) {
                    fprintf( 
                        stderr, "Error in spatial interpolation\n");
                    exit(1);
                }
            }
            // interpolate in time
            if( !lin_interp( t_1, t_2, uu[0], uu[1], t_now, &uuu ) ||
                !lin_interp( t_1, t_2, vv[0], vv[1], t_now, &vvv ) ||
                !lin_interp( t_1, t_2, ss[0], ss[1], t_now, &sss ) ) {
                fprintf( stderr, "Error in temporal interpolation\n" );
                exit(1);
            }

            // Final (u,v) components (as per RSD's code); RSD says this from 
            // M. Freilich. BWS says to continue to do it this way...
            double uuu_ = uuu * sss / sqrt( uuu*uuu + vvv*vvv );
            double vvv_ = vvv * sss / sqrt( uuu*uuu + vvv*vvv );

            // Use clockwise from North convention for wind direction 
            // (oceanographic)
            output_lat[ati][cti] = wvc_lat_deg;
            output_lon[ati][cti] = wvc_lon_deg;
            output_spd[ati][cti] = sqrt( uuu_ * uuu_ + vvv_ * vvv_ );
            output_dir[ati][cti] = RAD_TO_DEG( atan2( uuu_, vvv_ ) );
        }
    }

    // write out E2B file.
    FILE* ofp = fopen( out_file, "w" );
    write_array( ofp, &output_lat[0], sizeof(float), 2, n_ati, n_cti );
    write_array( ofp, &output_lon[0], sizeof(float), 2, n_ati, n_cti );
    write_array( ofp, &output_spd[0], sizeof(float), 2, n_ati, n_cti );
    write_array( ofp, &output_dir[0], sizeof(float), 2, n_ati, n_cti );
    fclose(ofp);

    // Free the arrays.
    free_array( output_lat, 2, n_ati, n_cti );
    free_array( output_lon, 2, n_ati, n_cti );
    free_array( output_spd, 2, n_ati, n_cti );
    free_array( output_dir, 2, n_ati, n_cti );
    return(0);
}
