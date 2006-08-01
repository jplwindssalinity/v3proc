//==============================================================//
// Copyright (C) 1997-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_grid_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <malloc.h>
#include "Grid.h"
#include "Meas.h"
#include "Sigma0.h"
#include <time.h>
#include <iostream.h>

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
    _ati_offset(0), _orbit_period(0.0), _grid(NULL)
{
    return;
}

Grid::~Grid()
{
    if (_grid != NULL)
    {
        free_array(_grid, 2, _crosstrack_bins, _alongtrack_bins);
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
    return(1);
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

    float ctd, atd;
    float measLon, measLat, lonFact, latFact;
    int lonIdx1, lonIdx2, latIdx1, latIdx2;
    double measHLL[3];

    meas->centroid.GetAltLonGDLat(measHLL,measHLL+1,measHLL+2); // get alt, lon and lat

    measLon = measHLL[1]*rtd;
    measLat = measHLL[2]*rtd;
    lonIdx1 = (int)(measLon*2.);
    lonIdx2 = (int)(measLon*2.)+1;
    latIdx1 = (int)(measLat*2.);
    latIdx2 = (int)(measLat*2.)+1;
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

    int cti = (int) ((ctd + _crosstrack_size/2.0)/_crosstrack_res + 0.5);
    int vati = (int) (atd/_alongtrack_res);    // virtual along track index.

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
        ShiftForward(do_composite);
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

    //printf("%d %d %f %f\n",cti,vati,ctd,atd);

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

int
Grid::ShiftForward(
    int  do_composite)
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

    // Write out the earliest row of measurement lists.
    for (int i=0; i < _crosstrack_bins; i++)
    {
        //----------------------------------------//
        // convert each offset list to a MeasList //
        //----------------------------------------//

        if (do_composite == 1)
        {
            l2a.frame.measList.FreeContents();

            for (OffsetList* offsetlist = _grid[i][_ati_start].GetHead();
                offsetlist; offsetlist = _grid[i][_ati_start].GetNext())
            {
                // each sublist is composited before output
                offsetlist->MakeMeasList(fp, &spot_measList);
                Meas* meas = new Meas;
                if (! meas->Composite(&spot_measList))
                {
                    fprintf(stderr, "Grid::ShiftForward: Error compositing\n");
                    delete meas;
                    return(0);
                }
                spot_measList.FreeContents();
                if (! l2a.frame.measList.Append(meas))
                {
                    fprintf(stderr,
                        "Grid::ShiftForward: Error forming list for output\n");
                    delete meas;
                    return(0);
                }
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

            char buffer[160000];
            float ss1, ss2, sa1, sa2;
            int ns1, ns2;

            OffsetList* offsetlist = _grid[i][_ati_start].GetHead();
            if (offsetlist != NULL)
            {

                // remove duplicate meas in offsetlist

                Meas* meas = new Meas;
                Meas* nextmeas = new Meas;

                off_t* l1b_nextoffset = new off_t;
                int dup_flag = 0;
                int dd = 0;

                for (off_t* l1b_offset = offsetlist->GetHead(); l1b_offset;
                            l1b_offset = offsetlist->GetNext()) {

                  if (dup_flag==1) {
                    l1b_offset = offsetlist->GetPrev();
                  }

                  //if (fseeko(fp, *l1b_offset+OFFSET_CK_DUP, SEEK_SET)==-1) {
                  //  return(0);
                  //}
                  if (fseeko(fp, *l1b_offset, SEEK_SET)==-1) {
                    return(0);
                  }

                  //fread(&ss1, sizeof(float), 1, fp);
                  //fread(&ns1, sizeof(int), 1, fp);
                  //fread(&sa1, sizeof(float), 1, fp);
                  meas->Read(fp);

                  l1b_nextoffset = offsetlist->GetNext();

                  //cout << "offset1: " << *l1b_offset << endl;

                  if (l1b_nextoffset != NULL) {

                    //cout << "offset2: " << *l1b_nextoffset << endl;

                    //if (fseeko(fp, *l1b_nextoffset+OFFSET_CK_DUP, SEEK_SET)==-1) {
                    //  return(0);
                    //}
                    if (fseeko(fp, *l1b_nextoffset, SEEK_SET)==-1) {
                      return(0);
                    }

                    //fread(&ss2, sizeof(float), 1, fp);
                    //fread(&ns2, sizeof(int), 1, fp);
                    //fread(&sa2, sizeof(float), 1, fp);
                    nextmeas->Read(fp);

                    //cout << "ss: " << meas->startSliceIdx << " " << nextmeas->startSliceIdx << endl;
                    //cout << "sa: " << meas->scanAngle << " " << nextmeas->scanAngle << endl;
                    //if (ss1==ss2 && sa1==sa2) {
                    //if (meas->startSliceIdx==nextmeas->startSliceIdx
                    //    && meas->scanAngle==nextmeas->scanAngle) {
                    if (meas->startSliceIdx==nextmeas->startSliceIdx
                        && meas->scanAngle==nextmeas->scanAngle
                        && meas->measType==nextmeas->measType) {
                      dup_flag = 1;
                      /* need to remove the element pointed by l1b_offset */
                      l1b_offset = offsetlist->GetPrev();
                      l1b_offset = offsetlist->RemoveCurrent();
                      delete l1b_offset;
                      l1b_offset = offsetlist->GetCurrent();
                      //cout << "dup: " << *l1b_offset << endl;
                      //cout << "type: " << meas->measType << " " << nextmeas->measType << endl;
                    } else {
                      dup_flag = 0;
                      /* put pointer backward */
                      l1b_offset = offsetlist->GetPrev();
                      //cout << "not dup: " << *l1b_offset << " " << dd << endl;
                      /* write record to buffer */
                      fseeko(fp, *l1b_offset, SEEK_SET);
                      fread(&buffer[dd], sizeof(char), meas_length, fp);
                      dd += meas_length;
                    }

                  } // l1b_nextoffset not NULL

                } // remove duplicate

                delete meas;
                delete nextmeas;

                /* write out final record */

                l1b_nextoffset = offsetlist->GetTail();
                fseeko(fp, *l1b_nextoffset, SEEK_SET);
                fread(&buffer[dd], sizeof(char), meas_length, fp);
                dd += meas_length;

                /* write out to l2a file */

                unsigned int rev = 0;

                fwrite((void *)&rev, sizeof(unsigned int), 1, l2a.GetOutputFp()); 
                fwrite((void *)&_ati_offset, sizeof(int), 1, l2a.GetOutputFp()); 
                fwrite((void *)&i, sizeof(int), 1, l2a.GetOutputFp()); 

                /* get the total count */

                //int nm = offsetlist->NodeCount();
                int nm = dd/meas_length;

                //cout << "# rec: " << nm << " " << dd << endl;

                if (nm>2000) {
                  cerr << "Number of measurements for a cell exceeds 2000!!" << endl;
                  exit(1);
                } 

                fwrite((void *)&nm, sizeof(int), 1, l2a.GetOutputFp()); 

                //int cc = 0;

                //for (off_t* l1b_offset = offsetlist->GetHead(); l1b_offset;
                //            l1b_offset = offsetlist->GetNext()) {
                //  if (fseeko(fp, *l1b_offset, SEEK_SET)==-1) {
                //    return(0);
                //  }   
                //  fread(&buffer[cc], sizeof(char), meas_length, fp);
                //  //fwrite(buffer, sizeof(char), meas_length, l2a.GetOutputFp());
                //  cc += meas_length;
                //}

                fwrite(buffer, sizeof(char), nm*meas_length, l2a.GetOutputFp());

                delete l1b_nextoffset;

            } // offset != NULL

        } // composite = 0

        //----------------------//
        // free the offset list //
        //----------------------//

        _grid[i][_ati_start].FreeContents();
    }

    // Update buffer indices.
    _ati_start = (_ati_start + 1) % _alongtrack_bins;
    _ati_offset++;

    //----------------------//
    // restore L1B location //
    //----------------------//

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
        ShiftForward(do_composite);
    }

    return(1);
}
