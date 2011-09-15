//==============================================================//
// Copyright (C) 1997-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_grid_c[] =
    "@(#) $Id$";

#include <malloc.h>
#include "Grid.h"
#include "Meas.h"
#include "Sigma0.h"
#include <time.h>
#include <iostream>
#include "Misc.h"

using namespace std;

#define OFFSET_CK_DUP 52
//#define TIME_STEP 16.8 // second for moving 1 deg in lat roughly
#define TIME_STEP 8.4 // second for moving 0.5 deg in lat roughly
#define LAT_OFFSET 40 // to cover area with lat less than 1st meas record, or lat larger than last rec

//======//
// Grid //
//======//

Grid::Grid()
:   _crosstrack_res(0.0), _alongtrack_res(0.0), _crosstrack_size(0.0),
    _alongtrack_size(0.0), _crosstrack_bins(0), _alongtrack_bins(0),
    _start_time(0.0), _end_time(0.0), _max_vati(0), _ati_start(0),
    _ati_offset(0), _orbit_period(0.0), _grid(NULL),_writeIndices(false),
    _indfp(NULL),_plotMode(false), grid_starts_north_pole(0), grid_starts_south_pole(0)
{
    return;
}

Grid::~Grid()
{
    if (_grid != NULL)
    {
        free_array(_grid, 2, _crosstrack_bins, _alongtrack_bins);
    }
    if (_indfp!=NULL){
      fclose(_indfp);
      _indfp=NULL;
    }
    return;
}

//--------------------//
// Grid::SetStartTime //
//--------------------//

int
Grid::SetStartTime(
    double  start_time)
{
    _start_time = start_time;
    l2a.header.startTime = _start_time;

    if (ephemeris.GetPosition(_start_time,EPHEMERIS_INTERP_ORDER,
        &_start_position) == 0)
    {
        fprintf(stderr,
            "Grid::SetStartTime: Grid start time is out of ephemeris range\n");
        exit(1);
    }
    
    _start_position = _start_position.Nadir();
    
    // Added for flexible SOM (IJBIN) gridding; need starting ati index
    // start_time must be the least measurement time; not a time before the 
    // grid start (i.e. want grid start time == instrument start time).
    if( algorithm == SOM )
    {
      double at_lon, ct_lat;
      ephemeris.GetSOMCoordinates( _start_position, _start_time, 
        grid_starts_north_pole, &ct_lat, &at_lon );
      double r_n_at_bins      = 1624.0 * 25.0 / _alongtrack_res; 
      double atrack_bin_const = 360.0         / r_n_at_bins; 
      _start_vati_SOM         = (int)floor( at_lon / atrack_bin_const + 1.0 ) - 1;
      
      // Added option for grid to start at north pole, 9/15/2011 AGF
      // ISRO OS2 data needs to force this to zero, or else we mess up the l2b grid.
      if( _start_vati_SOM < 0 || grid_starts_north_pole || grid_starts_south_pole )
        _start_vati_SOM = 0;

      fprintf(stdout,"Grid::SetStartTime: at_lon, _start_vati_SOM: %12.6f %d\n",at_lon,_start_vati_SOM);
    }
    return(1);
}

// Grid::SetPlotMode

void Grid::SetPlotMode(){ _plotMode=true;}

//------------------//
// Grid CreateIndicesFile
//------------------//

void Grid::CreateIndicesFile(char* filename)
{
  _writeIndices=true;
  _indfp=fopen(filename,"w");
  if(_indfp==NULL){
    fprintf(stderr,"Grid cannot create Indices file %s\n",filename);
    exit(1);
  }
}

//------------------//
// Grid::SetEndTime //
//------------------//

int
Grid::SetEndTime(
    double  end_time)
{
    _end_time = end_time;
//    l2a.header.endTime = _end_time;

    //--------------------------------------------------------------//
    // Determine corresponding along track index (in virtual grid).
    //--------------------------------------------------------------//

    EarthPosition r;
    if (ephemeris.GetPosition(_end_time,EPHEMERIS_INTERP_ORDER,&r) == 0)
    {
        fprintf(stderr,
            "Grid::SetEndTime: Grid end time is out of ephemeris range\n");
        exit(1);
    }

    float ctd, atd;
    if (ephemeris.GetSubtrackCoordinates(r, _start_position, _start_time,
        _end_time, &ctd, &atd) == 0)
    {
        fprintf(stderr,
            "Grid::SetEndTime: No subtrack coordinates for grid end time\n");
        exit(1);
    }

    _max_vati = (int) (atd/_alongtrack_res);    // virtual along track index.

    if (_max_vati < 0)
    {
        fprintf(stderr,
            "Grid::SetEndTime: Grid ends before grid starts -> no output\n");
    }

    return(1);
}

//----------------//
// Grid::Allocate //
//----------------//

int
Grid::Allocate(
    double  crosstrack_res,
    double  alongtrack_res,
    double  crosstrack_size,
    double  alongtrack_size)
{
    _alongtrack_res = alongtrack_res;
    _crosstrack_res = crosstrack_res;
    _alongtrack_size = alongtrack_size;
    _crosstrack_size = crosstrack_size;
    _alongtrack_bins = (int) (_alongtrack_size / _alongtrack_res);
    _crosstrack_bins = (int) (_crosstrack_size / _crosstrack_res);

    // Put data into L2A object for use in header.
    l2a.header.crossTrackResolution = _crosstrack_res;
    l2a.header.alongTrackResolution = _alongtrack_res;
    l2a.header.crossTrackBins = _crosstrack_bins;
    l2a.header.alongTrackBins = _alongtrack_bins;
    l2a.header.zeroIndex = (int) (_crosstrack_size/2.0/_crosstrack_res + 0.5);

    //
    // Set circular buffer tracking parameters to the beginning of the
    // virtual grid.
    //

    _ati_start = 0;
    _ati_offset = 0;

    _grid = (OffsetListList**)make_array(sizeof(OffsetListList), 2,
        _crosstrack_bins, _alongtrack_bins);
    if (_grid == NULL)
        return(0);

    // Make an empty OffsetListList object,
    // and use to initialize each grid element
    OffsetListList empty_list;
    for (int i=0; i < _crosstrack_bins; i++)
    {
        for (int j=0; j < _alongtrack_bins; j++)
        {
            _grid[i][j] = empty_list;
        }
    }

    return(1);
}

//-----------//
// Grid::Add //
//-----------//
// Add the measurement to the grid using an offset list.
// meas = pointer to the Meas object to grid
// meas_time = time at which the measurement was made
// spot_id = unique integer for each measurement spot - used for compositing.
// do_composite = 1 if slices should be composited, 0 if not.

int
Grid::Add(
    Meas*   meas,
    double  meas_time,
    long    spot_id,
    int     do_composite)
{
    //----------------------------------//
    // calculate the subtrack distances //
    //----------------------------------//
	
	float ctd, atd;
	int cti0, vati0;
	int numWVCs=0;
	
	
	if( algorithm == SOM )
	{
      int    ijbin_ati,   ijbin_cti;
      double ijbin_r_ati, ijbin_r_cti;
      double ijbin_atlon, ijbin_ctlat;
      
      ephemeris.GetSOMCoordinates( meas->centroid, meas_time, 
        grid_starts_north_pole, &ijbin_ctlat, &ijbin_atlon );

      // # of along-track bins for 360 of along-track longitude.
      double r_n_at_bins = 1624.0 * 25.0 / _alongtrack_res; 
      
      // Bin into equiangular bins, using the following constants.
      double atrack_bin_const = 360.0 / r_n_at_bins;                   // converts from at_lon to ati
      double xtrack_bin_const = 180.0*_crosstrack_res / (r1_earth*pi); // converts from ct_lat to cti
      int    j_offset         = (int)(_crosstrack_bins / 2);

      // Construct the floating-point along-track and cross-track indicies.
      ijbin_r_ati = ijbin_atlon / atrack_bin_const - double(_start_vati_SOM);
      ijbin_r_cti = ijbin_ctlat / xtrack_bin_const;
      
      // Construct the integer indicies.
      ijbin_ati   =  (int)floor( ijbin_r_ati + 1.0 );
      ijbin_cti   = -(int)floor( ijbin_r_cti ) + j_offset;
      
      ijbin_r_cti = -ijbin_r_cti + j_offset;
      
      // Create cross-track, along-track "distances" for rest of Grid::Add
      // created such that the logic used below will give the right cti, ati 
      // for the ijbin algorithm.  Need to set this way so that we can re-use
      // the existing OVERLAP code below.
      ctd   = _crosstrack_res * ( ijbin_r_cti - 0.5 ) - 0.5 * _crosstrack_size;
      atd   = ijbin_r_ati * _alongtrack_res;
      
      // Subtract 1 for C 0-based array indexing.
      cti0  = ijbin_cti - 1;
      vati0 = ijbin_ati - 1;
      
      // For debugging output
      //printf("at_lon, ct_lat, r_ati, vati0, r_cti, cti0: %12.6f %12.6f %12.6f %6d %12.6f %6d\n",
      //        ijbin_atlon, ijbin_ctlat, ijbin_r_ati, vati0, ijbin_r_cti, cti0);
      //double alt, lon, lat;
      //meas->centroid.GetAltLonGDLat( &alt, &lon, &lat );
      //printf("%12.6f %12.6f %6d %6d %12.6f %12.6f\n", 
      //      lat*rtd, lon*rtd, ijbin_ati, ijbin_cti, ijbin_atlon, ijbin_ctlat );
	}
	else if( algorithm == SUBTRACK )
	{  
	
    static int firstTime = 1;
    static double refTime = 0.;
    double hll[3];
    EarthPosition pos;
    int nLatStep = 0;

    /* generate atd and ctd table */

    //cout << firstTime << endl;

    if (firstTime) {

      /* initialize gctd and gatd */

      //for (int ii=0; ii<181; ii++) {
      //  for (int jj=0; jj<361; jj++) {
      for (int ii=0; ii<361; ii++) {
        for (int jj=0; jj<721; jj++) {
          gctd[ii][jj] = -9999.;
          gatd[ii][jj] = 0.;
        }
      }

    }

    if (firstTime || (meas_time-refTime)>5700.) { // first time or another orbit

      printf("Meas Time and Old Ref Time: %18.12g, %18.12g\n", meas_time, refTime);
      cout << "Time difference: " <<  meas_time - refTime << endl;

      refTime = meas_time;

      OrbitState os;
      int sign;

      ephemeris.GetOrbitState(refTime, EPHEMERIS_INTERP_ORDER, &os);
      if (os.vsat.Get(2) > 0.) {  // orbit moving up
        sign = 1;
      } else { // orbit moving down
        sign = -1;
      }

      // add LAT_OFFSET for covering meas with lat less than 1st rec,
      // or lat larger than last rec, now it is 60, means 15 deg for each side

      if (lat_end_time-refTime<6000.) { // 6000 sec is about the period of orbit
        nLatStep = int((lat_end_time - refTime)/TIME_STEP)+LAT_OFFSET;
      } else {
        nLatStep = int(6000./TIME_STEP);
      }

      if (nLatStep>720) nLatStep = 720;

      cout << "Number of lat steps: " << nLatStep << endl;

      os.rsat.GetAltLonGDLat(hll,hll+1,hll+2);
      cout << "At ref time, S/C alt, long, lat: " << hll[0] << " " << hll[1]*rtd << " " << hll[2]*rtd << endl;

      float lat, lon;
      int lonIdx, latIdx;
      int refLon, refLat;

      //refLon = (int)(hll[1]*rtd);
      //refLat = (int)(hll[2]*rtd)-20*sign; // -20 for 1st meas record might not have the lowest lat

      refLat = (int)(hll[2]*rtd)-LAT_OFFSET*sign/2/2; // -15 for 1st meas record might not have the lowest lat

      for (int ii=0; ii<nLatStep; ii++) { //lat loop

        double time = refTime + (ii-LAT_OFFSET/2)*TIME_STEP; // time increment for change of 0.5 deg in lat
        lat = refLat + sign*ii*0.5;

        // find refLon
        ephemeris.GetOrbitState(time, EPHEMERIS_INTERP_ORDER, &os);
        os.rsat.GetAltLonGDLat(hll,hll+1,hll+2);
        refLon = (int)(hll[1]*rtd);

        /* for moving up */

        if (lat>=90. && lat<270.) {
          lat = 180 - lat;
        }
        if (lat>=270.) lat = lat - 360;

        /* for moving down */

        if (lat<=-90. && lat>-270.) {
          lat = -180 - lat;
        }
        if (lat<=-270.) lat = lat + 360;

        //printf("Time: %30.26g\n", time);
        //cout << "Time: " << time << endl;
        //cout << "Lat: " << lat << endl;
        //cout << "refLon: " << refLon << endl;

        int plus_flag = 0;
        int minus_flag = 0;

        if (lat==90.) {

          latIdx = (int)(2*lat);
          pos.SetAltLonGDLat(0.,0.,89.999/rtd);
          ephemeris.GetSubtrackCoordinates(pos, _start_position,
          _start_time, time, &gctd[latIdx+180][0], &gatd[latIdx+180][0]);

          //cout << lat << " " << latIdx << " " << gctd[latIdx+180][0] << " " << gatd[latIdx+180][0] << endl;

          for (int ll=1; ll<=720; ll++) {
            gctd[latIdx+180][ll] = gctd[latIdx+180][0];
            gatd[latIdx+180][ll] = gatd[latIdx+180][0];
          }

        } else if (lat==-90.) {

          latIdx = (int)(2*lat);
          pos.SetAltLonGDLat(0.,0.,-89.999/rtd);
          ephemeris.GetSubtrackCoordinates(pos, _start_position,
          _start_time, time, &gctd[latIdx+180][0], &gatd[latIdx+180][0]);

          //cout << lat << " " << latIdx << " " << gctd[latIdx+180][0] << " " << gatd[latIdx+180][0] << endl;

          for (int ll=1; ll<=720; ll++) {
            gctd[latIdx+180][ll] = gctd[latIdx+180][0];
            gatd[latIdx+180][ll] = gatd[latIdx+180][0];
          }

        } else {

          latIdx = (int)(2*lat);

          for (int jj=0; jj<360; jj++) { //lon loop

            lon = refLon + jj*0.5;
            if (lon>=360.) lon -= 360.;

            lonIdx = (int)(2*lon);

            pos.SetAltLonGDLat(0.,double(lon)/rtd,double(lat)/rtd);
            ephemeris.GetSubtrackCoordinates(pos, _start_position,
            _start_time, time, &gctd[latIdx+180][lonIdx], &gatd[latIdx+180][lonIdx]);

            //cout << lon << " " << lonIdx << " " << gctd[latIdx+180][lonIdx] << " " << gatd[latIdx+180][lonIdx] << endl;

            if (fabs(gctd[latIdx+180][lonIdx]) > 1500.) plus_flag = 1;

            lon = refLon - jj*0.5;
            if (lon<0.) lon += 360.;

            lonIdx = (int)(2*lon);

            pos.SetAltLonGDLat(0.,double(lon)/rtd,double(lat)/rtd);
            ephemeris.GetSubtrackCoordinates(pos, _start_position,
            _start_time, time, &gctd[latIdx+180][lonIdx], &gatd[latIdx+180][lonIdx]);

            //cout << lon << " " << lonIdx << " " << gctd[latIdx+180][lonIdx] << " " << gatd[latIdx+180][lonIdx] << endl;

            if (fabs(gctd[latIdx+180][lonIdx]) > 1500.) minus_flag = 1;

            if (plus_flag && minus_flag) {
              //cout << "ref lon: " << refLon << endl;
              //cout << "counts : " << jj << endl;
              break;
            }

          }

        } // lon loop
      } // lat loop

      for (int ii=0; ii<361; ii++) {
        gctd[ii][720] = gctd[ii][0];
        gatd[ii][720] = gatd[ii][0];
      }

      firstTime = 0;

    }

    float measLon, measLat, lonFact, latFact;
    int lonIdx1, lonIdx2, latIdx1, latIdx2;
    double measHLL[3];

    
    meas->centroid.GetAltLonGDLat(measHLL,measHLL+1,measHLL+2); // get alt, lon and lat

    measLon = measHLL[1]*rtd;
    measLat = measHLL[2]*rtd;


    // Measurement Sanity check to avoid unnecessary computations for bad measurements
    if(isnan(meas->value) || (meas->range_width > 1000) || meas->azimuth_width > 1000){
                cerr << "Warning Grid::Add Meas (meas_time =)" << meas_time << " could not be added to Grid." << endl;
                cerr << "      Value = " << meas->value << " azim width = " << meas->azimuth_width << "range width = " << meas->range_width << endl;
                cerr << "      Lat = " << measLat << "  Long = " << measLon << endl;
    }
    // BWS bug fix July 30 2010 so taht negative latitudes are propoerly interpolated
    // originally they were extrapolated from neighboring cells, so it should not matter much
    // still assumes positive longitude
    // PUT in floor functions to do the fix.
    lonIdx1 = (int)(measLon*2.);
    lonIdx2 = (int)(measLon*2.)+1;
    latIdx1 = (int)floor(measLat*2.);
    latIdx2 = (int)floor(measLat*2.)+1;
    lonFact = measLon*2. - lonIdx1;
    latFact = measLat*2. - latIdx1;

    //cout << measLon << endl;
    //cout << measLat << endl;
    //cout << lonIdx1 << endl;
    //cout << lonIdx2 << endl;
    //cout << latIdx1 << endl;
    //cout << latIdx2 << endl;
    //cout << lonFact << endl;
    //cout << latFact << endl;

    //cout << gctd[latIdx1+180][lonIdx1] << " " << gatd[latIdx1+180][lonIdx1] << endl;
    //cout << gctd[latIdx2+180][lonIdx1] << " " << gatd[latIdx2+180][lonIdx1] << endl;
    //cout << gctd[latIdx1+180][lonIdx2] << " " << gatd[latIdx1+180][lonIdx2] << endl;
    //cout << gctd[latIdx2+180][lonIdx2] << " " << gatd[latIdx2+180][lonIdx2] << endl;

    if (gctd[latIdx1+180][lonIdx1]==-9999. ||
        gctd[latIdx2+180][lonIdx1]==-9999. ||
        gctd[latIdx1+180][lonIdx2]==-9999. ||
        gctd[latIdx2+180][lonIdx2]==-9999.) {
      printf("Meas Time: %30.26g\n", meas_time);
      printf("Ref Time: %30.26g\n", refTime);
      cerr << "latIdx1, lonIdx1: " << latIdx1 << " " << lonIdx1 << endl;
      cerr << "Grid ctd is not large enough!!" << endl;
      cout << gctd[latIdx1+180][lonIdx1] << " " << gatd[latIdx1+180][lonIdx1] << endl;
      cout << gctd[latIdx2+180][lonIdx1] << " " << gatd[latIdx2+180][lonIdx1] << endl;
      cout << gctd[latIdx1+180][lonIdx2] << " " << gatd[latIdx1+180][lonIdx2] << endl;
      cout << gctd[latIdx2+180][lonIdx2] << " " << gatd[latIdx2+180][lonIdx2] << endl;
      exit(1);
    }

    ctd = (1.-lonFact)*(1.-latFact)*gctd[latIdx1+180][lonIdx1]
         + (1.-lonFact)*latFact*gctd[latIdx2+180][lonIdx1]
         + lonFact*(1.-latFact)*gctd[latIdx1+180][lonIdx2]
         + lonFact*latFact*gctd[latIdx2+180][lonIdx2];

    atd = (1.-lonFact)*(1.-latFact)*gatd[latIdx1+180][lonIdx1]
         + (1.-lonFact)*latFact*gatd[latIdx2+180][lonIdx1]
         + lonFact*(1.-latFact)*gatd[latIdx1+180][lonIdx2]
         + lonFact*latFact*gatd[latIdx2+180][lonIdx2];

    //cout << "ss along, cross: " << atd << " " << ctd << endl;

    //if (ephemeris.GetSubtrackCoordinates(meas->centroid, _start_position,
    //    _start_time, meas_time, &ctd, &atd) == 0)
    //{
    //    return(0);  // Couldn't find a grid position, so dump this measurement.
    //}

    //cout << "from along, cross: " << atd << " " << ctd << endl;

    //
    // Compute grid indices, noting that the cross track grid starts on the left
    // side at cti = 0.
    //

    cti0 = (int) ((ctd + _crosstrack_size/2.0)/_crosstrack_res + 0.5);
    vati0 = (int) (atd/_alongtrack_res);    // virtual along track index.
	
	} //---End of conditional on which gridding method to use [ if( algorithm == SOM ) ]
	else
	{
	  fprintf(stderr,"Grid::Add: unknown REGRID_ALGORITHM choice, quitting!\n");
	  exit(1);
	}
	
    int ctimin,ctimax,vatimin,vatimax;
    float ctdmin=ctd,ctdmax=ctd,atdmin=atd,atdmax=atd;
    // compute bounds on measurement if we are using overlap
    float corn_ctd[4],corn_atd[4];
    float cosang,sinang,rwid,awid;


    if(method==OVERLAP){
      cosang=cos(meas->scanAngle);
      sinang=sin(meas->scanAngle);

      rwid=meas->range_width;
      awid=meas->azimuth_width;

      // zero width cases occur when the INTEGRATION_STEP was set coarsely during the simulation.
      if(rwid==0) rwid=0.01; 
      if(awid==0) awid=0.01;
 
      float r_c=0.5*sinang*rwid;
      float r_a=0.5*cosang*rwid;
      float az_c=-0.5*cosang*awid;
      float az_a=0.5*sinang*awid;
      
      int sgn[2]={-1,1};
      int off=0;
      for(int r=0;r<2;r++){
	for(int az=0;az<2;az++){
	  corn_ctd[off]=ctd+r_c*sgn[r]+az_c*sgn[az];
	  corn_atd[off]=atd+r_a*sgn[r]+az_a*sgn[az];
	  if(corn_atd[off]>atdmax)atdmax=corn_atd[off];
	  if(corn_atd[off]<atdmin)atdmin=corn_atd[off];
	  if(corn_ctd[off]>ctdmax)ctdmax=corn_ctd[off];
	  if(corn_ctd[off]<ctdmin)ctdmin=corn_ctd[off];
	    off++;
	}
      }
      if(_plotMode){
	printf("# For XMGRACE rwid=%g awid=%g scanang=%g\n",
	       rwid,awid,meas->scanAngle*rtd);
        printf("%g %g\n&\n",ctd,atd);
        printf("%g %g\n",corn_ctd[0],corn_atd[0]);
        printf("%g %g\n",corn_ctd[1],corn_atd[1]);
        printf("%g %g\n",corn_ctd[3],corn_atd[3]);
        printf("%g %g\n",corn_ctd[2],corn_atd[2]);
        printf("%g %g\n&\n",corn_ctd[0],corn_atd[0]);
        
      }
    ctimin = (int) ((ctdmin + _crosstrack_size/2.0)/_crosstrack_res + 0.5);
    vatimin = (int) (atdmin/_alongtrack_res);    // virtual along track index.
    ctimax = (int) ((ctdmax + _crosstrack_size/2.0)/_crosstrack_res + 0.5);
    vatimax = (int) (atdmax/_alongtrack_res);    // virtual along track index.
    
    //printf("ctimin,cti,ctimax,atimin,ati,atimax: %6d %6d %6d %6d %6d %6d\n",
    //       ctimin,cti0,ctimax,vatimin,vati0,vatimax);
    
    }
    else{
      ctimin=cti0;
      ctimax=cti0;
      vatimin=vati0;
      vatimax=vati0;
    }


    bool one_cell=(method!=OVERLAP) || (ctimin==ctimax && vatimin==vatimax);
    for(int vati=vatimin;vati<=vatimax;vati++){
     for(int cti=ctimin;cti<=ctimax;cti++){   

       // if OVERLAP method is used check to see if this WVC falls into meas
       // region (do the meas and the cell intersect)
       // skips trivial (meas smaller than WVC case)
       bool overlaps=false;
       if(!one_cell){
         // check if centroid of measurement in WVC case
	 if(cti0==cti && vati0==vati) overlaps=true;

         // check if center of WVC inside measurement case
         float wvc_ctd=cti*_crosstrack_res-0.5*_crosstrack_size;
         float wvc_atd=(vati+0.5)*_alongtrack_res;
         float dctd=wvc_ctd-ctd;
         float datd=wvc_atd-atd;

	 // compute cell boundaries ctd aand atd
	 float c0=wvc_ctd-0.5*_crosstrack_res*overlapFactor;
	 float c1=wvc_ctd+0.5*_crosstrack_res*overlapFactor;
	 float a0=wvc_atd-0.5*_alongtrack_res*overlapFactor;
	 float a1=wvc_atd+0.5*_alongtrack_res*overlapFactor;
         
	 if(_plotMode){
	   printf("%g %g\n",c0,a0);
	   printf("%g %g\n",c1,a0);
	   printf("%g %g\n",c1,a1);
	   printf("%g %g\n",c0,a1);
	   printf("%g %g\n&\n",c0,a0);
	 }
         float dr=fabs(cosang*datd+sinang*dctd);
         float da=fabs(-sinang*datd+cosang*dctd);
         if(dr<=0.5*rwid && da<=0.5*awid) overlaps=true;
	   
	 
         // check if any side of the measurement intersects a sides of
         // the
         // wvc
         //only necessary if cell dimensions overlap if measurement is
         // smaller than wvc in one dimension but not the other 
         
         if(MAX(rwid,awid)>MIN(_crosstrack_res,_alongtrack_res) &&
            MIN(rwid,awid)<MAX(_crosstrack_res,_alongtrack_res))
	   {
             // corner numbers for one end of each meas edge i0 and
             // the other end i1
	     int i0[4]={0,0,1,2};
             int i1[4]={1,2,3,3};

             //foreach edge of a measuremnt
             for(int j=0;j<4;j++){
	       // Does meas edge j overlap any of the wvc edges?
               // If any measurement edge overlaps a side of the wvc
               // then overlaps is true and flow drops out of loop.

               // Get short names for meas edge end coords
               float mc0=corn_ctd[i0[j]];
               float mc1=corn_ctd[i1[j]];
               float ma0=corn_atd[i0[j]];
               float ma1=corn_atd[i1[j]];

               // compute slopes
	       float dcda= (mc1-mc0)/(ma1-ma0);
               float dadc=1/dcda;

               // compute offsets
	       float coff=mc1-dcda*ma1;
	       float aoff=ma1-dadc*mc1;

 

               // check for overlap between meas edge j and -ati edge of wvc
               // 1) compute intersection point of the two lines
               // 2) Is it on the cell side line segment?
               // 3) Is it on the meas edge line segment?

	       float c=coff+a0*dcda;
               if(c>c0 && c<=c1){
		 if(ma0<ma1){
		   if(a0> ma0 && a0<= ma1){
		     overlaps=true;
                     break;
		   }
		 }
		 else  if(a0> ma1 && a0<= ma0){
		   overlaps=true;
		   break;
		 }
	       }

               // check for overlap between meas edge j and +ati edge of wvc
	       c=coff+a1*dcda;
               if(c>c0 && c<=c1){
		 if(ma0<ma1){
		   if(a1> ma0 && a1<= ma1){
		     overlaps=true;
                     break;
		   }
		 }
		 else  if(a1> ma1 && a1<= ma0){
		   overlaps=true;
		   break;
		 }
	       }
		 
               // check for overlap between meas edge j and -cti edge of wvc
	       float a=aoff+c0*dadc;
               if(a>a0 && a<=a1){
		 if(mc0<mc1){
		   if(c0> mc0 && c0<= mc1){
		     overlaps=true;
                     break;
		   }
		 }
		 else  if(c0> mc1 && c0<= mc0){
		   overlaps=true;
		   break;
		 }
	       }

               // check for overlap between meas edge j and +cti edge of wvc
	       a=aoff+c1*dadc;
               if(a>a0 && a<=a1){
		 if(mc0<mc1){
		   if(c1> mc0 && c1<= mc1){
		     overlaps=true;
                     break;
		   }
		 }
		 else  if(c1> mc1 && c1<= mc0){
		   overlaps=true;
		   break;
		 }
	       }
	     }	     
	   }

         if(!overlaps) continue;
	 if(_plotMode){          
	   printf("%g %g\n",c0,a0);
	   printf("%g %g\n",c1,a1);
	   printf("%g %g\n",c1,a0);
	   printf("%g %g\n&\n",c0,a1);
	 }
       }
       if ((cti >= _crosstrack_bins) || (cti < 0))
	 {
	   fprintf(stderr, "Grid::Add: crosstrack index = %d out of range\n",
		   cti);
	   return(0);
	 }

       //
       // Negative vati means the point falls before the defined grid start.
       // For this special case, Add() returns success, but dumps the measurement.
       // If vati falls inside the defined grid, but in a portion that has been
       // already output, then an error message is generated (see below).
       //
       numWVCs++;
       if (vati < 0)
	 {
	   //        delete meas;
	   return(1);
	 }
       
       if (vati > _max_vati)
	 {    // beyond end of grid, so do nothing.
	   return(1);
	 }

       //
       // Note that reverse shifting is not implemented.
       // Thus, if vati falls before the earliest row in memory, it is considered
       // out of range instead of trying to back up the buffer.  This limitation
       // is imposed by BufferedList.
       //

       if (vati < _ati_offset)
	 {
	   fprintf(stderr, "Grid::Add: alongtrack index = %d out of range\n",
		   vati);
	   return(0);
	 }
       
       //
       // Determine if the along track index is in memory or not.
       // If not, read and write rows until it is in range.
       //

       while (vati - _ati_offset >= _alongtrack_bins)
	 {
	   // vati is beyond latest row, so need to shift the grid buffer
	   if(!ShiftForward(do_composite,0)) return(0);
	 }
       
       //
       // Convert the along track index into the virtual buffer (vati) to an
       // index into the grid in memory (ati).
       //

       int ati = (vati - _ati_offset + _ati_start) % _alongtrack_bins;
       
       // convert the measurement to an offset
       
       off_t* offset = new off_t;
       *offset = meas->offset;

       OffsetList* offsetlist = _grid[cti][ati].GetHead();

       if (do_composite == 1)
	 {
	   // Composite slices that fall in the same grid location and spot.
	   // Scan for an offset list with the same spot id.
	   while (offsetlist != NULL)
	     {
	       if (offsetlist->spotId == spot_id)
		 {
		   break;
		 }
	       offsetlist = _grid[cti][ati].GetNext();
	     }
	   
	   if (offsetlist == NULL)
	     {
	       // Append a new offsetlist (with the new offset) for the new spot
	       offsetlist = new OffsetList;
	       offsetlist->Append(offset);
	       offsetlist->spotId = spot_id;
	       _grid[cti][ati].Append(offsetlist);
	     }
	   else
	     {
	       // Append the offset in the list with the matching spot id.
	       offsetlist->Append(offset);
	     }
	 }
       else
	 {
	   // Put all slices in the first (and only) offset list at this grid loc.
	   if (offsetlist == NULL)
	     {
	       // Need to create a sublist and attach to the grid square
	       offsetlist = new OffsetList;
	       if (offsetlist == NULL)
		 {
		   fprintf(stderr, "Grid::Add: error allocating memory\n");
		   exit(1);
		 }
	       _grid[cti][ati].Append(offsetlist);
	     }
	   offsetlist->Append(offset);
	 }
     } // end cti loop
    } // end vati loop

    if(_writeIndices){
      fprintf(_indfp,"%ld %d %d %g %g %d %d\n",spot_id,meas->startSliceIdx,numWVCs,atd,ctd,vati0,cti0);
    }
    //printf("%d %d %f %f\n",cti,vati,ctd,atd);
    if(_plotMode){
      fflush(stdout);
      exit(0);
    }
    return(1);
}

//
// Grid::ShiftForward
//
// Shift the grid of measurements by one alongtrack row.
// The earliest row is written out to the output file and its storage
// is reset to receive another row.  The circular buffer tracking indices
// are updated appropriately so that the newly freed row is mapped to
// the next along track index.
// If requested, slices are composited before output.
// The slices to be composited should all be together in a sublist of their
// grid location.
//
// Inputs:
//    do_composite = flag set to 1 if compositing is desired, 0 otherwise.

//#define MAX_MEAS_PER_BIN 5000
#define MAX_MEAS_PER_BIN 20000
int
Grid::ShiftForward(
    int  do_composite,
    int  forget_position )
{
    static int headerWritten = 0;

    //---------------------------//
    // remember the L1B location //
    //---------------------------//

    FILE* fp = l1b.GetInputFp();
    off_t offset = ftello(fp);
    if (offset == -1)
        return(0);

    MeasList spot_measList;
    
    char* buffer = NULL;
    
    if( !do_composite )
      buffer=(char*)malloc(sizeof(char)*MAX_MEAS_PER_BIN*meas_length);
      
    //----------------------------------------------
    // Write out the earliest row of measurement lists.
    //----------------------------------------------

    // Loop over cross track bins
    for (int i=0; i < _crosstrack_bins; i++)
    {

      if(!_plotMode){
        //----------------------------------------//
        // convert each offset list to a MeasList //
        //----------------------------------------//

        if (do_composite == 1)
        {
            l2a.frame.measList.FreeContents();
            spot_measList.FreeContents();

            for (OffsetList* offsetlist = _grid[i][_ati_start].GetHead();
                 offsetlist; offsetlist = _grid[i][_ati_start].GetNext())
            {
                // each sublist is composited before output
                offsetlist->MakeMeasList(fp, &spot_measList);
                Meas* meas = new Meas;
                if (! meas->Composite(&spot_measList))
                {
                    //fprintf(stderr, "Grid::ShiftForward: Error compositing\n");
                    delete meas;
                    //return(0);
                }
                else
                {
                  if (! l2a.frame.measList.Append(meas))
                  {
                      fprintf(stderr,
                          "Grid::ShiftForward: Error forming list for output\n");
                      delete meas;
                      return(0);
                  }
              }
              spot_measList.FreeContents();
            }

            //----------------------------------//
            // complete and write the L2A frame //
            //----------------------------------//

            l2a.frame.rev = 0;
            l2a.frame.cti = i;
            l2a.frame.ati = _ati_offset;
            if (l2a.frame.measList.GetHead() != NULL)
            {
                l2a.WriteDataRec();
            }

        }
        else
        {
            /* write l2a header in the first time */

            if (!headerWritten) {
              l2a.WriteHeader();
              headerWritten = 1;
            }

            float ss1, ss2, sa1, sa2;
            int ns1, ns2;

            OffsetList* offsetlist = _grid[i][_ati_start].GetHead();
            if (offsetlist != NULL)
            {
	        //char* buffer=(char*)malloc(sizeof(char)*MAX_MEAS_PER_BIN*meas_length);

 		int dd = 0;
                if (offsetlist->NodeCount()>MAX_MEAS_PER_BIN) {
                  cerr << "Error Number of measurements for a cell exceeds " <<
		    MAX_MEAS_PER_BIN << " !!" << endl;
                  cerr << "Cti = " << i << " Ati =" << _ati_start << endl;
                  exit(1);
                } 
                for (off_t* l1b_offset = offsetlist->GetHead(); l1b_offset;
                            l1b_offset = offsetlist->GetNext()) {

		  // check for valid offset and seek offset in file
		  if (fseeko(fp, *l1b_offset, SEEK_SET)==-1) {
		    return(0);
		  }

 		  fread(&buffer[dd], sizeof(char), meas_length, fp);

                  // debugging tool
                  //if(_ati_start==279 && i==29){
		  //  cout << i <<  " " << _ati_start << " " << meas_length << endl;
                  //  cout << l1b_offset << " " << *l1b_offset << " " << dd << endl;
		  //}
		  dd += meas_length;
                }  // end loop through offset list

 

                /* write out to l2a file */

                unsigned int rev = 0;

                fwrite((void *)&rev, sizeof(unsigned int), 1, l2a.GetOutputFp()); 
                fwrite((void *)&_ati_offset, sizeof(int), 1, l2a.GetOutputFp()); 
                fwrite((void *)&i, sizeof(int), 1, l2a.GetOutputFp()); 

                /* get the total count */


                int nm = dd/meas_length;

                fwrite((void *)&nm, sizeof(int), 1, l2a.GetOutputFp()); 
                fwrite(buffer, sizeof(char), nm*meas_length, l2a.GetOutputFp());
		//free(buffer);
	    } // end if offsetlist != NULL
	    
        } // end case composite == 0
      } // end if not _plotMode (Nothing is written to the L2A in plot mode
        
      //----------------------//
      // free the offset list //
      //----------------------//

      _grid[i][_ati_start].FreeContents();
    } // end cross track bin loop
    
    if( buffer != NULL ) free(buffer);
    
    // Update buffer indices.
    _ati_start = (_ati_start + 1) % _alongtrack_bins;
    _ati_offset++;

    //----------------------//
    // restore L1B location //
    //----------------------//
    if( !forget_position )
      if (fseeko(fp, offset, SEEK_SET) == -1)
        return(0);

    return(1);
}

//-------------//
// Grid::Flush //
//-------------//
// Write out all the grid rows in memory.

int
Grid::Flush(
    int  do_composite)
{
    for (int i = 0; i < _alongtrack_bins; i++)
    {
      if(!ShiftForward(do_composite,1)) return(0);
    }

    return(1);
}
