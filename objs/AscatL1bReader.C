
using namespace std;

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "AscatL1bReader.h"

///////////////////////////////////
// useful definitions
///////////////////////////////////

// record id

#define MPHR_ID   1
#define SPHR_ID   2
#define IPR_ID    3
#define GEADR_ID  4
#define GIADR_ID  5
#define VEADR_ID  6
#define VIADR_ID  7
#define MDR_ID    8
#define ASCAT_ID  2
#define SZO_ID    2
#define SZR_ID    1
#define SZF_ID    3

// record size

#define GRH_SIZE     20
#define MPHR_SIZE  3307
#define IPR_SIZE     27
#define SZO_SIZE   4018
#define SZR_SIZE   7818
#define SZF_SIZE  41624
#define SZF_SIZE_NEW  3684

// number of nodes

#define NODES_SZO   21
#define NODES_SZR   41
#define NODES_SZF  256
#define NODES_SZF_NEW  192

#define DINSTR_ID  13
#define DFMT_ID     1
#define DUMMY_SIZE 21

// fail procedure

#define FAIL { printf("*** %s %i\n",__FILE__,__LINE__); exit(1); }

void ymd2julian( int year, int month, int day, int &julian )
{

// Based on datetime modules in ASCAT processor.

  int iyear, imonth;
  
  iyear  = year;
  imonth = month;
  
  if( imonth > 12 ) 
  {
    iyear  = iyear + imonth/12;
    imonth = imonth%12;
  }
  else if ( imonth < 1 )
  {
    imonth = imonth - 1;
    iyear  = iyear - imonth/12 - 1;
    imonth = 12 - imonth%12;
  }
  
  if( imonth < 3 ) 
  {
    iyear  = iyear - 1;
    imonth = imonth + 12;
  }
  
  julian = day    + 1720994;
  julian = julian + 306001*(imonth + 1)/10000;
    
  if( iyear > 0 )
    julian = julian + (1461*iyear)/4;
  else
    julian = julian + (1461*iyear - 3)/4;
    
  if(iyear*10000 + imonth*100 + day > 15821014)
    julian  = julian + 2 - iyear/100 + iyear/400;
    
}

void julian2ymd( int julian, int &year, int &month, int &day )
{
// Based on datetime modules in ASCAT processor.

  int b,c,d,e;
  int alpha;

  if (julian < 2299160)
       b = julian + 1525;
  else
  {
    alpha = (4*julian - 7468861)/146097;
    b = julian + 1526 + alpha - alpha/4;
  }
  
  c   = (20*b - 2442)/7305;
  d   = 1461*c/4;
  e   = 10000*(b - d)/306001;
  day = int(b - d - 306001*e/10000);

  if (e < 14)
    month = int(e - 1);
  else
    month = int(e - 13);

  if (month > 2)
    year = c - 4716;
  else
    year = c - 4715;
}

///////////////////////////////////
// calc ascending flag from track
///////////////////////////////////

static int calc_asc( double track )
{
 if( track < 0.0 ) track += 360.0;
 if( track>90.0 && track<270.0 )
  return 0;
 return 1;
}

///////////////////////////////////
// extract integers from mdr
///////////////////////////////////

static void get_char( unsigned char *x, int pos, int *a )
{
 x += pos;
 *a = *x;
 if( *a >= 128 ) *a -= 256;
}

static void get_ushort( unsigned char *x, int pos, int *a )
{
 x += pos;
 *a = *x;
 x++;
 *a = *a*256 + (*x);
}

static void get_short( unsigned char *x, int pos, int *a )
{
 x += pos;
 *a = *x;
 if( *a >= 128 ) *a -= 256;
 x++;
 *a = *a*256 + (*x);
}

static void get_uint( unsigned char *x, int pos, int *a )
{
 x += pos;
 *a = *x;
 if( *a > 128 ) FAIL;
 x++;
 *a = *a*256 + (*x);
 x++;
 *a = *a*256 + (*x);
 x++;
 *a = *a*256 + (*x);
}

static void get_int( unsigned char *x, int pos, int *a )
{
 x += pos;
 *a = *x;
 if( *a > 128 ) *a -= 256;
 x++;
 *a = *a*256 + (*x);
 x++;
 *a = *a*256 + (*x);
 x++;
 *a = *a*256 + (*x);
}

static void get_byte( unsigned char *x, int pos, int *a )
{
 x += pos;
 *a = *x;
}

static void get_byte( unsigned char *x, int pos, int *a, int *b, int *c )
{
 x += pos;
 *a = *x;
 x++;
 *b = *x;
 x++;
 *c = *x;
}

///////////////////////////////////
// extract doubles from mdr
///////////////////////////////////

static void get_time( unsigned char *x, double *t, 
                      int &year, int &month, int &day, int &hour, int &minute, int &second)
{
int julian_start;
int julian_day;
int msec;
  
 get_ushort(x,20,&julian_day);
 get_uint(x,22,&msec);
 second = msec/1000;
 *t = julian_day + second/86400.0;
 
 ymd2julian(2000,1,1,julian_start);
 julian2ymd(julian_start+julian_day,year,month,day);
 
 
 
 hour   = second / 3600;
 minute = second / 60 - hour * 60;
 second = second - hour * 3600 - minute * 60;
}

static void get_time_new( unsigned char *x, double *t, 
                      int &year, int &month, int &day, int &hour, int &minute, int &second)
{
int julian_start;
int julian_day;
int msec;
  
 get_ushort(x,22,&julian_day);
 get_uint(x,24,&msec);
 second = msec/1000;
 *t = julian_day + second/86400.0;
 
 ymd2julian(2000,1,1,julian_start);
 julian2ymd(julian_start+julian_day,year,month,day);
 
 
 
 hour   = second / 3600;
 minute = second / 60 - hour * 60;
 second = second - hour * 3600 - minute * 60;
}
static void get_ushort( unsigned char *x, int pos, double sf, double *ans )
{
 int a;

 get_ushort(x,pos,&a);
 *ans = a*sf;
}

static void get_short( unsigned char *x, int pos, double sf, double *ans )
{
 int a;

 get_short(x,pos,&a);
 *ans = a*sf;
}

static void get_uint( unsigned char *x, int pos, double sf, double *ans )
{
 int a;

 get_uint(x,pos,&a);
 *ans = a*sf;
}

static void get_int( unsigned char *x, int pos, double sf, double *ans )
{
 int a;

 get_int(x,pos,&a);
 *ans = a*sf;
}

static void get_ushort( unsigned char *x, int pos, double sf, double *a, double *b, double *c )
{
 int ans;

 get_ushort(x,pos,&ans);
 *a = ans*sf;
 get_ushort(x,pos+2,&ans);
 *b = ans*sf;
 get_ushort(x,pos+4,&ans);
 *c = ans*sf;
}

static void get_short( unsigned char *x, int pos, double sf, double *a, double *b, double *c )
{
 int ans;

 get_short(x,pos,&ans);
 *a = ans*sf;
 get_short(x,pos+2,&ans);
 *b = ans*sf;
 get_short(x,pos+4,&ans);
 *c = ans*sf;
}

static void get_int( unsigned char *x, int pos, double sf, double *a, double *b, double *c )
{
 int ans;

 get_int(x,pos,&ans);
 *a = ans*sf;
 get_int(x,pos+4,&ans);
 *b = ans*sf;
 get_int(x,pos+8,&ans);
 *c = ans*sf;
}

///////////////////////////////////
// extract string data from mphr
///////////////////////////////////

static void get_str( unsigned char *x, int n, char *a )
{
 if( n <= 0 ) FAIL;
 while( n )
  {
   *a = (char)( *x );
   x++;
   a++;
   n--;
  }
 *a = '\0';
}

static int get_str( unsigned char *x, int pos, int n, string *ans )
{
 char s[16];

 if( n >= 16 ) FAIL;
 x += pos;
 get_str(x,n,s);
 *ans = s;
 return 0;
}

static int get_num( unsigned char *x, int pos, int *ans )
{
 char s[16];

 x += pos;
 get_str(x,6,s);
 if( sscanf(s,"%i",ans) != 1 ) return 1;
 if( *ans < 0 ) return 1;
 return 0;
}

///////////////////////////////////
// record handling
///////////////////////////////////

static int read_rec( FILE *fp, int size, char id, unsigned char *x )
{
 int n;

 if( fread(x,1,size,fp) != size ) FAIL;
 if( *x != id ) return 1;
 get_uint(x,4,&n);
 if( n != size ) return 1;
 return 0;
}

static int skip_rec( FILE *fp, char id )
{
 unsigned char x[GRH_SIZE];
 int n,size;

 if( fread(x,1,GRH_SIZE,fp) != GRH_SIZE ) FAIL;
 if( *x != id ) return 1;
 get_uint(x,4,&n);
 if( n < GRH_SIZE ) return 1;
 n -= GRH_SIZE;
 if( fseek(fp,n,SEEK_CUR) ) FAIL;
 return 0;
}

static int skip_nrec( FILE *fp, int n, char id )
{
 if( n < 0 ) FAIL;
 while( n )
  {
   if( skip_rec( fp, id ) ) return 1;
   n--;
  }
 return 0;
}

///////////////////////////////////
// misc methods
///////////////////////////////////

AscatFile::AscatFile()
{
 mdr = NULL;
 fp = NULL;
}

AscatFile::~AscatFile()
{
 if( mdr ) free(mdr);
 if( fp ) fclose(fp);
}

void AscatFile::close()
{
 if( fp ) { fclose(fp); fp = NULL; }
 if( mdr ) { free(mdr); mdr = NULL; }
}

///////////////////////////////////
// open file, read mphr and
// extract information
///////////////////////////////////

int AscatFile::open( const char* fname, int *nr0, int *nn0 )
{
 unsigned char x[MPHR_SIZE];
 string instr,prod,satid,dumstr;
 int num_mphr,num_sphr;
 int num_geadr,num_giadr;
 int num_veadr,num_viadr;
 int num_ipr,num_mdr;
 int rec_size,i;
 
 int proc_maj_version,proc_min_version;
 int orbitnr;
 
 // tidy old data
 if( mdr ) { free(mdr); mdr = NULL; }
 if( fp ) { fclose(fp); fp = NULL; }

 // open
 fp = fopen(fname,"r");
 if( fp == NULL ) return 1;

 // mphr
 if( read_rec(fp,MPHR_SIZE,MPHR_ID,x) ) return 1;
 if( get_str(x,552,4,&instr) ) return 1;
 if( get_str(x,625,3,&prod) ) return 1;
 if( get_str(x,696,3,&satid) ) return 1;
 
 if( get_num(x,960,&proc_maj_version) ) return 1;
 if( get_num(x,998,&proc_min_version) ) return 1;
 if( get_num(x,1408,&orbitnr) ) return 1;
 if( get_num(x,1036, &fmt_maj) ) return 1;
 if( get_num(x,1074, &fmt_min) ) return 1;
 
 
 if( get_str(x,1529,4,&dumstr) ) return 1;
 s_v_year = atoi( dumstr.c_str() );

 if( get_str(x,1533,2,&dumstr) ) return 1;
 s_v_month = atoi( dumstr.c_str() );

 if( get_str(x,1535,2,&dumstr) ) return 1;
 s_v_day = atoi( dumstr.c_str() );

 if( get_str(x,1537,2,&dumstr) ) return 1;
 s_v_hour = atoi( dumstr.c_str() );

 if( get_str(x,1539,2,&dumstr) ) return 1;
 s_v_minute = atoi( dumstr.c_str() );

 if( get_str(x,1541,2,&dumstr) ) return 1;
 s_v_second = atoi( dumstr.c_str() );
 
 
 int tmp_int;
 
 if( get_str(x,1580,12,&dumstr) ) return 1; 
 semi_major_axis   = double(atof(dumstr.c_str())) * 1e-3; // meters
 
 if( get_str(x,1624,12,&dumstr) ) return 1; 
 eccentricity      = double(atof(dumstr.c_str())) * 1e-6; // unitless
 
 if( get_str(x,1668,12,&dumstr) ) return 1; 
 inclination       = double(atof(dumstr.c_str())) * 1e-3; // deg
 
 if( get_str(x,1712,12,&dumstr) ) return 1; 
 perigee_argument  = double(atof(dumstr.c_str())) * 1e-3; // deg
 
 if( get_str(x,1756,12,&dumstr) ) return 1; 
 ra_asc_node       = double(atof(dumstr.c_str())) * 1e-3; // deg
 
 if( get_str(x,1800,12,&dumstr) ) return 1; 
 mean_anomoly      = double(atof(dumstr.c_str())) * 1e-3; // deg
 
 
 if( get_str(x,1844,12,&dumstr) ) return 1; 
 sc_pos_x_asc_node = double(atof(dumstr.c_str())) * 1e-3; // meters
 
 if( get_str(x,1888,12,&dumstr) ) return 1; 
 sc_pos_y_asc_node = double(atof(dumstr.c_str())) * 1e-3; // meters
 
 if( get_str(x,1932,12,&dumstr) ) return 1; 
 sc_pos_z_asc_node = double(atof(dumstr.c_str())) * 1e-3; // meters
 
 if( get_str(x,1976,12,&dumstr) ) return 1; 
 sc_vel_x_asc_node = double(atof(dumstr.c_str())) * 1e-3; // meters / sec
 
 if( get_str(x,2020,12,&dumstr) ) return 1; 
 sc_vel_y_asc_node = double(atof(dumstr.c_str())) * 1e-3; // meters / sec
 
 if( get_str(x,2064,12,&dumstr) ) return 1; 
 sc_vel_z_asc_node = double(atof(dumstr.c_str())) * 1e-3; // meters / sec
 
 int asc_node_time_int, julian_start;
 
 ymd2julian( s_v_year, s_v_month, s_v_day, asc_node_time_int );
 ymd2julian(2000,1,1,julian_start);
 
 
 asc_node_time = double(asc_node_time_int)  +
                 double(s_v_hour)/24        + 
                 double(s_v_minute)/(24*60) +
                 double(s_v_second)/(24*60*60) - double(julian_start);
 
 if( get_num(x,2714,&num_mphr) ) return 1;
 if( get_num(x,2753,&num_sphr) ) return 1;
 if( get_num(x,2792,&num_ipr) )  return 1;
 if( get_num(x,2831,&num_geadr) ) return 1;
 if( get_num(x,2870,&num_giadr) ) return 1;
 if( get_num(x,2909,&num_veadr) ) return 1;
 if( get_num(x,2948,&num_viadr) ) return 1;
 if( get_num(x,2987,&num_mdr) ) return 1;
  
 // check
 if( num_mphr != 1 ) FAIL;
 if( instr != "ASCA" ) FAIL;

 // skip records
 if( skip_nrec( fp, num_sphr,  SPHR_ID ) ) return 1;
 if( skip_nrec( fp, num_ipr,   IPR_ID ) ) return 1;
 if( skip_nrec( fp, num_geadr, GEADR_ID ) ) return 1;
 if( skip_nrec( fp, num_giadr, GIADR_ID ) ) return 1;
 if( skip_nrec( fp, num_veadr, VEADR_ID ) ) return 1;
 if( skip_nrec( fp, num_viadr, VIADR_ID ) ) return 1;
 
 // misc
 if( prod == "SZO" )
  {
   nn = NODES_SZO;
   size = SZO_SIZE;
   id = SZO_ID;
  }
 else if( prod == "SZR" )
  {
   nn = NODES_SZR;
   size = SZR_SIZE;
   id = SZR_ID;
  }
 else if( prod == "SZF" )
  {
   if(fmt_maj>=12) {
        nn = NODES_SZF_NEW;
        size = SZF_SIZE_NEW;
        id = SZF_ID;
    } else {
        nn = NODES_SZF;
        size = SZF_SIZE;
        id = SZF_ID;
    }
  }
 else
  return 1;

 if( satid == "M01" )
  sat = 1;
 else if( satid == "M02" )
  sat = 2;
 else if( satid == "M03" )
  sat = 2;
 else
  sat = 0;
 
 proc_maj = proc_maj_version;
 proc_min = proc_min_version;
 
 orbit = orbitnr;
 
 *nn0 = nn;
 *nr0 = num_mdr;
 mdr = (unsigned char*)malloc( sizeof(unsigned char)*size );
 if( mdr == NULL ) FAIL;
 return 0;
}

int AscatFile::read_mdr( int *isdummy )
{
 char instr;
 int n;

 *isdummy = 0;
 if( fread(mdr,1,GRH_SIZE,fp) != GRH_SIZE ) return 1;
 if( mdr[0] != MDR_ID ) return 1;
 get_uint(mdr,4,&n);
 if( n < GRH_SIZE ) return 1;
 instr = mdr[1];

 // dummy mdr
 if( instr == DINSTR_ID )
  {
   if( mdr[2] != DFMT_ID ) return 1;
   if( n != DUMMY_SIZE ) return 1;
   if( fread(mdr,1,1,fp) != 1 ) return 1;
   *isdummy = 1;
   return 0;
  }

 // mdr
 if( instr != ASCAT_ID ) return 1;
 if( mdr[2] != id ) return 1;
 if( n != size ) return 1;
 n = size - GRH_SIZE;
 if( fread(mdr+GRH_SIZE,1,n,fp) != n ) return 1;
 return 0;
}



void ASCAT_L1B_calibrate_SZF_node( AscatSZFNode *b, int i_dir )
{
  // Alex Fore -- 2/2/2010
  //
  // Inputs: 
  //  AscatSZFNode *b    -- pointer to structure containing SZF node data.
  //  int          i_dir -- +1 to apply calibration factors; -1 to unapply them.
  
  //  This function will modify b->s0 and apply or un-aply the calibration factors
  //  that I obtained from version 1.1 of the AWDP software.
  
  double software_id = double(b->proc_maj) * 100 + double(b->proc_min);
  
  double tot_cf[3][42];
  
  int cf_table_version;
  
  // Calibration tables obtained from AWDP/awdp/src/awdp_prepost.F90
  // from version 1.1 of AWDP on 2/2/2010.
  
  // Calibation tables have 1st index being [fore,mid,aft] beam and
  // 2nd index is the cross-track WVC #. (the wvc as in the ascat L2 SZ0 product)
  
  // Alex Fore made a fit of cross-track WVC # to incidence angle for each beam
  // based on L2 ASCAT SZ0 data.  This is how we will use these tables to 
  // calibrate the full-resolution SZF Sigma-0s. 
  // (inc_ang -> cell_index -> interpolated calibration factor )
  
  // note CTI in [0,20] is on left-hand swath, cti in [21,41] is
  // on the right-hand swath.  Must be careful when interpolating
  // to not interpolate across this boundary!
  
  // updated 7/12/2012 with V2.1 AWDP AGF with versions 5, 6
  
  cf_table_version = 1;
  if( software_id >= 502 ) cf_table_version = 2;
  if( software_id >= 503 ) cf_table_version = 3;
  if( software_id >= 603 ) cf_table_version = 4;
  if( software_id >= 703 ) cf_table_version = 5;
  if( software_id >= 704 ) cf_table_version = 6;
  
//  fprintf(stdout,"cf_table_version: %d\n",cf_table_version);
  
  
  // calibration factors are added to sigma-0 when sigma0 is in units of dB; 
  // thus a mutiplicative factor for sigma-0 in linear units.
  if( cf_table_version == 1 )
  {
	tot_cf[1-1][ 1-1] =  1.0089417780 ; tot_cf[2-1][ 1-1] =  0.5332721647 ; tot_cf[3-1][ 1-1] =  1.0314410251;
	tot_cf[1-1][ 2-1] =  0.9475136214 ; tot_cf[2-1][ 2-1] =  0.4499089445 ; tot_cf[3-1][ 2-1] =  0.9756664732;
	tot_cf[1-1][ 3-1] =  0.8768062343 ; tot_cf[2-1][ 3-1] =  0.3517250460 ; tot_cf[3-1][ 3-1] =  0.8534230622;
	tot_cf[1-1][ 4-1] =  0.7776493288 ; tot_cf[2-1][ 4-1] =  0.2686091522 ; tot_cf[3-1][ 4-1] =  0.7251015783;
	tot_cf[1-1][ 5-1] =  0.6660857702 ; tot_cf[2-1][ 5-1] =  0.1962292784 ; tot_cf[3-1][ 5-1] =  0.6043919028;
	tot_cf[1-1][ 6-1] =  0.5715672011 ; tot_cf[2-1][ 6-1] =  0.1532711147 ; tot_cf[3-1][ 6-1] =  0.4848813908;
	tot_cf[1-1][ 7-1] =  0.4982916965 ; tot_cf[2-1][ 7-1] =  0.1666519217 ; tot_cf[3-1][ 7-1] =  0.3796583694;
	tot_cf[1-1][ 8-1] =  0.4263741784 ; tot_cf[2-1][ 8-1] =  0.2278685241 ; tot_cf[3-1][ 8-1] =  0.3053788138;
	tot_cf[1-1][ 9-1] =  0.3501762261 ; tot_cf[2-1][ 9-1] =  0.3058385280 ; tot_cf[3-1][ 9-1] =  0.2795437437;
	tot_cf[1-1][10-1] =  0.2776630538 ; tot_cf[2-1][10-1] =  0.3108736848 ; tot_cf[3-1][10-1] =  0.2813018315;
	tot_cf[1-1][11-1] =  0.2330843018 ; tot_cf[2-1][11-1] =  0.2136852014 ; tot_cf[3-1][11-1] =  0.2672445657;
	tot_cf[1-1][12-1] =  0.2358116158 ; tot_cf[2-1][12-1] =  0.1323502466 ; tot_cf[3-1][12-1] =  0.2233028928;
	tot_cf[1-1][13-1] =  0.2549066622 ; tot_cf[2-1][13-1] =  0.1860119704 ; tot_cf[3-1][13-1] =  0.1518613778;
	tot_cf[1-1][14-1] =  0.2455446198 ; tot_cf[2-1][14-1] =  0.3754145831 ; tot_cf[3-1][14-1] =  0.1051379367;
	tot_cf[1-1][15-1] =  0.1745830534 ; tot_cf[2-1][15-1] =  0.4614974988 ; tot_cf[3-1][15-1] =  0.0797326277;
	tot_cf[1-1][16-1] =  0.0664446836 ; tot_cf[2-1][16-1] =  0.2751318615 ; tot_cf[3-1][16-1] =  0.0203876213;
	tot_cf[1-1][17-1] = -0.0107877216 ; tot_cf[2-1][17-1] =  0.0648618008 ; tot_cf[3-1][17-1] = -0.0617309288;
	tot_cf[1-1][18-1] = -0.0439479696 ; tot_cf[2-1][18-1] =  0.1363840561 ; tot_cf[3-1][18-1] = -0.0939864572;
	tot_cf[1-1][19-1] = -0.0438378849 ; tot_cf[2-1][19-1] =  0.3483383480 ; tot_cf[3-1][19-1] = -0.0043692938;
	tot_cf[1-1][20-1] = -0.0272704408 ; tot_cf[2-1][20-1] =  0.3768231543 ; tot_cf[3-1][20-1] =  0.1430275984;
	tot_cf[1-1][21-1] =  0.0098746061 ; tot_cf[2-1][21-1] =  0.3611906064 ; tot_cf[3-1][21-1] =  0.1619728700;
	tot_cf[1-1][22-1] =  0.0470110922 ; tot_cf[2-1][22-1] =  0.2321082090 ; tot_cf[3-1][22-1] = -0.0246550379;
	tot_cf[1-1][23-1] =  0.0286952617 ; tot_cf[2-1][23-1] =  0.1123263511 ; tot_cf[3-1][23-1] = -0.0478035376;
	tot_cf[1-1][24-1] =  0.0222508137 ; tot_cf[2-1][24-1] =  0.0217788093 ; tot_cf[3-1][24-1] = -0.0930045676;
	tot_cf[1-1][25-1] =  0.0188182132 ; tot_cf[2-1][25-1] =  0.0784221384 ; tot_cf[3-1][25-1] = -0.0955503879;
	tot_cf[1-1][26-1] = -0.0008819585 ; tot_cf[2-1][26-1] =  0.2035449905 ; tot_cf[3-1][26-1] = -0.0385889713;
	tot_cf[1-1][27-1] =  0.0013858901 ; tot_cf[2-1][27-1] =  0.2646498329 ; tot_cf[3-1][27-1] =  0.0254466750;
	tot_cf[1-1][28-1] =  0.0748075652 ; tot_cf[2-1][28-1] =  0.2224671173 ; tot_cf[3-1][28-1] =  0.0696140063;
	tot_cf[1-1][29-1] =  0.1576087292 ; tot_cf[2-1][29-1] =  0.2084987865 ; tot_cf[3-1][29-1] =  0.0991386510;
	tot_cf[1-1][30-1] =  0.2011542145 ; tot_cf[2-1][30-1] =  0.2254729902 ; tot_cf[3-1][30-1] =  0.1452397550;
	tot_cf[1-1][31-1] =  0.2139803500 ; tot_cf[2-1][31-1] =  0.2427484095 ; tot_cf[3-1][31-1] =  0.2147944732;
	tot_cf[1-1][32-1] =  0.2471861488 ; tot_cf[2-1][32-1] =  0.2611664846 ; tot_cf[3-1][32-1] =  0.2777386984;
	tot_cf[1-1][33-1] =  0.3079276886 ; tot_cf[2-1][33-1] =  0.2487325332 ; tot_cf[3-1][33-1] =  0.3160472676;
	tot_cf[1-1][34-1] =  0.3819976262 ; tot_cf[2-1][34-1] =  0.2194173375 ; tot_cf[3-1][34-1] =  0.3445001695;
	tot_cf[1-1][35-1] =  0.4561099111 ; tot_cf[2-1][35-1] =  0.1732209777 ; tot_cf[3-1][35-1] =  0.3979586595;
	tot_cf[1-1][36-1] =  0.5052982178 ; tot_cf[2-1][36-1] =  0.1142253168 ; tot_cf[3-1][36-1] =  0.4951924460;
	tot_cf[1-1][37-1] =  0.5390153958 ; tot_cf[2-1][37-1] =  0.0636983945 ; tot_cf[3-1][37-1] =  0.5988533159;
	tot_cf[1-1][38-1] =  0.6046131254 ; tot_cf[2-1][38-1] =  0.0837190631 ; tot_cf[3-1][38-1] =  0.6905200104;
	tot_cf[1-1][39-1] =  0.7040608091 ; tot_cf[2-1][39-1] =  0.1894505867 ; tot_cf[3-1][39-1] =  0.7584154867;
	tot_cf[1-1][40-1] =  0.7877729648 ; tot_cf[2-1][40-1] =  0.3167614922 ; tot_cf[3-1][40-1] =  0.8197632726;
	tot_cf[1-1][41-1] =  0.8611636346 ; tot_cf[2-1][41-1] =  0.4116086487 ; tot_cf[3-1][41-1] =  0.9120342139;
	tot_cf[1-1][42-1] =  0.9389469449 ; tot_cf[2-1][42-1] =  0.4620250818 ; tot_cf[3-1][42-1] =  1.0057685693;
  }  
  if( cf_table_version == 2 )
  {
	tot_cf[1-1][ 1-1] =  1.0089417780 ; tot_cf[2-1][ 1-1] =  0.5332721647 ; tot_cf[3-1][ 1-1] =  1.0314410251;
	tot_cf[1-1][ 2-1] =  0.9475136214 ; tot_cf[2-1][ 2-1] =  0.4499089445 ; tot_cf[3-1][ 2-1] =  0.9756664732;
	tot_cf[1-1][ 3-1] =  0.8768062343 ; tot_cf[2-1][ 3-1] =  0.3517250460 ; tot_cf[3-1][ 3-1] =  0.8534230622;
	tot_cf[1-1][ 4-1] =  0.7776493288 ; tot_cf[2-1][ 4-1] =  0.2686091522 ; tot_cf[3-1][ 4-1] =  0.7251015783;
	tot_cf[1-1][ 5-1] =  0.6660857702 ; tot_cf[2-1][ 5-1] =  0.1962292784 ; tot_cf[3-1][ 5-1] =  0.6043919028;
	tot_cf[1-1][ 6-1] =  0.5715672011 ; tot_cf[2-1][ 6-1] =  0.1532711147 ; tot_cf[3-1][ 6-1] =  0.4848813908;
	tot_cf[1-1][ 7-1] =  0.4982916965 ; tot_cf[2-1][ 7-1] =  0.1666519217 ; tot_cf[3-1][ 7-1] =  0.3796583694;
	tot_cf[1-1][ 8-1] =  0.4263741784 ; tot_cf[2-1][ 8-1] =  0.2278685241 ; tot_cf[3-1][ 8-1] =  0.3053788138;
	tot_cf[1-1][ 9-1] =  0.3501762261 ; tot_cf[2-1][ 9-1] =  0.3058385280 ; tot_cf[3-1][ 9-1] =  0.2795437437;
	tot_cf[1-1][10-1] =  0.2776630538 ; tot_cf[2-1][10-1] =  0.3108736848 ; tot_cf[3-1][10-1] =  0.2813018315;
	tot_cf[1-1][11-1] =  0.2330843018 ; tot_cf[2-1][11-1] =  0.2136852014 ; tot_cf[3-1][11-1] =  0.2672445657;
	tot_cf[1-1][12-1] =  0.2358116158 ; tot_cf[2-1][12-1] =  0.1323502466 ; tot_cf[3-1][12-1] =  0.2233028928;
	tot_cf[1-1][13-1] =  0.2549066622 ; tot_cf[2-1][13-1] =  0.1860119704 ; tot_cf[3-1][13-1] =  0.1518613778;
	tot_cf[1-1][14-1] =  0.2455446198 ; tot_cf[2-1][14-1] =  0.3754145831 ; tot_cf[3-1][14-1] =  0.1051379367;
	tot_cf[1-1][15-1] =  0.1745830534 ; tot_cf[2-1][15-1] =  0.4614974988 ; tot_cf[3-1][15-1] =  0.0797326277;
	tot_cf[1-1][16-1] =  0.0664446836 ; tot_cf[2-1][16-1] =  0.2751318615 ; tot_cf[3-1][16-1] =  0.0203876213;
	tot_cf[1-1][17-1] = -0.0107877216 ; tot_cf[2-1][17-1] =  0.0648618008 ; tot_cf[3-1][17-1] = -0.0617309288;
	tot_cf[1-1][18-1] = -0.0439479696 ; tot_cf[2-1][18-1] =  0.1363840561 ; tot_cf[3-1][18-1] = -0.0939864572;
	tot_cf[1-1][19-1] = -0.0438378849 ; tot_cf[2-1][19-1] =  0.3483383480 ; tot_cf[3-1][19-1] = -0.0043692938;
	tot_cf[1-1][20-1] = -0.0272704408 ; tot_cf[2-1][20-1] =  0.3768231543 ; tot_cf[3-1][20-1] =  0.1430275984;
	tot_cf[1-1][21-1] =  0.0098746061 ; tot_cf[2-1][21-1] =  0.3611906064 ; tot_cf[3-1][21-1] =  0.1619728700;
	tot_cf[1-1][22-1] =  0.0470110922 ; tot_cf[2-1][22-1] =  0.2321082090 ; tot_cf[3-1][22-1] = -0.0246550379;
	tot_cf[1-1][23-1] =  0.0286952617 ; tot_cf[2-1][23-1] =  0.1123263511 ; tot_cf[3-1][23-1] = -0.0478035376;
	tot_cf[1-1][24-1] =  0.0222508137 ; tot_cf[2-1][24-1] =  0.0217788093 ; tot_cf[3-1][24-1] = -0.0930045676;
	tot_cf[1-1][25-1] =  0.0188182132 ; tot_cf[2-1][25-1] =  0.0784221384 ; tot_cf[3-1][25-1] = -0.0955503879;
	tot_cf[1-1][26-1] = -0.0008819585 ; tot_cf[2-1][26-1] =  0.2035449905 ; tot_cf[3-1][26-1] = -0.0385889713;
	tot_cf[1-1][27-1] =  0.0013858901 ; tot_cf[2-1][27-1] =  0.2646498329 ; tot_cf[3-1][27-1] =  0.0254466750;
	tot_cf[1-1][28-1] =  0.0748075652 ; tot_cf[2-1][28-1] =  0.2224671173 ; tot_cf[3-1][28-1] =  0.0696140063;
	tot_cf[1-1][29-1] =  0.1576087292 ; tot_cf[2-1][29-1] =  0.2084987865 ; tot_cf[3-1][29-1] =  0.0991386510;
	tot_cf[1-1][30-1] =  0.2011542145 ; tot_cf[2-1][30-1] =  0.2254729902 ; tot_cf[3-1][30-1] =  0.1452397550;
	tot_cf[1-1][31-1] =  0.2139803500 ; tot_cf[2-1][31-1] =  0.2427484095 ; tot_cf[3-1][31-1] =  0.2147944732;
	tot_cf[1-1][32-1] =  0.2471861488 ; tot_cf[2-1][32-1] =  0.2611664846 ; tot_cf[3-1][32-1] =  0.2777386984;
	tot_cf[1-1][33-1] =  0.3079276886 ; tot_cf[2-1][33-1] =  0.2487325332 ; tot_cf[3-1][33-1] =  0.3160472676;
	tot_cf[1-1][34-1] =  0.3819976262 ; tot_cf[2-1][34-1] =  0.2194173375 ; tot_cf[3-1][34-1] =  0.3445001695;
	tot_cf[1-1][35-1] =  0.4561099111 ; tot_cf[2-1][35-1] =  0.1732209777 ; tot_cf[3-1][35-1] =  0.3979586595;
	tot_cf[1-1][36-1] =  0.5052982178 ; tot_cf[2-1][36-1] =  0.1142253168 ; tot_cf[3-1][36-1] =  0.4951924460;
	tot_cf[1-1][37-1] =  0.5390153958 ; tot_cf[2-1][37-1] =  0.0636983945 ; tot_cf[3-1][37-1] =  0.5988533159;
	tot_cf[1-1][38-1] =  0.6046131254 ; tot_cf[2-1][38-1] =  0.0837190631 ; tot_cf[3-1][38-1] =  0.6905200104;
	tot_cf[1-1][39-1] =  0.7040608091 ; tot_cf[2-1][39-1] =  0.1894505867 ; tot_cf[3-1][39-1] =  0.7584154867;
	tot_cf[1-1][40-1] =  0.7877729648 ; tot_cf[2-1][40-1] =  0.3167614922 ; tot_cf[3-1][40-1] =  0.8197632726;
	tot_cf[1-1][41-1] =  0.8611636346 ; tot_cf[2-1][41-1] =  0.4116086487 ; tot_cf[3-1][41-1] =  0.9120342139;
	tot_cf[1-1][42-1] =  0.9389469449 ; tot_cf[2-1][42-1] =  0.4620250818 ; tot_cf[3-1][42-1] =  1.0057685693;
  }
  if( cf_table_version == 3 )
  {
	tot_cf[1-1][ 1-1] =  0.7374987090 ; tot_cf[2-1][ 1-1] = -0.0216406409 ; tot_cf[3-1][ 1-1] =  0.6584923488;
	tot_cf[1-1][ 2-1] =  0.6381797307 ; tot_cf[2-1][ 2-1] = -0.0962878739 ; tot_cf[3-1][ 2-1] =  0.5125377872;
	tot_cf[1-1][ 3-1] =  0.5322854270 ; tot_cf[2-1][ 3-1] = -0.1616879064 ; tot_cf[3-1][ 3-1] =  0.3740076753;
	tot_cf[1-1][ 4-1] =  0.4085276879 ; tot_cf[2-1][ 4-1] = -0.1870543202 ; tot_cf[3-1][ 4-1] =  0.2820721746;
	tot_cf[1-1][ 5-1] =  0.2841235603 ; tot_cf[2-1][ 5-1] = -0.2073829180 ; tot_cf[3-1][ 5-1] =  0.2172183694;
	tot_cf[1-1][ 6-1] =  0.1928264434 ; tot_cf[2-1][ 6-1] = -0.2268070520 ; tot_cf[3-1][ 6-1] =  0.1499487357;
	tot_cf[1-1][ 7-1] =  0.1361925258 ; tot_cf[2-1][ 7-1] = -0.2214531131 ; tot_cf[3-1][ 7-1] =  0.0766269249;
	tot_cf[1-1][ 8-1] =  0.0866921358 ; tot_cf[2-1][ 8-1] = -0.1875269146 ; tot_cf[3-1][ 8-1] =  0.0061811400;
	tot_cf[1-1][ 9-1] =  0.0259598007 ; tot_cf[2-1][ 9-1] = -0.1357828948 ; tot_cf[3-1][ 9-1] = -0.0445374388;
	tot_cf[1-1][10-1] = -0.0484388751 ; tot_cf[2-1][10-1] = -0.1377665425 ; tot_cf[3-1][10-1] = -0.0892334826;
	tot_cf[1-1][11-1] = -0.1166049732 ; tot_cf[2-1][11-1] = -0.2184717488 ; tot_cf[3-1][11-1] = -0.1585145710;
	tot_cf[1-1][12-1] = -0.1526782862 ; tot_cf[2-1][12-1] = -0.2742063001 ; tot_cf[3-1][12-1] = -0.2473538002;
	tot_cf[1-1][13-1] = -0.1762184720 ; tot_cf[2-1][13-1] = -0.2090151008 ; tot_cf[3-1][13-1] = -0.3374804474;
	tot_cf[1-1][14-1] = -0.2125862465 ; tot_cf[2-1][14-1] = -0.0370130807 ; tot_cf[3-1][14-1] = -0.3719382063;
	tot_cf[1-1][15-1] = -0.2682619037 ; tot_cf[2-1][15-1] =  0.0068403018 ; tot_cf[3-1][15-1] = -0.3548357476;
	tot_cf[1-1][16-1] = -0.3109169716 ; tot_cf[2-1][16-1] = -0.2207526166 ; tot_cf[3-1][16-1] = -0.3530475184;
	tot_cf[1-1][17-1] = -0.2978170774 ; tot_cf[2-1][17-1] = -0.4450819229 ; tot_cf[3-1][17-1] = -0.3827720956;
	tot_cf[1-1][18-1] = -0.2563748556 ; tot_cf[2-1][18-1] = -0.3648237724 ; tot_cf[3-1][18-1] = -0.3960043248;
	tot_cf[1-1][19-1] = -0.2420537569 ; tot_cf[2-1][19-1] = -0.1665284809 ; tot_cf[3-1][19-1] = -0.3341037682;
	tot_cf[1-1][20-1] = -0.2647813753 ; tot_cf[2-1][20-1] = -0.2308983532 ; tot_cf[3-1][20-1] = -0.2499970846;
	tot_cf[1-1][21-1] = -0.2568983198 ; tot_cf[2-1][21-1] = -0.4033463108 ; tot_cf[3-1][21-1] = -0.2860080703;
	tot_cf[1-1][22-1] = -0.2878482194 ; tot_cf[2-1][22-1] = -0.4279722835 ; tot_cf[3-1][22-1] = -0.2653559832;
	tot_cf[1-1][23-1] = -0.2503303228 ; tot_cf[2-1][23-1] = -0.2925348905 ; tot_cf[3-1][23-1] = -0.2042521105;
	tot_cf[1-1][24-1] = -0.2165957864 ; tot_cf[2-1][24-1] = -0.2462915309 ; tot_cf[3-1][24-1] = -0.2070520726;
	tot_cf[1-1][25-1] = -0.2189973755 ; tot_cf[2-1][25-1] = -0.1906883028 ; tot_cf[3-1][25-1] = -0.2297872452;
	tot_cf[1-1][26-1] = -0.2661085909 ; tot_cf[2-1][26-1] = -0.1282803255 ; tot_cf[3-1][26-1] = -0.2434935425;
	tot_cf[1-1][27-1] = -0.2995054199 ; tot_cf[2-1][27-1] = -0.1162335926 ; tot_cf[3-1][27-1] = -0.2677745603;
	tot_cf[1-1][28-1] = -0.2578588021 ; tot_cf[2-1][28-1] = -0.1712718678 ; tot_cf[3-1][28-1] = -0.3012311625;
	tot_cf[1-1][29-1] = -0.1976968531 ; tot_cf[2-1][29-1] = -0.1800909831 ; tot_cf[3-1][29-1] = -0.3205878698;
	tot_cf[1-1][30-1] = -0.1684501346 ; tot_cf[2-1][30-1] = -0.1671334470 ; tot_cf[3-1][30-1] = -0.2963082646;
	tot_cf[1-1][31-1] = -0.1648755639 ; tot_cf[2-1][31-1] = -0.1738044322 ; tot_cf[3-1][31-1] = -0.2289846675;
	tot_cf[1-1][32-1] = -0.1350217217 ; tot_cf[2-1][32-1] = -0.1852121339 ; tot_cf[3-1][32-1] = -0.1548752108;
	tot_cf[1-1][33-1] = -0.0753598604 ; tot_cf[2-1][33-1] = -0.2045653501 ; tot_cf[3-1][33-1] = -0.1022406355;
	tot_cf[1-1][34-1] = -0.0019710847 ; tot_cf[2-1][34-1] = -0.1995118817 ; tot_cf[3-1][34-1] = -0.0670431402;
	tot_cf[1-1][35-1] =  0.0721611976 ; tot_cf[2-1][35-1] = -0.1775201107 ; tot_cf[3-1][35-1] = -0.0139864749;
	tot_cf[1-1][36-1] =  0.1257131901 ; tot_cf[2-1][36-1] = -0.1642448716 ; tot_cf[3-1][36-1] =  0.0790178554;
	tot_cf[1-1][37-1] =  0.1706623568 ; tot_cf[2-1][37-1] = -0.1745077834 ; tot_cf[3-1][37-1] =  0.1809427698;
	tot_cf[1-1][38-1] =  0.2553033531 ; tot_cf[2-1][38-1] = -0.1734632668 ; tot_cf[3-1][38-1] =  0.2749518375;
	tot_cf[1-1][39-1] =  0.3774592740 ; tot_cf[2-1][39-1] = -0.1459711543 ; tot_cf[3-1][39-1] =  0.3518159949;
	tot_cf[1-1][40-1] =  0.4844655209 ; tot_cf[2-1][40-1] = -0.1212304845 ; tot_cf[3-1][40-1] =  0.4272457953;
	tot_cf[1-1][41-1] =  0.5753005629 ; tot_cf[2-1][41-1] = -0.0874898310 ; tot_cf[3-1][41-1] =  0.5413441066;
	tot_cf[1-1][42-1] =  0.6670098726 ; tot_cf[2-1][42-1] = -0.0123397949 ; tot_cf[3-1][42-1] =  0.6725124438;
  }
  if( cf_table_version == 4 )
  {
	tot_cf[1-1][ 1-1] =  0.7527500000 ; tot_cf[2-1][ 1-1] = -0.0230044167 ; tot_cf[3-1][ 1-1] =  0.6575827083;
	tot_cf[1-1][ 2-1] =  0.6440216667 ; tot_cf[2-1][ 2-1] = -0.0856651667 ; tot_cf[3-1][ 2-1] =  0.5837577083;
	tot_cf[1-1][ 3-1] =  0.5462115000 ; tot_cf[2-1][ 3-1] = -0.1362626250 ; tot_cf[3-1][ 3-1] =  0.4732178333;
	tot_cf[1-1][ 4-1] =  0.4460542917 ; tot_cf[2-1][ 4-1] = -0.1545033333 ; tot_cf[3-1][ 4-1] =  0.3561588333;
	tot_cf[1-1][ 5-1] =  0.3468600417 ; tot_cf[2-1][ 5-1] = -0.1727874583 ; tot_cf[3-1][ 5-1] =  0.2408824583;
	tot_cf[1-1][ 6-1] =  0.2623807500 ; tot_cf[2-1][ 6-1] = -0.1833632083 ; tot_cf[3-1][ 6-1] =  0.1387078333;
	tot_cf[1-1][ 7-1] =  0.1855206667 ; tot_cf[2-1][ 7-1] = -0.1705478333 ; tot_cf[3-1][ 7-1] =  0.0621917500;
	tot_cf[1-1][ 8-1] =  0.1067061667 ; tot_cf[2-1][ 8-1] = -0.1605198750 ; tot_cf[3-1][ 8-1] = -0.0008821250;
	tot_cf[1-1][ 9-1] =  0.0372294167 ; tot_cf[2-1][ 9-1] = -0.1622260000 ; tot_cf[3-1][ 9-1] = -0.0595680417;
	tot_cf[1-1][10-1] = -0.0224833750 ; tot_cf[2-1][10-1] = -0.1785357917 ; tot_cf[3-1][10-1] = -0.1173710417;
	tot_cf[1-1][11-1] = -0.0873646667 ; tot_cf[2-1][11-1] = -0.1827957500 ; tot_cf[3-1][11-1] = -0.1672893750;
	tot_cf[1-1][12-1] = -0.1522796667 ; tot_cf[2-1][12-1] = -0.1554819167 ; tot_cf[3-1][12-1] = -0.1999095833;
	tot_cf[1-1][13-1] = -0.1952360417 ; tot_cf[2-1][13-1] = -0.1478095833 ; tot_cf[3-1][13-1] = -0.2514313750;
	tot_cf[1-1][14-1] = -0.1911560000 ; tot_cf[2-1][14-1] = -0.1502096250 ; tot_cf[3-1][14-1] = -0.3139609583;
	tot_cf[1-1][15-1] = -0.1803797500 ; tot_cf[2-1][15-1] = -0.1463843750 ; tot_cf[3-1][15-1] = -0.3642462500;
	tot_cf[1-1][16-1] = -0.2075813750 ; tot_cf[2-1][16-1] = -0.1563540000 ; tot_cf[3-1][16-1] = -0.3869849167;
	tot_cf[1-1][17-1] = -0.2326694167 ; tot_cf[2-1][17-1] = -0.1798955833 ; tot_cf[3-1][17-1] = -0.3678594167;
	tot_cf[1-1][18-1] = -0.2490312500 ; tot_cf[2-1][18-1] = -0.1766313750 ; tot_cf[3-1][18-1] = -0.3485277500;
	tot_cf[1-1][19-1] = -0.2861305833 ; tot_cf[2-1][19-1] = -0.2116778333 ; tot_cf[3-1][19-1] = -0.3481561667;
	tot_cf[1-1][20-1] = -0.2864783750 ; tot_cf[2-1][20-1] = -0.3582055000 ; tot_cf[3-1][20-1] = -0.3070170000;
	tot_cf[1-1][21-1] = -0.1848961250 ; tot_cf[2-1][21-1] = -0.4235212500 ; tot_cf[3-1][21-1] = -0.2763151667;
	tot_cf[1-1][22-1] = -0.2283909583 ; tot_cf[2-1][22-1] = -0.4889305833 ; tot_cf[3-1][22-1] = -0.2305319583;
	tot_cf[1-1][23-1] = -0.2103072083 ; tot_cf[2-1][23-1] = -0.3209973750 ; tot_cf[3-1][23-1] = -0.2045955833;
	tot_cf[1-1][24-1] = -0.2329126250 ; tot_cf[2-1][24-1] = -0.1640147917 ; tot_cf[3-1][24-1] = -0.2306804583;
	tot_cf[1-1][25-1] = -0.1903672083 ; tot_cf[2-1][25-1] = -0.1230767500 ; tot_cf[3-1][25-1] = -0.2036381250;
	tot_cf[1-1][26-1] = -0.2290035000 ; tot_cf[2-1][26-1] = -0.1713146667 ; tot_cf[3-1][26-1] = -0.2115137917;
	tot_cf[1-1][27-1] = -0.2694079167 ; tot_cf[2-1][27-1] = -0.1761405000 ; tot_cf[3-1][27-1] = -0.2759290000;
	tot_cf[1-1][28-1] = -0.2183376667 ; tot_cf[2-1][28-1] = -0.1551655417 ; tot_cf[3-1][28-1] = -0.2932271667;
	tot_cf[1-1][29-1] = -0.2039872917 ; tot_cf[2-1][29-1] = -0.1313691667 ; tot_cf[3-1][29-1] = -0.2675770000;
	tot_cf[1-1][30-1] = -0.2255609583 ; tot_cf[2-1][30-1] = -0.1560321667 ; tot_cf[3-1][30-1] = -0.2561065000;
	tot_cf[1-1][31-1] = -0.1811293333 ; tot_cf[2-1][31-1] = -0.1915475000 ; tot_cf[3-1][31-1] = -0.2489320417;
	tot_cf[1-1][32-1] = -0.0743322500 ; tot_cf[2-1][32-1] = -0.1924822083 ; tot_cf[3-1][32-1] = -0.2089870417;
	tot_cf[1-1][33-1] = -0.0104357917 ; tot_cf[2-1][33-1] = -0.2032437500 ; tot_cf[3-1][33-1] = -0.1352895000;
	tot_cf[1-1][34-1] =  0.0060968333 ; tot_cf[2-1][34-1] = -0.2120782917 ; tot_cf[3-1][34-1] = -0.0557159583;
	tot_cf[1-1][35-1] =  0.0516618750 ; tot_cf[2-1][35-1] = -0.1961577083 ; tot_cf[3-1][35-1] =  0.0268811667;
	tot_cf[1-1][36-1] =  0.1354945833 ; tot_cf[2-1][36-1] = -0.1636799167 ; tot_cf[3-1][36-1] =  0.1256538333;
	tot_cf[1-1][37-1] =  0.2198451667 ; tot_cf[2-1][37-1] = -0.1578588333 ; tot_cf[3-1][37-1] =  0.2183897083;
	tot_cf[1-1][38-1] =  0.3046496250 ; tot_cf[2-1][38-1] = -0.1692336667 ; tot_cf[3-1][38-1] =  0.2984691667;
	tot_cf[1-1][39-1] =  0.3939666667 ; tot_cf[2-1][39-1] = -0.1613200833 ; tot_cf[3-1][39-1] =  0.3622061667;
	tot_cf[1-1][40-1] =  0.4751345833 ; tot_cf[2-1][40-1] = -0.1257707500 ; tot_cf[3-1][40-1] =  0.4292358333;
	tot_cf[1-1][41-1] =  0.5719531667 ; tot_cf[2-1][41-1] = -0.0568519167 ; tot_cf[3-1][41-1] =  0.5438968750;
	tot_cf[1-1][42-1] =  0.6915420417 ; tot_cf[2-1][42-1] =  0.0304964167 ; tot_cf[3-1][42-1] =  0.6858306667;

  }
  if( cf_table_version == 5 )
  {
	tot_cf[1-1][ 1-1] =  0.7527500000 ; tot_cf[2-1][ 1-1] = -0.1480044167 ; tot_cf[3-1][ 1-1] =  0.6575827083;
	tot_cf[1-1][ 2-1] =  0.6440216667 ; tot_cf[2-1][ 2-1] = -0.2106651667 ; tot_cf[3-1][ 2-1] =  0.5837577083;
	tot_cf[1-1][ 3-1] =  0.5462115000 ; tot_cf[2-1][ 3-1] = -0.2612626250 ; tot_cf[3-1][ 3-1] =  0.4732178333;
	tot_cf[1-1][ 4-1] =  0.4460542917 ; tot_cf[2-1][ 4-1] = -0.2795033333 ; tot_cf[3-1][ 4-1] =  0.3561588333;
	tot_cf[1-1][ 5-1] =  0.3468600417 ; tot_cf[2-1][ 5-1] = -0.2977874583 ; tot_cf[3-1][ 5-1] =  0.2408824583;
	tot_cf[1-1][ 6-1] =  0.2623807500 ; tot_cf[2-1][ 6-1] = -0.3083632083 ; tot_cf[3-1][ 6-1] =  0.1387078333;
	tot_cf[1-1][ 7-1] =  0.1855206667 ; tot_cf[2-1][ 7-1] = -0.2955478333 ; tot_cf[3-1][ 7-1] =  0.0621917500;
	tot_cf[1-1][ 8-1] =  0.1067061667 ; tot_cf[2-1][ 8-1] = -0.2855198750 ; tot_cf[3-1][ 8-1] = -0.0008821250;
	tot_cf[1-1][ 9-1] =  0.0372294167 ; tot_cf[2-1][ 9-1] = -0.2872260000 ; tot_cf[3-1][ 9-1] = -0.0595680417;
	tot_cf[1-1][10-1] = -0.0224833750 ; tot_cf[2-1][10-1] = -0.3035357917 ; tot_cf[3-1][10-1] = -0.1173710417;
	tot_cf[1-1][11-1] = -0.0873646667 ; tot_cf[2-1][11-1] = -0.3077957500 ; tot_cf[3-1][11-1] = -0.1672893750;
	tot_cf[1-1][12-1] = -0.1522796667 ; tot_cf[2-1][12-1] = -0.2804819167 ; tot_cf[3-1][12-1] = -0.1999095833;
	tot_cf[1-1][13-1] = -0.1952360417 ; tot_cf[2-1][13-1] = -0.2728095833 ; tot_cf[3-1][13-1] = -0.2514313750;
	tot_cf[1-1][14-1] = -0.1911560000 ; tot_cf[2-1][14-1] = -0.2752096250 ; tot_cf[3-1][14-1] = -0.3139609583;
	tot_cf[1-1][15-1] = -0.1803797500 ; tot_cf[2-1][15-1] = -0.2713843750 ; tot_cf[3-1][15-1] = -0.3642462500;
	tot_cf[1-1][16-1] = -0.2075813750 ; tot_cf[2-1][16-1] = -0.2813540000 ; tot_cf[3-1][16-1] = -0.3869849167;
	tot_cf[1-1][17-1] = -0.2326694167 ; tot_cf[2-1][17-1] = -0.3048955833 ; tot_cf[3-1][17-1] = -0.3678594167;
	tot_cf[1-1][18-1] = -0.2490312500 ; tot_cf[2-1][18-1] = -0.3016313750 ; tot_cf[3-1][18-1] = -0.3485277500;
	tot_cf[1-1][19-1] = -0.2861305833 ; tot_cf[2-1][19-1] = -0.3366778333 ; tot_cf[3-1][19-1] = -0.3481561667;
	tot_cf[1-1][20-1] = -0.2864783750 ; tot_cf[2-1][20-1] = -0.4832055000 ; tot_cf[3-1][20-1] = -0.3070170000;
	tot_cf[1-1][21-1] = -0.1848961250 ; tot_cf[2-1][21-1] = -0.5485212500 ; tot_cf[3-1][21-1] = -0.2763151667;
	tot_cf[1-1][22-1] = -0.2283909583 ; tot_cf[2-1][22-1] = -0.4889305833 ; tot_cf[3-1][22-1] = -0.2305319583;
	tot_cf[1-1][23-1] = -0.2103072083 ; tot_cf[2-1][23-1] = -0.3209973750 ; tot_cf[3-1][23-1] = -0.2045955833;
	tot_cf[1-1][24-1] = -0.2329126250 ; tot_cf[2-1][24-1] = -0.1640147917 ; tot_cf[3-1][24-1] = -0.2306804583;
	tot_cf[1-1][25-1] = -0.1903672083 ; tot_cf[2-1][25-1] = -0.1230767500 ; tot_cf[3-1][25-1] = -0.2036381250;
	tot_cf[1-1][26-1] = -0.2290035000 ; tot_cf[2-1][26-1] = -0.1713146667 ; tot_cf[3-1][26-1] = -0.2115137917;
	tot_cf[1-1][27-1] = -0.2694079167 ; tot_cf[2-1][27-1] = -0.1761405000 ; tot_cf[3-1][27-1] = -0.2759290000;
	tot_cf[1-1][28-1] = -0.2183376667 ; tot_cf[2-1][28-1] = -0.1551655417 ; tot_cf[3-1][28-1] = -0.2932271667;
	tot_cf[1-1][29-1] = -0.2039872917 ; tot_cf[2-1][29-1] = -0.1313691667 ; tot_cf[3-1][29-1] = -0.2675770000;
	tot_cf[1-1][30-1] = -0.2255609583 ; tot_cf[2-1][30-1] = -0.1560321667 ; tot_cf[3-1][30-1] = -0.2561065000;
	tot_cf[1-1][31-1] = -0.1811293333 ; tot_cf[2-1][31-1] = -0.1915475000 ; tot_cf[3-1][31-1] = -0.2489320417;
	tot_cf[1-1][32-1] = -0.0743322500 ; tot_cf[2-1][32-1] = -0.1924822083 ; tot_cf[3-1][32-1] = -0.2089870417;
	tot_cf[1-1][33-1] = -0.0104357917 ; tot_cf[2-1][33-1] = -0.2032437500 ; tot_cf[3-1][33-1] = -0.1352895000;
	tot_cf[1-1][34-1] =  0.0060968333 ; tot_cf[2-1][34-1] = -0.2120782917 ; tot_cf[3-1][34-1] = -0.0557159583;
	tot_cf[1-1][35-1] =  0.0516618750 ; tot_cf[2-1][35-1] = -0.1961577083 ; tot_cf[3-1][35-1] =  0.0268811667;
	tot_cf[1-1][36-1] =  0.1354945833 ; tot_cf[2-1][36-1] = -0.1636799167 ; tot_cf[3-1][36-1] =  0.1256538333;
	tot_cf[1-1][37-1] =  0.2198451667 ; tot_cf[2-1][37-1] = -0.1578588333 ; tot_cf[3-1][37-1] =  0.2183897083;
	tot_cf[1-1][38-1] =  0.3046496250 ; tot_cf[2-1][38-1] = -0.1692336667 ; tot_cf[3-1][38-1] =  0.2984691667;
	tot_cf[1-1][39-1] =  0.3939666667 ; tot_cf[2-1][39-1] = -0.1613200833 ; tot_cf[3-1][39-1] =  0.3622061667;
	tot_cf[1-1][40-1] =  0.4751345833 ; tot_cf[2-1][40-1] = -0.1257707500 ; tot_cf[3-1][40-1] =  0.4292358333;
	tot_cf[1-1][41-1] =  0.5719531667 ; tot_cf[2-1][41-1] = -0.0568519167 ; tot_cf[3-1][41-1] =  0.5438968750;
	tot_cf[1-1][42-1] =  0.6915420417 ; tot_cf[2-1][42-1] =  0.0304964167 ; tot_cf[3-1][42-1] =  0.6858306667;
  }
  if( cf_table_version == 6 )
  {
	tot_cf[1-1][ 1-1] =  0.8018716779 ; tot_cf[2-1][ 1-1] =  0.0859305073 ; tot_cf[3-1][ 1-1] =  0.7325258708;
	tot_cf[1-1][ 2-1] =  0.7181340448 ; tot_cf[2-1][ 2-1] =  0.0366328206 ; tot_cf[3-1][ 2-1] =  0.6042055213;
	tot_cf[1-1][ 3-1] =  0.6295362676 ; tot_cf[2-1][ 3-1] = -0.0244297083 ; tot_cf[3-1][ 3-1] =  0.4839094016;
	tot_cf[1-1][ 4-1] =  0.5176335171 ; tot_cf[2-1][ 4-1] = -0.0571292368 ; tot_cf[3-1][ 4-1] =  0.4064279192;
	tot_cf[1-1][ 5-1] =  0.3947779144 ; tot_cf[2-1][ 5-1] = -0.0776668346 ; tot_cf[3-1][ 5-1] =  0.3473067080;
	tot_cf[1-1][ 6-1] =  0.2975472147 ; tot_cf[2-1][ 6-1] = -0.0927377434 ; tot_cf[3-1][ 6-1] =  0.2745190132;
	tot_cf[1-1][ 7-1] =  0.2325232430 ; tot_cf[2-1][ 7-1] = -0.1039539926 ; tot_cf[3-1][ 7-1] =  0.1845415636;
	tot_cf[1-1][ 8-1] =  0.1741605346 ; tot_cf[2-1][ 8-1] = -0.1228798249 ; tot_cf[3-1][ 8-1] =  0.0940875574;
	tot_cf[1-1][ 9-1] =  0.1079525331 ; tot_cf[2-1][ 9-1] = -0.1256100173 ; tot_cf[3-1][ 9-1] =  0.0319477378;
	tot_cf[1-1][10-1] =  0.0304863229 ; tot_cf[2-1][10-1] = -0.1214661325 ; tot_cf[3-1][10-1] = -0.0035836365;
	tot_cf[1-1][11-1] = -0.0386470673 ; tot_cf[2-1][11-1] = -0.1112376768 ; tot_cf[3-1][11-1] = -0.0406098353;
	tot_cf[1-1][12-1] = -0.0712857387 ; tot_cf[2-1][12-1] = -0.0784461916 ; tot_cf[3-1][12-1] = -0.0859960621;
	tot_cf[1-1][13-1] = -0.0804471816 ; tot_cf[2-1][13-1] = -0.0607186887 ; tot_cf[3-1][13-1] = -0.1411249258;
	tot_cf[1-1][14-1] = -0.0845171736 ; tot_cf[2-1][14-1] = -0.0590661659 ; tot_cf[3-1][14-1] = -0.1660108008;
	tot_cf[1-1][15-1] = -0.0895250829 ; tot_cf[2-1][15-1] = -0.0775878148 ; tot_cf[3-1][15-1] = -0.1630972185;
	tot_cf[1-1][16-1] = -0.0924973527 ; tot_cf[2-1][16-1] = -0.1103278297 ; tot_cf[3-1][16-1] = -0.1812702588;
	tot_cf[1-1][17-1] = -0.0905734784 ; tot_cf[2-1][17-1] = -0.1239095216 ; tot_cf[3-1][17-1] = -0.2255981351;
	tot_cf[1-1][18-1] = -0.1105183555 ; tot_cf[2-1][18-1] = -0.1095731843 ; tot_cf[3-1][18-1] = -0.2642443459;
	tot_cf[1-1][19-1] = -0.1373356640 ; tot_cf[2-1][19-1] = -0.1540148396 ; tot_cf[3-1][19-1] = -0.2325020558;
	tot_cf[1-1][20-1] = -0.1401801900 ; tot_cf[2-1][20-1] = -0.2636431632 ; tot_cf[3-1][20-1] = -0.1516758069;
	tot_cf[1-1][21-1] = -0.1275123642 ; tot_cf[2-1][21-1] = -0.2425632770 ; tot_cf[3-1][21-1] = -0.1698903433;
	tot_cf[1-1][22-1] = -0.1872681977 ; tot_cf[2-1][22-1] = -0.2806181762 ; tot_cf[3-1][22-1] = -0.1606082445;
	tot_cf[1-1][23-1] = -0.1910043819 ; tot_cf[2-1][23-1] = -0.2102294145 ; tot_cf[3-1][23-1] = -0.1692098033;
	tot_cf[1-1][24-1] = -0.1746359531 ; tot_cf[2-1][24-1] = -0.1800489478 ; tot_cf[3-1][24-1] = -0.1933144928;
	tot_cf[1-1][25-1] = -0.1601948025 ; tot_cf[2-1][25-1] = -0.1229574936 ; tot_cf[3-1][25-1] = -0.2030069510;
	tot_cf[1-1][26-1] = -0.1782998925 ; tot_cf[2-1][26-1] = -0.0554334323 ; tot_cf[3-1][26-1] = -0.1840278623;
	tot_cf[1-1][27-1] = -0.1985283679 ; tot_cf[2-1][27-1] = -0.0384197959 ; tot_cf[3-1][27-1] = -0.1712463980;
	tot_cf[1-1][28-1] = -0.1646166970 ; tot_cf[2-1][28-1] = -0.0821969023 ; tot_cf[3-1][28-1] = -0.1783563353;
	tot_cf[1-1][29-1] = -0.1176769274 ; tot_cf[2-1][29-1] = -0.0734982609 ; tot_cf[3-1][29-1] = -0.1873039678;
	tot_cf[1-1][30-1] = -0.0973163795 ; tot_cf[2-1][30-1] = -0.0479640060 ; tot_cf[3-1][30-1] = -0.1628154036;
	tot_cf[1-1][31-1] = -0.0929345980 ; tot_cf[2-1][31-1] = -0.0573003013 ; tot_cf[3-1][31-1] = -0.1006528516;
	tot_cf[1-1][32-1] = -0.0539669988 ; tot_cf[2-1][32-1] = -0.0816801688 ; tot_cf[3-1][32-1] = -0.0333028579;
	tot_cf[1-1][33-1] =  0.0202331226 ; tot_cf[2-1][33-1] = -0.1145149272 ; tot_cf[3-1][33-1] =  0.0131419352;
	tot_cf[1-1][34-1] =  0.1067630285 ; tot_cf[2-1][34-1] = -0.1153900286 ; tot_cf[3-1][34-1] =  0.0473941689;
	tot_cf[1-1][35-1] =  0.1892595998 ; tot_cf[2-1][35-1] = -0.0893081124 ; tot_cf[3-1][35-1] =  0.1034842664;
	tot_cf[1-1][36-1] =  0.2455317387 ; tot_cf[2-1][36-1] = -0.0661231394 ; tot_cf[3-1][36-1] =  0.2008387373;
	tot_cf[1-1][37-1] =  0.2896155178 ; tot_cf[2-1][37-1] = -0.0667160150 ; tot_cf[3-1][37-1] =  0.3045409506;
	tot_cf[1-1][38-1] =  0.3726118543 ; tot_cf[2-1][38-1] = -0.0597727764 ; tot_cf[3-1][38-1] =  0.3931667017;
	tot_cf[1-1][39-1] =  0.4938259789 ; tot_cf[2-1][39-1] = -0.0300710429 ; tot_cf[3-1][39-1] =  0.4581851942;
	tot_cf[1-1][40-1] =  0.5983423610 ; tot_cf[2-1][40-1] = -0.0109126964 ; tot_cf[3-1][40-1] =  0.5174092525;
	tot_cf[1-1][41-1] =  0.6820125025 ; tot_cf[2-1][41-1] =  0.0061174028 ; tot_cf[3-1][41-1] =  0.6133651721;
	tot_cf[1-1][42-1] =  0.7612789545 ; tot_cf[2-1][42-1] =  0.0473044124 ; tot_cf[3-1][42-1] =  0.7235131404;
  }
  
  double d_cell_idx;
  
  switch ( b->beam )
  {
    case 0 :
      d_cell_idx = 22.74078952 + b->t0 *  0.34311458 + pow( b->t0, 2 ) * -0.01077318;
      break;
    case 1 :
      d_cell_idx = 29.85368578 + b->t0 * -0.08197740 + pow( b->t0, 2 ) * -0.00890972;
      break;
    case 2 :
      d_cell_idx = 22.71597845 + b->t0 *  0.34229021 + pow( b->t0, 2 ) * -0.01071746;
      break;
    case 3 :
      d_cell_idx = 19.97380546 + b->t0 * -0.32317523 + pow( b->t0, 2 ) *  0.01045421;
      break;
    case 4 :
      d_cell_idx = 13.11020585 + b->t0 *  0.08378595 + pow( b->t0, 2 ) *  0.00888844;
      break;
    case 5 :
      d_cell_idx = 20.05410608 + b->t0 * -0.32623327 + pow( b->t0, 2 ) *  0.01048349;
      break;
    default:
      fprintf(stderr,"ASCAT_L1B_calibrate_SZF_node: Error: beam index out of range!\n");
      exit(1);
  }
   
  // keep d_cell_idx in bounds depending on the swath
  int    cell_idx_0, cell_idx_1;
  
  if( b->beam < 3 ) // Left-Hand swath
  {
    if( d_cell_idx < 0  ) d_cell_idx =  0;
    if( d_cell_idx > 20 ) d_cell_idx = 20;
  }
  else // right-hand swath
  {
    if( d_cell_idx < 21 ) d_cell_idx = 21;
    if( d_cell_idx > 41 ) d_cell_idx = 41;
  }
  
  cell_idx_0 = int( floor( d_cell_idx ) );
  
  if( b->beam < 3 ) // Left-Hand swath
  {
    // keep cell_idx_0 in range [0->19]
    if( cell_idx_0 < 0  ) cell_idx_0 = 0;
    if( cell_idx_0 > 19 ) cell_idx_0 = 19;
  }
  else // right-hand swath
  {
    // keep cell_idx_0 in range [21->40]
    if( cell_idx_0 < 21 ) cell_idx_0 = 21;
    if( cell_idx_0 > 40 ) cell_idx_0 = 40;
  }  
  cell_idx_1 = cell_idx_0 + 1;  
  
  // linearly interpolate between cf_0 @ cell_idx_0 and cf_1 @ cell_idx_1
  
  double cf_0 = tot_cf[ b->beam % 3 ][ cell_idx_0 ];
  double cf_1 = tot_cf[ b->beam % 3 ][ cell_idx_1 ];
  
  double calibration_factor = cf_0 + ( cf_1 - cf_0 ) * ( d_cell_idx - double(cell_idx_0) ) /
     ( double(cell_idx_1) - double(cell_idx_0) );
    
//  fprintf(stdout,"i_beam, inc_ang, d_cell_idx, cell_idx_0, cell_idx_1, cf0, cf1, cf: %d %f %f %d %d %f %f %f\n",
//          b->beam, b->t0, d_cell_idx, cell_idx_0, cell_idx_1, cf_0, cf_1, calibration_factor );
  
  // Apply calibration factor (or un-apply if i_dir == -1 );
  
  if( i_dir == -1 )
    b->s0 -= calibration_factor;
  else
    b->s0 += calibration_factor;
    
}



///////////////////////////////////
// get szo & szr node data
///////////////////////////////////

static void get_szo_node( unsigned char *x, int i, int swath, AscatNode *b )
{
 // check node index is ok

 printf("I never claimed this would work!!!\n");
 FAIL;


 if( i < 0 ) FAIL;
 if( i >= NODES_SZO ) FAIL;

 // use the node index and
 // swath value to calculate the
 // index of the data in the mdr

 if( swath )
  i += NODES_SZO;
 else
  i = NODES_SZO - 1 - i;

 // extract node data from mdr

 //get_time( x, &b->tm );
 
 get_time( x, &b->tm, b->year, b->month, b->day, b->hour, b->minute, b->second );
 
 get_ushort( x, 26, 1.0e-2, &b->track );
 get_int( x, 154 + i*4, 1.0e-6, &b->lat );
 get_int( x, 322 + i*4, 1.0e-6, &b->lon );
 
 get_ushort( x, 490+i*2, 1.0e-3,  &b->atht );
 get_uint(   x, 574+i*4, 1.0e-10, &b->atls );
 
 get_int( x, 742 + i*12, 1.0e-6, &b->s0, &b->s1, &b->s2 );
 get_ushort( x, 1246 + i*6, 1.0e-4, &b->kp0, &b->kp1, &b->kp2 );
 get_ushort( x, 1498 + i*6, 1.0e-2, &b->t0, &b->t1, &b->t2 );
 get_short( x, 1750 + i*6, 1.0e-2, &b->a0, &b->a1, &b->a2 );
 get_byte( x, 2002 + i*3, &b->fkp0, &b->fkp1, &b->fkp2 );
 get_byte( x, 2128 + i*3, &b->fuse0, &b->fuse1, &b->fuse2 );
 get_ushort( x, 2254 + i*6, 1.0e-3, &b->fsyn0, &b->fsyn1, &b->fsyn2 );
 get_ushort( x, 2506 + i*6, 1.0e-3, &b->fsynq0, &b->fsynq1, &b->fsynq2 );
 get_ushort( x, 2758 + i*6, 1.0e-3, &b->forb0, &b->forb1, &b->forb2 );
 get_ushort( x, 3010 + i*6, 1.0e-3, &b->fsol0, &b->fsol1, &b->fsol2 );
 get_ushort( x, 3262 + i*6, 1.0e-3, &b->ftel0, &b->ftel1, &b->ftel2 );
 get_ushort( x, 3514 + i*6, 1.0e-3, &b->fext0, &b->fext1, &b->fext2 );
 get_ushort( x, 3766 + i*6, 1.0e-3, &b->fland0, &b->fland1, &b->fland2 );
}

static void get_szr_node( unsigned char *x, int i, int swath, AscatNode *b )
{
 // check node index is ok
 
 printf("I never claimed this would work!!!\n");
 FAIL;
 
 if( i < 0 ) FAIL;
 if( i >= NODES_SZR ) FAIL;

 // use the node index and
 // swath value to calculate the
 // index of the data in the mdr

 if( swath )
  i += NODES_SZR;
 else
  i = NODES_SZR - 1 - i;

 // extract node data from mdr

 //get_time( x, &b->tm );
 get_time( x, &b->tm, b->year, b->month, b->day, b->hour, b->minute, b->second );
 get_ushort( x, 26, 1.0e-2, &b->track );
 get_int( x, 274 + i*4, 1.0e-6, &b->lat );
 get_int( x, 602 + i*4, 1.0e-6, &b->lon );
 
 get_ushort( x, 930+i*2, 1.0e-3, &b->atht );
 get_uint(   x, 1094+i*4, 1.0e-10, &b->atls );
 
 get_int( x, 1422 + i*12, 1.0e-6, &b->s0, &b->s1, &b->s2 );
 get_ushort( x, 2406 + i*6, 1.0e-4, &b->kp0, &b->kp1, &b->kp2 );
 get_ushort( x, 2898 + i*6, 1.0e-2, &b->t0, &b->t1, &b->t2 );
 get_short( x, 3390 + i*6, 1.0e-2, &b->a0, &b->a1, &b->a2 );
 get_byte( x, 3882 + i*3, &b->fkp0, &b->fkp1, &b->fkp2 );
 get_byte( x, 4128 + i*3, &b->fuse0, &b->fuse1, &b->fuse2 );
 get_ushort( x, 4374 + i*6, 1.0e-3, &b->fsyn0, &b->fsyn1, &b->fsyn2 );
 get_ushort( x, 4866 + i*6, 1.0e-3, &b->fsynq0, &b->fsynq1, &b->fsynq2 );
 get_ushort( x, 5358 + i*6, 1.0e-3, &b->forb0, &b->forb1, &b->forb2 );
 get_ushort( x, 5850 + i*6, 1.0e-3, &b->fsol0, &b->fsol1, &b->fsol2 );
 get_ushort( x, 6342 + i*6, 1.0e-3, &b->ftel0, &b->ftel1, &b->ftel2 );
 get_ushort( x, 6834 + i*6, 1.0e-3, &b->fext0, &b->fext1, &b->fext2 );
 get_ushort( x, 7326 + i*6, 1.0e-3, &b->fland0, &b->fland1, &b->fland2 );
}

void AscatFile::get_node( int i, int swath, AscatNode *b )
{

 int julian1, julian2, time_diff;

 switch( nn )
  {
   case NODES_SZO :
    get_szo_node(mdr,i,swath,b);
    break;
   case NODES_SZR :
    get_szr_node(mdr,i,swath,b);
    break;
   default :
    FAIL;
  }
 //calculate time diference between sensing time and state vector time
 ymd2julian( s_v_year, s_v_month, s_v_day, julian1 );
 ymd2julian( b->year,  b->month,  b->day,  julian2 );
 
 if( fabs( julian2 - julian1) <= 1 )
 {
   time_diff = (julian2 - julian1) * 60 * 60 * 24;
   time_diff = time_diff + (b->hour   - s_v_hour  ) * 60 * 60;
   time_diff = time_diff + (b->minute - s_v_minute) * 60;
   time_diff = time_diff + (b->second - s_v_second);
 }
 else
   time_diff = 0;

 // fill node data from header data
 // correct orbit number taking the time difference into account
 // MetOp does one orbit in ~101 minutes (6081.72 seconds)
 b->proc_maj = proc_maj;
 b->proc_min = proc_min;
 b->sat      = sat;
 b->orbit    = orbit + floor( float(time_diff) / 6081.72 );
 b->index    = i;
 b->swath    = swath;
 b->asc      = calc_asc( b->track );
 
 // transform lon and azimuth angles
 if( b->lon > 180 ) b->lon -= 360;
 if( b->a0 < 0    ) b->a0  += 360;
 if( b->a1 < 0    ) b->a1  += 360;
 if( b->a2 < 0    ) b->a2  += 360;
 
 // represent Kp values as percentage
 b->kp0 *= 100;
 b->kp1 *= 100;
 b->kp2 *= 100;
 
 // represent atmospheric height and loss in meters
 b->atht *= 1000;
 b->atls *= 1.0e-3;
}

///////////////////////////////////
// get szf node data
///////////////////////////////////

void AscatFile::get_node( int i, int beam, AscatSZFNode *b )
{
 int julian1, julian2, time_diff;
 int ant;
 int pos; 

 if( nn   != NODES_SZF ) FAIL;
 if( i    < 0          ) FAIL;
 if( i    >= NODES_SZF ) FAIL;
 
 if( beam < 0 || beam > 5 )
 {
   printf("ERROR: beam index is invalid in AscatFile::get_node %d\n",beam);
   FAIL;
 }
 
 // index offset for arrays (mulitply by 4 for byte offset of 4 bytes types) 
 // where 1st dimension of array is 256 and second is 6.
 pos = beam * 256 + i;

 get_time(   mdr, &b->tm, b->year, b->month, b->day, b->hour, b->minute, b->second );
 
 get_int(    mdr, 68    + beam*4, 1.0e-2,  &b->track );
 get_int(    mdr, 128   + pos*4,  1.0e-6,  &b->s0 );
 get_int(    mdr, 6272  + pos*4,  1.0e-6,  &b->t0 );
 get_int(    mdr, 12416 + pos*4,  1.0e-6,  &b->a0 );
 get_int(    mdr, 18560 + pos*4,  1.0e-6,  &b->lat );
 get_int(    mdr, 24704 + pos*4,  1.0e-6,  &b->lon );
 get_ushort( mdr, 30848 + pos*2,  1.0e-3,  &b->atht );
 get_uint(   mdr, 33920 + pos*4,  1.0e-10, &b->atls );
 get_byte(   mdr, 40064 + beam,            &b->fsyn );
 get_byte(   mdr, 40070 + beam,            &b->fref );
 get_byte(   mdr, 40076 + beam,            &b->forb );
 get_byte(   mdr, 40082 + beam,            &b->fgen1 );
 get_byte(   mdr, 40088 + pos,             &b->fgen2 );

 //calculate time diference between sensing time and state vector time
 ymd2julian( s_v_year, s_v_month, s_v_day, julian1 );
 ymd2julian( b->year,  b->month,  b->day,  julian2 );
 
 if( fabs( julian2 - julian1) <= 1 )
 {
   time_diff = (julian2 - julian1) * 60 * 60 * 24;
   time_diff = time_diff + (b->hour   - s_v_hour  ) * 60 * 60;
   time_diff = time_diff + (b->minute - s_v_minute) * 60;
   time_diff = time_diff + (b->second - s_v_second);
 }
 else
   time_diff = 0;

 // fill node data from header data
 // correct orbit number taking the time difference into account
 // MetOp does one orbit in ~101 minutes (6081.72 seconds)
 b->proc_maj = proc_maj;
 b->proc_min = proc_min;
 b->sat      = sat;
 b->orbit    = orbit + floor( float(time_diff) / 6081.72 );
 b->index    = i;
 b->beam     = beam;
 b->asc      = calc_asc( b->track );
 
 
 
 // transform lon and azimuth angles
 if( b->lon > 180 ) b->lon -= 360;
 
 // Transform azimuth angle to angle between north and projection
 // of pointing vector on ground, measured clockwise. 
 // (p. 132 of ascat L1 product generation function spec document).
 b->a0 += 180;
 if( b->a0 > 360 ) b->a0 -= 360;
 
 // represent atmospheric height and loss in meters
 b->atht *= 1000;
 b->atls *= 1.0e-3;
 
 // Apply beam dependant calibration factors obtained from AWDP processing software.
 ASCAT_L1B_calibrate_SZF_node( b, 1 );
}


void AscatFile::get_node_new(int inode, AscatSZFNodeNew *ascat_szf_node) {
// for format 12 and up
    int julian1, julian2, time_diff;
    int ant;
    int pos;

    if(nn != NODES_SZF_NEW || inode < 0 || inode >= NODES_SZF_NEW) FAIL;

    // index offset for arrays (mulitply by 4 for byte offset of 4 bytes types)
    pos = inode;

    double tm;
    get_time_new(
        mdr, &tm, ascat_szf_node->year, ascat_szf_node->month,
        ascat_szf_node->day, ascat_szf_node->hour, ascat_szf_node->minute,
        ascat_szf_node->second);

    ascat_szf_node->tm = tm;

    get_ushort(mdr, 28, 1.0e-2, &ascat_szf_node->track);
    get_byte(mdr, 30, &ascat_szf_node->is_asc);
    get_byte(mdr, 31, &ascat_szf_node->beam);
    get_int(mdr, 32+pos*4, 1.0e-6, &ascat_szf_node->s0);
    get_ushort(mdr, 800+pos*2, 1.0e-2, &ascat_szf_node->t0);
    get_short(mdr, 1184+pos*2, 1.0e-2, &ascat_szf_node->a0);
    get_int(mdr, 1568+pos*4, 1.0e-6, &ascat_szf_node->lat);
    get_int(mdr, 2336+pos*4, 1.0e-6, &ascat_szf_node->lon);
    get_byte(mdr, 3488, &ascat_szf_node->fref1);
    get_byte(mdr, 3489, &ascat_szf_node->fref2);
    get_byte(mdr, 3490, &ascat_szf_node->fpl);
    get_byte(mdr, 3491, &ascat_szf_node->fgen1);
    get_byte(mdr, 3492+pos, &ascat_szf_node->fgen2);

    ascat_szf_node->beam = (int)mdr[31];

    // transform lon and azimuth angles
    if(ascat_szf_node->lon > 180) ascat_szf_node->lon -= 360;
 
    // Transform azimuth angle to angle between north and projection
    // of pointing vector on ground, measured clockwise. 
    // (p. 132 of ascat L1 product generation function spec document).
    ascat_szf_node->a0 += 180;
    if(ascat_szf_node->a0 > 360)
        ascat_szf_node->a0 -= 360;

    // Check quality flags, set usability
    unsigned char FLAGFIELD_RF1_RED_MASK = 0x14;
    unsigned char FLAGFIELD_RF1_AMBER_MASK = 0x0B;
    unsigned char FLAGFIELD_RF2_RED_MASK = 0x03;
    unsigned char FLAGFIELD_RF2_AMBER_MASK = 0x04;
    unsigned char FLAGFIELD_PL_RED_MASK = 0x0F;
    unsigned char FLAGFIELD_GEN1_RED_MASK = 0x02;
    unsigned char FLAGFIELD_GEN1_AMBER_MASK = 0x01;
    unsigned char FLAGFIELD_GEN2_RED_MASK = 0x04;
    unsigned char FLAGFIELD_GEN2_AMBER_MASK = 0x01;

    int is_red = 0;
    if( ascat_szf_node->fref1 & FLAGFIELD_RF1_RED_MASK ||
        ascat_szf_node->fref2 & FLAGFIELD_RF2_RED_MASK ||
        ascat_szf_node->fpl & FLAGFIELD_PL_RED_MASK ||
        ascat_szf_node->fgen1 & FLAGFIELD_GEN1_RED_MASK ||
        ascat_szf_node->fgen2 & FLAGFIELD_GEN2_RED_MASK) is_red = 1;

    int is_amber = 0;
    if( ascat_szf_node->fref1 & FLAGFIELD_RF1_AMBER_MASK ||
        ascat_szf_node->fref2 & FLAGFIELD_RF2_AMBER_MASK ||
        ascat_szf_node->fgen1 & FLAGFIELD_GEN1_AMBER_MASK ||
        ascat_szf_node->fgen2 & FLAGFIELD_GEN2_AMBER_MASK) is_amber = 1;

    ascat_szf_node->is_good = (is_red == 0 && is_amber == 0) ? 1 : 0;
    ascat_szf_node->is_marginal = (is_red == 0 && is_amber == 1) ? 1 : 0;
    ascat_szf_node->is_bad = (is_red > 0) ?  1 : 0;
    ascat_szf_node->is_land = (ascat_szf_node->fgen2 & 0x02) ? 1 : 0;

    ascat_szf_node->s0 = pow(10.0, 0.1*ascat_szf_node->s0);
    if(ascat_szf_node->fgen2 & 0x08)
        ascat_szf_node->s0 *= -1;

}

ASCATNOC::ASCATNOC() {
    return;
}

ASCATNOC::ASCATNOC(const char* filename) {
    Read(filename);
    return;
}

ASCATNOC::~ASCATNOC() {
    return;
}

int ASCATNOC::Read(const char* filename) {
    FILE* ifp = fopen(filename, "r");
    fread(&_incmin, sizeof(float), 1, ifp);
    fread(&_dinc, sizeof(float), 1, ifp);
    fread(&_ninc, sizeof(int), 1, ifp);
    _table.resize(6*_ninc);
    fread(&_table[0], sizeof(float), _ninc*6, ifp);
    fclose(ifp);
    return(1);
}

int ASCATNOC::Get(float inc, int ibeam, float* correction) {

    if(inc < _incmin || inc > _incmin+_dinc*_ninc) {
        printf("ASCATNOC::Get: inputs out of range: %f\n", inc);
        *correction = 0;
        return(0);
    }

    int iinc0 = floor((inc-_incmin)/_dinc);
    if(iinc0 == _ninc) iinc0 = _ninc - 1;

    int iinc1 = 1 + iinc0;

    float finc0 = _incmin+(float)iinc0*_dinc;
    float finc1 = _incmin+(float)iinc1*_dinc;
    float corr0 = _table[iinc0+ibeam*_ninc];
    float corr1 = _table[iinc1+ibeam*_ninc];

    *correction = corr0 + (inc-finc0)*(corr1-corr0) / (finc1-finc0);

//     printf(
//         "%f %d %d %f %f %f %f %f\n", inc, iinc0, iinc1, finc0, finc1, corr0,
//         corr1, *correction);

    return(1);
}
