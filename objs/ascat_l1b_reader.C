
using namespace std;

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ascat_l1b_reader.h"

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

ascat_file::ascat_file()
{
 mdr = NULL;
 fp = NULL;
}

ascat_file::~ascat_file()
{
 if( mdr ) free(mdr);
 if( fp ) fclose(fp);
}

void ascat_file::close()
{
 if( fp ) { fclose(fp); fp = NULL; }
 if( mdr ) { free(mdr); mdr = NULL; }
}

///////////////////////////////////
// open file, read mphr and
// extract information
///////////////////////////////////

int ascat_file::open( string fname, int *nr0, int *nn0 )
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
 fp = fopen(fname.c_str(),"r");
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

int ascat_file::read_mdr( int *isdummy )
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

///////////////////////////////////
// get szo & szr node data
///////////////////////////////////

static void get_szo_node( unsigned char *x, int i, int swath, ascat_node *b )
{
 // check node index is ok

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

static void get_szr_node( unsigned char *x, int i, int swath, ascat_node *b )
{
 // check node index is ok

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

void ascat_file::get_node( int i, int swath, ascat_node *b )
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

void ascat_file::get_node( int i, int beam, ascat_szf_node *b )
{
 int julian1, julian2, time_diff;
 int ant, pos; 

 if( nn   != NODES_SZF ) FAIL;
 if( i    < 0          ) FAIL;
 if( i    >= NODES_SZF ) FAIL;
 
 if( beam < 0 || beam > 5 )
 {
   printf("ERROR: beam index is invalid in ascat_file::get_node %d\n",beam);
   FAIL;
 }
 pos = beam * 1024 + i * 4;

 get_time(   mdr, &b->tm, b->year, b->month, b->day, b->hour, b->minute, b->second );
 
 get_int(    mdr, 68 + beam*4, 1.0e-4,  &b->track );
 get_int(    mdr, 128 + pos,   1.0e-6,  &b->s0 );
 get_int(    mdr, 6272 + pos,  1.0e-6,  &b->t0 );
 get_int(    mdr, 12416 + pos, 1.0e-6,  &b->a0 );
 get_int(    mdr, 18560 + pos, 1.0e-6,  &b->lat );
 get_int(    mdr, 24704 + pos, 1.0e-6,  &b->lon );
 get_ushort( mdr, 30848 + pos, 1.0e-3,  &b->atht );
 get_uint(   mdr, 33920 + pos, 1.0e-10, &b->atls );
 get_byte(   mdr, 40064,                &b->fsyn );
 get_byte(   mdr, 40065,                &b->fref );
 get_byte(   mdr, 40066,                &b->forb );
 get_byte(   mdr, 40067,                &b->fgen1 );
 get_byte(   mdr, 40068 + i,            &b->fgen2 );

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
 if( b->a0  < 0   ) b->a0  += 360;
 
 // represent atmospheric height and loss in meters
 b->atht *= 1000;
 b->atls *= 1.0e-3;
 
}

