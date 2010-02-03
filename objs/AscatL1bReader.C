
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

// number of nodes

#define NODES_SZO   21
#define NODES_SZR   41
#define NODES_SZF  256

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

static void get_ushort( unsigned char *x, int pos, double sf, double *ans )
{
 int a;

 get_ushort(x,pos,&a);
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
   nn = NODES_SZF;
   size = SZF_SIZE;
   id = SZF_ID;
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
  
  
  cf_table_version = 1;
  if( software_id >= 502 ) cf_table_version = 2;
  if( software_id >= 503 ) cf_table_version = 3;
  if( software_id >= 603 ) cf_table_version = 4;
  
//  fprintf(stdout,"cf_table_version: %d\n",cf_table_version);
  
  
  // calibration factors are added to sigma-0 when sigma0 is in units of dB; 
  // thus a mutiplicative factor for sigma-0 in linear units.
  if( cf_table_version == 1 )
  {
tot_cf[0][0] =   1.213313; tot_cf[1][0] =   0.315538; tot_cf[2][0] =   1.059316;
tot_cf[0][1] =   1.054658; tot_cf[1][1] =   0.327152; tot_cf[2][1] =   0.846011;
tot_cf[0][2] =   0.886552; tot_cf[1][2] =   0.247977; tot_cf[2][2] =   0.649600;
tot_cf[0][3] =   0.732555; tot_cf[1][3] =   0.145125; tot_cf[2][3] =   0.499465;
tot_cf[0][4] =   0.582938; tot_cf[1][4] =   0.033152; tot_cf[2][4] =   0.384449;
tot_cf[0][5] =   0.428137; tot_cf[1][5] =  -0.084411; tot_cf[2][5] =   0.289353;
tot_cf[0][6] =   0.296586; tot_cf[1][6] =  -0.171885; tot_cf[2][6] =   0.239143;
tot_cf[0][7] =   0.237024; tot_cf[1][7] =  -0.171666; tot_cf[2][7] =   0.134594;
tot_cf[0][8] =   0.280968; tot_cf[1][8] =  -0.147833; tot_cf[2][8] =   0.007191;
tot_cf[0][9] =   0.179476; tot_cf[1][9] =  -0.221267; tot_cf[2][9] =   0.031042;
tot_cf[0][10] =   0.208221; tot_cf[1][10] =  -0.323422; tot_cf[2][10] =  -0.098438;
tot_cf[0][11] =   0.106781; tot_cf[1][11] =  -0.192095; tot_cf[2][11] =  -0.294169;
tot_cf[0][12] =   0.130019; tot_cf[1][12] =  -0.147054; tot_cf[2][12] =  -0.400186;
tot_cf[0][13] =  -0.042703; tot_cf[1][13] =   0.161051; tot_cf[2][13] =  -0.411267;
tot_cf[0][14] =  -0.139033; tot_cf[1][14] =   0.141696; tot_cf[2][14] =  -0.258106;
tot_cf[0][15] =  -0.242416; tot_cf[1][15] =  -0.034760; tot_cf[2][15] =  -0.160887;
tot_cf[0][16] =  -0.229325; tot_cf[1][16] =  -0.192706; tot_cf[2][16] =  -0.128773;
tot_cf[0][17] =   0.015222; tot_cf[1][17] =  -0.230362; tot_cf[2][17] =  -0.044352;
tot_cf[0][18] =   0.191119; tot_cf[1][18] =  -0.054178; tot_cf[2][18] =   0.108382;
tot_cf[0][19] =   0.173226; tot_cf[1][19] =  -0.016895; tot_cf[2][19] =   0.232809;
tot_cf[0][20] =   0.085356; tot_cf[1][20] =  -0.013824; tot_cf[2][20] =   0.014689;
tot_cf[0][21] =  -0.271788; tot_cf[1][21] =  -0.296621; tot_cf[2][21] =  -0.211424;
tot_cf[0][22] =   0.242286; tot_cf[1][22] =  -0.394656; tot_cf[2][22] =   0.254568;
tot_cf[0][23] =   0.178912; tot_cf[1][23] =  -0.162926; tot_cf[2][23] =   0.283585;
tot_cf[0][24] =  -0.009471; tot_cf[1][24] =   0.140090; tot_cf[2][24] =   0.156260;
tot_cf[0][25] =  -0.227179; tot_cf[1][25] =   0.328872; tot_cf[2][25] =   0.123964;
tot_cf[0][26] =  -0.217824; tot_cf[1][26] =   0.474185; tot_cf[2][26] =  -0.116240;
tot_cf[0][27] =  -0.272308; tot_cf[1][27] =   0.502942; tot_cf[2][27] =  -0.088553;
tot_cf[0][28] =  -0.227287; tot_cf[1][28] =   0.399949; tot_cf[2][28] =   0.037972;
tot_cf[0][29] =  -0.044225; tot_cf[1][29] =   0.294833; tot_cf[2][29] =   0.072355;
tot_cf[0][30] =   0.045013; tot_cf[1][30] =   0.253380; tot_cf[2][30] =   0.119882;
tot_cf[0][31] =   0.112916; tot_cf[1][31] =   0.229710; tot_cf[2][31] =   0.195454;
tot_cf[0][32] =   0.151822; tot_cf[1][32] =   0.144005; tot_cf[2][32] =   0.336945;
tot_cf[0][33] =   0.353612; tot_cf[1][33] =   0.097862; tot_cf[2][33] =   0.268630;
tot_cf[0][34] =   0.342894; tot_cf[1][34] =   0.102309; tot_cf[2][34] =   0.388789;
tot_cf[0][35] =   0.401056; tot_cf[1][35] =   0.101148; tot_cf[2][35] =   0.462459;
tot_cf[0][36] =   0.443390; tot_cf[1][36] =   0.155380; tot_cf[2][36] =   0.521556;
tot_cf[0][37] =   0.498341; tot_cf[1][37] =   0.229020; tot_cf[2][37] =   0.592000;
tot_cf[0][38] =   0.596388; tot_cf[1][38] =   0.270325; tot_cf[2][38] =   0.686273;
tot_cf[0][39] =   0.746513; tot_cf[1][39] =   0.218982; tot_cf[2][39] =   0.807296;
tot_cf[0][40] =   0.888548; tot_cf[1][40] =   0.261273; tot_cf[2][40] =   0.899864;
tot_cf[0][41] =   1.077605; tot_cf[1][41] =   0.426358; tot_cf[2][41] =   0.960776;
  }  
  if( cf_table_version == 2 )
  {
tot_cf[0][0] =   1.250347; tot_cf[1][0] =   0.617395; tot_cf[2][0] =   1.096529;
tot_cf[0][1] =   1.141402; tot_cf[1][1] =   0.500127; tot_cf[2][1] =   1.013789;
tot_cf[0][2] =   1.020764; tot_cf[1][2] =   0.355656; tot_cf[2][2] =   0.863830;
tot_cf[0][3] =   0.901663; tot_cf[1][3] =   0.214202; tot_cf[2][3] =   0.698354;
tot_cf[0][4] =   0.772740; tot_cf[1][4] =   0.096218; tot_cf[2][4] =   0.541182;
tot_cf[0][5] =   0.619394; tot_cf[1][5] =   0.010119; tot_cf[2][5] =   0.401760;
tot_cf[0][6] =   0.470278; tot_cf[1][6] =  -0.016497; tot_cf[2][6] =   0.322805;
tot_cf[0][7] =   0.382494; tot_cf[1][7] =   0.053763; tot_cf[2][7] =   0.216615;
tot_cf[0][8] =   0.400010; tot_cf[1][8] =   0.131225; tot_cf[2][8] =   0.117441;
tot_cf[0][9] =   0.288156; tot_cf[1][9] =   0.069674; tot_cf[2][9] =   0.197094;
tot_cf[0][10] =   0.328857; tot_cf[1][10] =  -0.070852; tot_cf[2][10] =   0.140768;
tot_cf[0][11] =   0.259876; tot_cf[1][11] =  -0.013951; tot_cf[2][11] =   0.017488;
tot_cf[0][12] =   0.330230; tot_cf[1][12] =  -0.050653; tot_cf[2][12] =  -0.037931;
tot_cf[0][13] =   0.203377; tot_cf[1][13] =   0.194497; tot_cf[2][13] =  -0.036577;
tot_cf[0][14] =   0.122389; tot_cf[1][14] =   0.147475; tot_cf[2][14] =   0.074733;
tot_cf[0][15] =  -0.014907; tot_cf[1][15] =  -0.014204; tot_cf[2][15] =   0.064328;
tot_cf[0][16] =  -0.088596; tot_cf[1][16] =  -0.126842; tot_cf[2][16] =  -0.066181;
tot_cf[0][17] =   0.035517; tot_cf[1][17] =  -0.110529; tot_cf[2][17] =  -0.154675;
tot_cf[0][18] =   0.105704; tot_cf[1][18] =   0.111736; tot_cf[2][18] =  -0.113623;
tot_cf[0][19] =   0.039402; tot_cf[1][19] =   0.180982; tot_cf[2][19] =   0.036362;
tot_cf[0][20] =   0.001833; tot_cf[1][20] =   0.182166; tot_cf[2][20] =   0.037808;
tot_cf[0][21] =   0.041734; tot_cf[1][21] =   0.016276; tot_cf[2][21] =   0.053222;
tot_cf[0][22] =   0.035736; tot_cf[1][22] =  -0.079141; tot_cf[2][22] =  -0.099798;
tot_cf[0][23] =   0.007548; tot_cf[1][23] =  -0.046546; tot_cf[2][23] =  -0.195821;
tot_cf[0][24] =   0.012427; tot_cf[1][24] =   0.058562; tot_cf[2][24] =  -0.200547;
tot_cf[0][25] =  -0.010088; tot_cf[1][25] =   0.121598; tot_cf[2][25] =  -0.021505;
tot_cf[0][26] =   0.118721; tot_cf[1][26] =   0.221479; tot_cf[2][26] =  -0.070372;
tot_cf[0][27] =   0.093595; tot_cf[1][27] =   0.268314; tot_cf[2][27] =   0.079026;
tot_cf[0][28] =   0.098474; tot_cf[1][28] =   0.235936; tot_cf[2][28] =   0.250564;
tot_cf[0][29] =   0.207157; tot_cf[1][29] =   0.237612; tot_cf[2][29] =   0.278741;
tot_cf[0][30] =   0.221811; tot_cf[1][30] =   0.303251; tot_cf[2][30] =   0.294028;
tot_cf[0][31] =   0.231575; tot_cf[1][31] =   0.354704; tot_cf[2][31] =   0.329075;
tot_cf[0][32] =   0.238619; tot_cf[1][32] =   0.286591; tot_cf[2][32] =   0.436648;
tot_cf[0][33] =   0.435178; tot_cf[1][33] =   0.198793; tot_cf[2][33] =   0.350946;
tot_cf[0][34] =   0.439077; tot_cf[1][34] =   0.127453; tot_cf[2][34] =   0.469962;
tot_cf[0][35] =   0.519876; tot_cf[1][35] =   0.056315; tot_cf[2][35] =   0.552711;
tot_cf[0][36] =   0.582019; tot_cf[1][36] =   0.084147; tot_cf[2][36] =   0.624881;
tot_cf[0][37] =   0.645297; tot_cf[1][37] =   0.196938; tot_cf[2][37] =   0.708500;
tot_cf[0][38] =   0.739055; tot_cf[1][38] =   0.337418; tot_cf[2][38] =   0.812105;
tot_cf[0][39] =   0.872448; tot_cf[1][39] =   0.401204; tot_cf[2][39] =   0.938522;
tot_cf[0][40] =   0.989461; tot_cf[1][40] =   0.502660; tot_cf[2][40] =   1.030320;
tot_cf[0][41] =   1.143630; tot_cf[1][41] =   0.621548; tot_cf[2][41] =   1.077636;
  }
  if( cf_table_version == 3 )
  {
tot_cf[0][0] =   0.978904; tot_cf[1][0] =   0.062482; tot_cf[2][0] =   0.723580;
tot_cf[0][1] =   0.832068; tot_cf[1][1] =  -0.046070; tot_cf[2][1] =   0.550661;
tot_cf[0][2] =   0.676243; tot_cf[1][2] =  -0.157757; tot_cf[2][2] =   0.384415;
tot_cf[0][3] =   0.532541; tot_cf[1][3] =  -0.241462; tot_cf[2][3] =   0.255324;
tot_cf[0][4] =   0.390777; tot_cf[1][4] =  -0.307394; tot_cf[2][4] =   0.154009;
tot_cf[0][5] =   0.240653; tot_cf[1][5] =  -0.369959; tot_cf[2][5] =   0.066827;
tot_cf[0][6] =   0.108179; tot_cf[1][6] =  -0.404602; tot_cf[2][6] =   0.019773;
tot_cf[0][7] =   0.042812; tot_cf[1][7] =  -0.361632; tot_cf[2][7] =  -0.082582;
tot_cf[0][8] =   0.075794; tot_cf[1][8] =  -0.310396; tot_cf[2][8] =  -0.206640;
tot_cf[0][9] =  -0.037946; tot_cf[1][9] =  -0.378967; tot_cf[2][9] =  -0.173441;
tot_cf[0][10] =  -0.020832; tot_cf[1][10] =  -0.503009; tot_cf[2][10] =  -0.284991;
tot_cf[0][11] =  -0.128614; tot_cf[1][11] =  -0.420507; tot_cf[2][11] =  -0.453169;
tot_cf[0][12] =  -0.100895; tot_cf[1][12] =  -0.445680; tot_cf[2][12] =  -0.527273;
tot_cf[0][13] =  -0.254754; tot_cf[1][13] =  -0.217931; tot_cf[2][13] =  -0.513653;
tot_cf[0][14] =  -0.320456; tot_cf[1][14] =  -0.307182; tot_cf[2][14] =  -0.359836;
tot_cf[0][15] =  -0.392269; tot_cf[1][15] =  -0.510089; tot_cf[2][15] =  -0.309107;
tot_cf[0][16] =  -0.375625; tot_cf[1][16] =  -0.636786; tot_cf[2][16] =  -0.387222;
tot_cf[0][17] =  -0.176910; tot_cf[1][17] =  -0.611737; tot_cf[2][17] =  -0.456693;
tot_cf[0][18] =  -0.092512; tot_cf[1][18] =  -0.403131; tot_cf[2][18] =  -0.443358;
tot_cf[0][19] =  -0.198109; tot_cf[1][19] =  -0.426740; tot_cf[2][19] =  -0.356663;
tot_cf[0][20] =  -0.264940; tot_cf[1][20] =  -0.582371; tot_cf[2][20] =  -0.410173;
tot_cf[0][21] =  -0.293126; tot_cf[1][21] =  -0.643804; tot_cf[2][21] =  -0.187479;
tot_cf[0][22] =  -0.243289; tot_cf[1][22] =  -0.484003; tot_cf[2][22] =  -0.256247;
tot_cf[0][23] =  -0.231298; tot_cf[1][23] =  -0.314617; tot_cf[2][23] =  -0.309868;
tot_cf[0][24] =  -0.225389; tot_cf[1][24] =  -0.210548; tot_cf[2][24] =  -0.334784;
tot_cf[0][25] =  -0.275315; tot_cf[1][25] =  -0.210228; tot_cf[2][25] =  -0.226410;
tot_cf[0][26] =  -0.182171; tot_cf[1][26] =  -0.159405; tot_cf[2][26] =  -0.363593;
tot_cf[0][27] =  -0.239071; tot_cf[1][27] =  -0.125425; tot_cf[2][27] =  -0.291819;
tot_cf[0][28] =  -0.256832; tot_cf[1][28] =  -0.152653; tot_cf[2][28] =  -0.169163;
tot_cf[0][29] =  -0.162447; tot_cf[1][29] =  -0.154995; tot_cf[2][29] =  -0.162807;
tot_cf[0][30] =  -0.157045; tot_cf[1][30] =  -0.113302; tot_cf[2][30] =  -0.149751;
tot_cf[0][31] =  -0.150633; tot_cf[1][31] =  -0.091674; tot_cf[2][31] =  -0.103539;
tot_cf[0][32] =  -0.144668; tot_cf[1][32] =  -0.166707; tot_cf[2][32] =   0.018360;
tot_cf[0][33] =   0.051210; tot_cf[1][33] =  -0.220136; tot_cf[2][33] =  -0.060597;
tot_cf[0][34] =   0.055128; tot_cf[1][34] =  -0.223288; tot_cf[2][34] =   0.058017;
tot_cf[0][35] =   0.140291; tot_cf[1][35] =  -0.222155; tot_cf[2][35] =   0.136536;
tot_cf[0][36] =   0.213666; tot_cf[1][36] =  -0.154059; tot_cf[2][36] =   0.206970;
tot_cf[0][37] =   0.295988; tot_cf[1][37] =  -0.060244; tot_cf[2][37] =   0.292932;
tot_cf[0][38] =   0.412453; tot_cf[1][38] =   0.001997; tot_cf[2][38] =   0.405506;
tot_cf[0][39] =   0.569140; tot_cf[1][39] =  -0.036788; tot_cf[2][39] =   0.546005;
tot_cf[0][40] =   0.703598; tot_cf[1][40] =   0.003561; tot_cf[2][40] =   0.659630;
tot_cf[0][41] =   0.871693; tot_cf[1][41] =   0.147183; tot_cf[2][41] =   0.744380;
  }
  if( cf_table_version == 4 )
  {
tot_cf[0][0] =   0.994155; tot_cf[1][0] =   0.061119; tot_cf[2][0] =   0.722671;
tot_cf[0][1] =   0.837910; tot_cf[1][1] =  -0.035447; tot_cf[2][1] =   0.621881;
tot_cf[0][2] =   0.690169; tot_cf[1][2] =  -0.132332; tot_cf[2][2] =   0.483625;
tot_cf[0][3] =   0.570068; tot_cf[1][3] =  -0.208911; tot_cf[2][3] =   0.329411;
tot_cf[0][4] =   0.453514; tot_cf[1][4] =  -0.272799; tot_cf[2][4] =   0.177673;
tot_cf[0][5] =   0.310207; tot_cf[1][5] =  -0.326515; tot_cf[2][5] =   0.055586;
tot_cf[0][6] =   0.157507; tot_cf[1][6] =  -0.353697; tot_cf[2][6] =   0.005338;
tot_cf[0][7] =   0.062826; tot_cf[1][7] =  -0.334625; tot_cf[2][7] =  -0.089646;
tot_cf[0][8] =   0.087063; tot_cf[1][8] =  -0.336839; tot_cf[2][8] =  -0.221671;
tot_cf[0][9] =  -0.011991; tot_cf[1][9] =  -0.419736; tot_cf[2][9] =  -0.201579;
tot_cf[0][10] =   0.008408; tot_cf[1][10] =  -0.467333; tot_cf[2][10] =  -0.293766;
tot_cf[0][11] =  -0.128215; tot_cf[1][11] =  -0.301783; tot_cf[2][11] =  -0.405725;
tot_cf[0][12] =  -0.119913; tot_cf[1][12] =  -0.384475; tot_cf[2][12] =  -0.441224;
tot_cf[0][13] =  -0.233324; tot_cf[1][13] =  -0.331128; tot_cf[2][13] =  -0.455676;
tot_cf[0][14] =  -0.232574; tot_cf[1][14] =  -0.460407; tot_cf[2][14] =  -0.369246;
tot_cf[0][15] =  -0.288933; tot_cf[1][15] =  -0.445690; tot_cf[2][15] =  -0.343045;
tot_cf[0][16] =  -0.310478; tot_cf[1][16] =  -0.371600; tot_cf[2][16] =  -0.372309;
tot_cf[0][17] =  -0.169567; tot_cf[1][17] =  -0.423545; tot_cf[2][17] =  -0.409216;
tot_cf[0][18] =  -0.136589; tot_cf[1][18] =  -0.448280; tot_cf[2][18] =  -0.457410;
tot_cf[0][19] =  -0.219806; tot_cf[1][19] =  -0.554047; tot_cf[2][19] =  -0.413683;
tot_cf[0][20] =  -0.192938; tot_cf[1][20] =  -0.602546; tot_cf[2][20] =  -0.400480;
tot_cf[0][21] =  -0.233668; tot_cf[1][21] =  -0.704763; tot_cf[2][21] =  -0.152655;
tot_cf[0][22] =  -0.203266; tot_cf[1][22] =  -0.512465; tot_cf[2][22] =  -0.256590;
tot_cf[0][23] =  -0.247615; tot_cf[1][23] =  -0.232340; tot_cf[2][23] =  -0.333497;
tot_cf[0][24] =  -0.196758; tot_cf[1][24] =  -0.142936; tot_cf[2][24] =  -0.308635;
tot_cf[0][25] =  -0.238210; tot_cf[1][25] =  -0.253262; tot_cf[2][25] =  -0.194430;
tot_cf[0][26] =  -0.152073; tot_cf[1][26] =  -0.219312; tot_cf[2][26] =  -0.371747;
tot_cf[0][27] =  -0.199550; tot_cf[1][27] =  -0.109318; tot_cf[2][27] =  -0.283815;
tot_cf[0][28] =  -0.263122; tot_cf[1][28] =  -0.103932; tot_cf[2][28] =  -0.116152;
tot_cf[0][29] =  -0.219558; tot_cf[1][29] =  -0.143894; tot_cf[2][29] =  -0.122605;
tot_cf[0][30] =  -0.173299; tot_cf[1][30] =  -0.131045; tot_cf[2][30] =  -0.169699;
tot_cf[0][31] =  -0.089944; tot_cf[1][31] =  -0.098944; tot_cf[2][31] =  -0.157651;
tot_cf[0][32] =  -0.079744; tot_cf[1][32] =  -0.165385; tot_cf[2][32] =  -0.014689;
tot_cf[0][33] =   0.059277; tot_cf[1][33] =  -0.232703; tot_cf[2][33] =  -0.049270;
tot_cf[0][34] =   0.034629; tot_cf[1][34] =  -0.241926; tot_cf[2][34] =   0.098885;
tot_cf[0][35] =   0.150073; tot_cf[1][35] =  -0.221590; tot_cf[2][35] =   0.183172;
tot_cf[0][36] =   0.262848; tot_cf[1][36] =  -0.137410; tot_cf[2][36] =   0.244417;
tot_cf[0][37] =   0.345334; tot_cf[1][37] =  -0.056015; tot_cf[2][37] =   0.316449;
tot_cf[0][38] =   0.428961; tot_cf[1][38] =  -0.013352; tot_cf[2][38] =   0.415896;
tot_cf[0][39] =   0.559809; tot_cf[1][39] =  -0.041328; tot_cf[2][39] =   0.547995;
tot_cf[0][40] =   0.700251; tot_cf[1][40] =   0.034199; tot_cf[2][40] =   0.662183;
tot_cf[0][41] =   0.896225; tot_cf[1][41] =   0.190020; tot_cf[2][41] =   0.757698;
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
 get_uint(   mdr, 33920 + pos*2,  1.0e-10, &b->atls );
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




