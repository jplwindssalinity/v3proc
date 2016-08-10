/////////////////////////////////////////
// header file for ascat level 1b reader
/////////////////////////////////////////

// data structure to hold
// szo & szr node data
// tm = epoch time in days
// lon,lat = long & lat in deg
// s0,s1,s2 = sigma0 in dB
// t0,t1,t2 = inc angle in deg
// a0,a1,a2 = az angle in deg
// kp0,kp1,kp2 = noise
// asc = ascending flag
// index = node number
// sat = satellite
// fuse0,fuse1,fuse2 = usable flags
// fsyn0, fsyn1, fsyn2 = synthetic data flags
// fsynq0, fsynq1, fsynq2 = synthetic data quality flags
// forb0, forb1, forb2 = orbit quality flags
// fsol0, fsol1, fsol2 = solar array flags
// ftel0, ftel1, ftel2 = telemetry flags
// fext0, fext1, fext2 = extrapolated fn flags
// fland0, fland1, fland2 = land flags


typedef struct
{
 int proc_maj, proc_min;
 int asc;
 int sat,orbit,swath,index;
 int year,month,day,hour,minute,second;
 double lon,lat;
 double tm,track,atht,atls;
 double s0,s1,s2;
 double t0,t1,t2;
 double a0,a1,a2;
 double kp0,kp1,kp2;
 int fkp0,fkp1,fkp2;
 int fuse0,fuse1,fuse2;
 double fsyn0,fsyn1,fsyn2;
 double fsynq0,fsynq1,fsynq2;
 double forb0,forb1,forb2;
 double fsol0,fsol1,fsol2;
 double ftel0,ftel1,ftel2;
 double fext0,fext1,fext2;
 double fland0,fland1,fland2;
}
AscatNode;


// data structure to hold
// szf node data
// tm = epoch time in days
// lon,lat = long & lat in deg
// s0 = sigma0 in dB
// t0 = inc angle in deg
// a0 = az angle in deg
// fuse0,fuse1,fuse2 = usable flags
// asc = ascending flag
// beam = antenna beam index
// index = node number
// sat = sat id
// fsyn = synthetic data flags
// fref = reference fn flags
// forb = orbit quality flags
// fgen1 = general flags 1
// fgen2 = general flags 2


typedef struct
{
 int proc_maj, proc_min;
 int sat, asc, orbit, beam, index;
 int year,month,day,hour,minute,second;
 double lon,lat;
 double tm,track,atht,atls;
 double s0,t0,a0; //sigma0, inc angle, azimuth angle
 int fsyn,fref,forb;
 int fgen1,fgen2;
}
AscatSZFNode;

typedef struct {
    int year, month, day, hour, minute, second;
    double track;
    int beam;
    double tm;
    double s0;
    double t0; // inc
    double a0; // cell azimuth
    double lon, lat;
    double land_frac;
    int fref1, fref2, fpl, fgen1, fgen2;
    int is_good, is_marginal, is_bad, is_asc, is_land;
} AscatSZFNodeNew;

// class to read ascat level 1b
// files and return node data
// fp = file pointer
// nn = number of nodes
// mdr = mdr read from file
// size = size of mdr
// id = subclass of mdr
// sat = satellite

class AscatFile
{
 private:
 FILE *fp;
 int size, id;
 int sat, orbit, nn;
 int s_v_year, s_v_month, s_v_day;     // state vector date
 int s_v_hour, s_v_minute, s_v_second; // state vector time
 int proc_maj, proc_min;

 unsigned char *mdr;



 public:
 int fmt_maj, fmt_min;
 
 double semi_major_axis;
 double eccentricity;
 double inclination;
 double perigee_argument;
 double ra_asc_node;
 double mean_anomoly;
 double sc_pos_x_asc_node;
 double sc_pos_y_asc_node;
 double sc_pos_z_asc_node;
 double sc_vel_x_asc_node;
 double sc_vel_y_asc_node;
 double sc_vel_z_asc_node;
 double asc_node_time;
 
 AscatFile();

 ~AscatFile();

 int open( const char* fname, int *nr, int *nn );
 // open a file for reading

 int read_mdr( int *isdummy );
 // read an mdr from the file

 void get_node( int i, int swath, AscatNode *b );
 // return szo & szr node data

 void get_node( int i, int beam, AscatSZFNode *b );
 void get_node_new( int i, AscatSZFNodeNew *b );
 // return szf node data

 void close( );
 // close the file

};

