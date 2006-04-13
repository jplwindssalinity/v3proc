//==============================================================//
// Copyright (C) 1997-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_byuxtable_c[] =
    "@(#) $Id$";

#include "BYUXTable.h"
#include "Qscat.h"
#include "Sigma0.h"
#include "InstrumentGeom.h"
#include "CheckFrame.h"
#include "Topo.h"

//----------------------//
// BYUXTable::BYUXTable //
//----------------------//

BYUXTable::BYUXTable()
:   xnom(NULL), a(NULL), b(NULL), c(NULL), d(NULL), e(NULL), f(NULL),
    xnomEgg(NULL), aEgg(NULL), bEgg(NULL), cEgg(NULL), dEgg(NULL), eEgg(NULL),
    fEgg(NULL)
{
    _azimuthStepSize = two_pi / BYU_AZIMUTH_BINS;
    _numSlices = BYU_NUM_SCIENCE_SLICES + BYU_NUM_GUARD_SLICES_PER_SIDE * 2;
    return;
}

//---------------------//
// BYUXTable::Allocate //
//---------------------//
// Order is 3 or 5. This is such a ugly way to make these arrays!

int
BYUXTable::Allocate(
    int  order)
{
    //-----------------//
    // check the order //
    //-----------------//

    if (order != 3 && order != 5)
    {
        fprintf(stderr, "BYUXTable::Allocate: order must be 3 or 5, not %d\n",
            order);
        exit(1);
    }

    //---------------------//
    // allocate for slices //
    //---------------------//

    xnom = (float****)make_array(sizeof(float), 4, BYU_NUM_BEAMS, _numSlices,
        BYU_ORBIT_POSITION_BINS, BYU_AZIMUTH_BINS);
    if (xnom == NULL)
        return(0);

    a = (float****)make_array(sizeof(float), 4, BYU_NUM_BEAMS, _numSlices,
        BYU_ORBIT_POSITION_BINS, BYU_AZIMUTH_BINS);
    if (a == NULL)
        return(0);

    b = (float****)make_array(sizeof(float), 4, BYU_NUM_BEAMS, _numSlices,
        BYU_ORBIT_POSITION_BINS, BYU_AZIMUTH_BINS);
    if (b == NULL)
        return(0);

    c = (float****)make_array(sizeof(float), 4, BYU_NUM_BEAMS, _numSlices,
        BYU_ORBIT_POSITION_BINS, BYU_AZIMUTH_BINS);
    if (c == NULL)
        return(0);

    d = (float****)make_array(sizeof(float), 4, BYU_NUM_BEAMS, _numSlices,
        BYU_ORBIT_POSITION_BINS, BYU_AZIMUTH_BINS);
    if (d == NULL)
        return(0);

    if (order == 5)
    {
        e = (float****)make_array(sizeof(float), 4, BYU_NUM_BEAMS, _numSlices,
            BYU_ORBIT_POSITION_BINS, BYU_AZIMUTH_BINS);
        if (e == NULL)
            return(0);

        f = (float****)make_array(sizeof(float), 4, BYU_NUM_BEAMS, _numSlices,
            BYU_ORBIT_POSITION_BINS, BYU_AZIMUTH_BINS);
        if (f == NULL)
            return(0);
    }

    //-------------------//
    // allocate for eggs //
    //-------------------//

    xnomEgg = (float***)make_array(sizeof(float), 3, BYU_NUM_BEAMS,
        BYU_ORBIT_POSITION_BINS, BYU_AZIMUTH_BINS);
    if (xnomEgg == NULL)
        return(0);

    aEgg = (float***)make_array(sizeof(float), 3, BYU_NUM_BEAMS,
        BYU_ORBIT_POSITION_BINS, BYU_AZIMUTH_BINS);
    if (aEgg == NULL)
        return(0);

    bEgg = (float***)make_array(sizeof(float), 3, BYU_NUM_BEAMS,
        BYU_ORBIT_POSITION_BINS, BYU_AZIMUTH_BINS);
    if (bEgg == NULL)
        return(0);

    cEgg = (float***)make_array(sizeof(float), 3, BYU_NUM_BEAMS,
        BYU_ORBIT_POSITION_BINS, BYU_AZIMUTH_BINS);
    if (cEgg == NULL)
        return(0);

    dEgg = (float***)make_array(sizeof(float), 3, BYU_NUM_BEAMS,
        BYU_ORBIT_POSITION_BINS, BYU_AZIMUTH_BINS);
    if (dEgg==NULL)
        return(0);

    if (order == 5)
    {
        eEgg = (float***)make_array(sizeof(float), 3, BYU_NUM_BEAMS,
            BYU_ORBIT_POSITION_BINS, BYU_AZIMUTH_BINS);
        if (eEgg == NULL)
            return(0);

        fEgg = (float***)make_array(sizeof(float), 3, BYU_NUM_BEAMS,
            BYU_ORBIT_POSITION_BINS, BYU_AZIMUTH_BINS);
        if (fEgg == NULL)
            return(0);
    }

    return(1);
}

//-----------------------//
// BYUXTable::Deallocate //
//-----------------------//

int
BYUXTable::Deallocate()
{
    if (xnom != NULL) {
        free_array((void*)xnom, 4, BYU_NUM_BEAMS, _numSlices,
            BYU_ORBIT_POSITION_BINS, BYU_AZIMUTH_BINS);
        xnom = NULL;
    }
    if (a != NULL) {
        free_array((void*)a, 4, BYU_NUM_BEAMS, _numSlices,
            BYU_ORBIT_POSITION_BINS, BYU_AZIMUTH_BINS);
        a = NULL;
    }
    if (b != NULL) {
        free_array((void*)b, 4, BYU_NUM_BEAMS, _numSlices,
            BYU_ORBIT_POSITION_BINS, BYU_AZIMUTH_BINS);
        b = NULL;
    }
    if (c != NULL) {
        free_array((void*)c, 4, BYU_NUM_BEAMS, _numSlices,
            BYU_ORBIT_POSITION_BINS, BYU_AZIMUTH_BINS);
        c = NULL;
    }
    if (d != NULL) {
        free_array((void*)d, 4, BYU_NUM_BEAMS, _numSlices,
            BYU_ORBIT_POSITION_BINS, BYU_AZIMUTH_BINS);
        d = NULL;
    }
    if (e != NULL) {
        free_array((void*)e, 4, BYU_NUM_BEAMS, _numSlices,
            BYU_ORBIT_POSITION_BINS, BYU_AZIMUTH_BINS);
        e = NULL;
    }
    if (f != NULL) {
        free_array((void*)f, 4, BYU_NUM_BEAMS, _numSlices,
            BYU_ORBIT_POSITION_BINS, BYU_AZIMUTH_BINS);
        f = NULL;
    }
    if (xnomEgg != NULL) {
        free_array((void*)xnomEgg, 3, BYU_NUM_BEAMS, BYU_ORBIT_POSITION_BINS,
            BYU_AZIMUTH_BINS);
        xnomEgg = NULL;
    }
    if (aEgg != NULL) {
        free_array((void*)aEgg, 3, BYU_NUM_BEAMS, BYU_ORBIT_POSITION_BINS,
            BYU_AZIMUTH_BINS);
        aEgg = NULL;
    }
    if (bEgg != NULL) {
        free_array((void*)bEgg, 3, BYU_NUM_BEAMS, BYU_ORBIT_POSITION_BINS,
            BYU_AZIMUTH_BINS);
        bEgg = NULL;
    }
    if (cEgg != NULL) {
        free_array((void*)cEgg, 3, BYU_NUM_BEAMS, BYU_ORBIT_POSITION_BINS,
            BYU_AZIMUTH_BINS);
        cEgg = NULL;
    }
    if (dEgg != NULL) {
        free_array((void*)dEgg, 3, BYU_NUM_BEAMS, BYU_ORBIT_POSITION_BINS,
            BYU_AZIMUTH_BINS);
        dEgg = NULL;
    }
    if (eEgg != NULL) {
        free_array((void*)eEgg, 3, BYU_NUM_BEAMS, BYU_ORBIT_POSITION_BINS,
            BYU_AZIMUTH_BINS);
        eEgg = NULL;
    }
    if (fEgg != NULL) {
        free_array((void*)fEgg, 3, BYU_NUM_BEAMS, BYU_ORBIT_POSITION_BINS,
            BYU_AZIMUTH_BINS);
        fEgg = NULL;
    }
    return(1);
}

//-----------------------//
// BYUXTable::~BYUXTable //
//-----------------------//

BYUXTable::~BYUXTable()
{
    Deallocate();
    return;
}

//-----------------//
// BYUXTable::Read //
//-----------------//
int
BYUXTable::Read(
    const char*  ibeam_file,
    const char*  obeam_file)
{
    // if filenames are the same it must be a Ground System format file
    if(strcmp(ibeam_file,obeam_file)==0){
      fprintf(stderr, "Warning BYUXTable: reading GS format.\n");
      fprintf(stderr, "Warning (cont): Also assuming resolution mode 3 QuikSCAT nominal for 2005\n");
      return(ReadGS(ibeam_file,3));
    }
    FILE* ifp[2];
    ifp[0] = fopen(ibeam_file, "r");
    if (ifp[0] == NULL)
    {
        fprintf(stderr, "BYUXTable::Read: error opening X Factor file %s\n",
            ibeam_file);
        return(0);
    }
    ifp[1] = fopen(obeam_file, "r");
    if (ifp[1] == NULL)
    {
        fprintf(stderr, "BYUXTable::Read: error opening X Factor file %s\n",
            obeam_file);
        return(0);
    }

    //---------------------//
    // determine the order //
    //---------------------//

    int count0 = 0;
    do {
        float number;
        if (fscanf(ifp[0], "%*[ \t]%g", &number) != 1)
            break;
        count0++;
    } while (1);
    rewind(ifp[0]);

    int count1 = 0;
    do {
        float number;
        if (fscanf(ifp[1], "%*[ \t]%g", &number) != 1)
            break;
        count1++;
    } while (1);
    rewind(ifp[1]);

    if (count0 != count1) {
        fprintf(stderr, "BYUXTable::Read: mismatched X factor tables\n");
        exit(1);
    }

    int order = 0;
    if (count0 == 130) {
        order = 3;
    } else if (count0 == 156) {
        printf("BYUXTable::Read: fifth order tables read\n");
        order = 5;
    } else {
        fprintf(stderr,
            "BYUXTable::Read: error determine order of X factor tables\n");
        exit(1);
    }

    //---------------//
    // Create Arrays //
    //---------------//

    if (! Allocate(order))
        return(0);

    //--------------------//
    // Loop through beams //
    //--------------------//

    for (int bm = 0; bm < 2; bm++)
    {
        //--------------------------//
        // Loop through Orbit Times //
        //--------------------------//

        for (int o = 0; o < BYU_ORBIT_POSITION_BINS; o++)
        {
            //-----------------------//
            // Loop through Azimuths //
            //-----------------------//

            for (int ah = 0; ah < BYU_AZIMUTH_BINS; ah++)
            {
                char string[20];

                // Sanity Check on Orbit and azimuth;
                float orbit, azimuth;
                fscanf(ifp[bm], "%s", string);
                orbit = atof(string);
                fscanf(ifp[bm], "%s", string);
                azimuth = atof(string);
                if (fabs(o * BYU_TIME_INTERVAL_BETWEEN_STEPS - orbit) > 0.01
                    || fabs(ah * _azimuthStepSize - azimuth * dtr) > 0.0001)
                {
                    fprintf(stderr,
                        "BYUXTable::Read Error Sanity Check failed.\n");
                    return(0);
                }

                // Read in slice Xnom,A,B,C,D values
                for (int s = 0; s < _numSlices; s++)
                {
                    // Xnom
                    fscanf(ifp[bm], "%s", string);
                    xnom[bm][s][o][ah] = atof(string);

                    // G skipped for now.
                    fscanf(ifp[bm], "%s", string);

                    // A
                    fscanf(ifp[bm], "%s", string);
                    a[bm][s][o][ah] = atof(string);

                    // B
                    fscanf(ifp[bm], "%s", string);
                    b[bm][s][o][ah] = atof(string);

                    // C
                    fscanf(ifp[bm], "%s", string);
                    c[bm][s][o][ah] = atof(string);

                    // D
                    fscanf(ifp[bm], "%s", string);
                    d[bm][s][o][ah] = atof(string);

                    if (order == 5) {
                        // E
                        fscanf(ifp[bm], "%s", string);
                        e[bm][s][o][ah] = atof(string);

                        // F
                        fscanf(ifp[bm], "%s", string);
                        f[bm][s][o][ah] = atof(string);
                    }

                    // Skip A_az for now
                    fscanf(ifp[bm], "%s", string);

                    // Skip B_az for now
                    fscanf(ifp[bm], "%s", string);

                    // Skip A_el for now
                    fscanf(ifp[bm], "%s", string);

                    // Skip B_el for now
                    fscanf(ifp[bm], "%s", string);
                }

                //-----------------------------------//
                // Read Egg Xnom A B C, and D values //
                //-----------------------------------//

                // Xnom
                fscanf(ifp[bm], "%s", string);
                xnomEgg[bm][o][ah] = atof(string);

                // A
                fscanf(ifp[bm], "%s", string);
                aEgg[bm][o][ah] = atof(string);

                // B
                fscanf(ifp[bm], "%s", string);
                bEgg[bm][o][ah] = atof(string);

                // C
                fscanf(ifp[bm], "%s", string);
                cEgg[bm][o][ah] = atof(string);

                // D
                fscanf(ifp[bm], "%s", string);
                dEgg[bm][o][ah] = atof(string);

                if (order == 5) {
                    // E
                    fscanf(ifp[bm], "%s", string);
                    eEgg[bm][o][ah] = atof(string);

                    // F
                    fscanf(ifp[bm], "%s", string);
                    fEgg[bm][o][ah] = atof(string);
                }

                //-----------------------------------------//
                // Skip over Doppler, Range, and S for now //
                //-----------------------------------------//

                fscanf(ifp[bm], "%s", string);
                fscanf(ifp[bm], "%s", string);
                fscanf(ifp[bm], "%s", string);
            }
        }
    }
    return(1);
}

//-----------------//
// BYUXTable::ReadGS //
//-----------------//
int
BYUXTable::ReadGS(
    const char*  filename, int mode_idx)
{
    FILE* ifp;
    ifp = fopen(filename, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "BYUXTable::Read: error opening X Factor file %s\n",
           filename);
        return(0);
    }

    //------------------------------//
    // for GS format the order is 5 //
    //------------------------------//

 

    int order = 5;


    //---------------//
    // Create Arrays //
    //---------------//

    if (! Allocate(order))
        return(0);


    //------------------------------//
    // skip fortran header          //
    // and best8 threshold table    //
    //------------------------------//
    int num_modes=8;
    int byteskip=4+2*2*num_modes*sizeof(float);
    fseek(ifp,byteskip,SEEK_SET);

    //------ read in Xnom (df=0 xfactor) ---/

    // skip to correct mode
    byteskip=mode_idx*BYU_ORBIT_POSITION_BINS*BYU_AZIMUTH_BINS*BYU_NUM_BEAMS*(BYU_NUM_SCIENCE_SLICES+1)*sizeof(float);
    fseek(ifp,byteskip,SEEK_CUR);


    for(int o=0;o<BYU_ORBIT_POSITION_BINS;o++){
      for(int ah=0;ah<BYU_AZIMUTH_BINS;ah++){
	for(int bm=0;bm<BYU_NUM_BEAMS;bm++){
	  for(int s=0;s<BYU_NUM_SCIENCE_SLICES+1;s++){
	    float val=0;
	    if(fread(&val,sizeof(float),1,ifp)!=1){
	      fprintf(stderr,"BYUXTable::ReadGS read failed for filename %s\n",
		      filename);
	      exit(1);
	    }
            // egg
	    if(s==BYU_NUM_SCIENCE_SLICES){
	      xnomEgg[bm][o][ah]=val;
	    }
            //slices
            else{
              int sidx=s+BYU_NUM_GUARD_SLICES_PER_SIDE;
	      xnom[bm][sidx][o][ah]=val;
	    }
	  }
	}
      }
    }

      
    // skip the rest of the modes
    byteskip=(num_modes-mode_idx-1)*BYU_ORBIT_POSITION_BINS*BYU_AZIMUTH_BINS*BYU_NUM_BEAMS*(BYU_NUM_SCIENCE_SLICES+1)*sizeof(float);
    fseek(ifp,byteskip,SEEK_CUR);




    //------ read in A,B,C,D,E,F ---/
    unsigned int numcoeffs=6;
    
    // skip to correct mode
    byteskip=mode_idx*BYU_ORBIT_POSITION_BINS*BYU_AZIMUTH_BINS*BYU_NUM_BEAMS*(BYU_NUM_SCIENCE_SLICES+1)*numcoeffs*sizeof(float);
    fseek(ifp,byteskip,SEEK_CUR);


    for(int o=0;o<BYU_ORBIT_POSITION_BINS;o++){
      for(int ah=0;ah<BYU_AZIMUTH_BINS;ah++){
	for(int bm=0;bm<BYU_NUM_BEAMS;bm++){
	  for(int s=0;s<BYU_NUM_SCIENCE_SLICES+1;s++){
	    float val[6]; 
	    if(fread(&val[0],sizeof(float),numcoeffs,ifp)!=numcoeffs){
	      fprintf(stderr,"BYUXTable::ReadGS read failed for filename %s\n",
		      filename);
	      exit(1);
	    }
            // egg
	    if(s==BYU_NUM_SCIENCE_SLICES){
	      aEgg[bm][o][ah]=val[0];
	      bEgg[bm][o][ah]=val[1];
	      cEgg[bm][o][ah]=val[2];
	      dEgg[bm][o][ah]=val[3];
	      eEgg[bm][o][ah]=val[4];
	      fEgg[bm][o][ah]=val[5];
	    }
            //slices
            else{
              int sidx=s+BYU_NUM_GUARD_SLICES_PER_SIDE;
	      a[bm][sidx][o][ah]=val[0];
	      b[bm][sidx][o][ah]=val[1];
	      c[bm][sidx][o][ah]=val[2];
	      d[bm][sidx][o][ah]=val[3];
	      e[bm][sidx][o][ah]=val[4];
	      f[bm][sidx][o][ah]=val[5];
	    }
	  }
	}
      }
    }

      

    // skip the rest of the modes
    byteskip=(num_modes-mode_idx-1)*BYU_ORBIT_POSITION_BINS*BYU_AZIMUTH_BINS*BYU_NUM_BEAMS*(BYU_NUM_SCIENCE_SLICES+1)*numcoeffs*sizeof(float);
    fseek(ifp,byteskip,SEEK_CUR);

    return(1);
}

//----------------------//
// BYUXTable::GetXTotal //
//----------------------//

float
BYUXTable::GetXTotal(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    Meas*        meas,
    Topo*        topo,
    Stable*      stable,
    CheckFrame*  cf)
{
    // true Es_cal based on true PtGr
    float Es_cal = true_Es_cal(qscat);
    float X = GetXTotal(spacecraft, qscat, meas, Es_cal, topo, stable, cf);
    return(X);
}

//----------------------//
// BYUXTable::GetXTotal //
//----------------------//

float
BYUXTable::GetXTotal(
    Spacecraft*   spacecraft,
    Qscat*        qscat,
    Meas*         meas,
    float         Es_cal,
    Topo*         topo,
    Stable*       stable,
    CheckFrame*   cf)
{
    float X = GetX(spacecraft, qscat, meas, topo, stable, cf);

    //--------------------------------------------------//
    // Compute the Xcal portion of the overall X factor //
    // Reference: IOM-3347-98-019                       //
    //--------------------------------------------------//

    double Xcal;
    radar_Xcal(qscat, Es_cal, &Xcal);

    return(X * Xcal);    // Total X
}

//-----------------//
// BYUXTable::GetX //
//-----------------//

float
BYUXTable::GetX(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    Meas*        meas,
    Topo*        topo,
    Stable*      stable,
    CheckFrame*  cf)
{
    float delta_freq = GetDeltaFreq(spacecraft, qscat, topo, stable, cf);
    float orbit_position = qscat->cds.OrbitFraction();
    int beam_number = qscat->cds.currentBeamIdx;
    float azim = qscat->sas.antenna.groundImpactAzimuthAngle;
    int sliceno = meas->startSliceIdx;
    return(GetX(beam_number, azim, orbit_position, sliceno, delta_freq));
}

//-------------------------//
// BYUXTable::GetDeltaFreq //
//-------------------------//

float
BYUXTable::GetDeltaFreq(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    Topo*        topo,
    Stable*      stable,
    CheckFrame*  cf)
{
    //-----------//
    // predigest //
    //-----------//

    Antenna* antenna = &(qscat->sas.antenna);
    OrbitState* orbit_state = &(spacecraft->orbitState);
    Attitude* attitude = &(spacecraft->attitude);

  //-------------------------------------------------------------------------//
  // The nominal_boresight and antenna_frame_to_gc coordinate switch below
  // work together to realize the equations in IOM-3347-98-54.  In particular,
  // look at eqns (6) and (7).  The nominal_boresight is put at an elevation
  // angle matching the reference vector, and the azimuth is set to:
  // groundImpactAzimuth - txCenterAzimuth + referenceAzimuth.  The
  // coordinate switch is formed at the txCenterAzimuth.  Thus, the
  // txCenterAzimuth is the origin.  The look vector has to be referenced
  // to this origin, and that is why the txCenterAzimuth is subtracted
  // from the nominal_boresight azimuth, leaving groundImpactAzimuth + refAzim
  // as needed in eqns (6) and (7).
  //-------------------------------------------------------------------------//

    double look;
    double azim;
    if (! GetBYUBoresight(spacecraft, qscat, &look, &azim))
    {
        fprintf(stderr, "BYUXTable::GetDeltaFreq failed\n");
        fprintf(stderr, "Probably means earth_intercept not found\n");
        exit(1);
    }

    Vector3 nominal_boresight;
    nominal_boresight.SphericalSet(1.0, look, azim);

    //--------------------------------//
    // generate the coordinate switch //
    //--------------------------------//

    CoordinateSwitch antenna_frame_to_gc = AntennaFrameToGC(orbit_state,
        attitude, antenna, antenna->txCenterAzimuthAngle);

    //---------------------------//
    // determine delta frequency //
    //---------------------------//

    QscatTargetInfo qti;
    if (! qscat->TargetInfo(&antenna_frame_to_gc, spacecraft,
        nominal_boresight, &qti))
    {
        fprintf(stderr, "BYUXTable::GetDeltaFreq failed\n");
        fprintf(stderr, "Probably means earth_intercept not found\n");
        exit(1);
    }

    //----------------------------------//
    // apply the topographic correction //
    //----------------------------------//

    if (topo && stable)
    {
        int beam_idx = qscat->cds.currentBeamIdx;
        float orbit_fraction = qscat->cds.OrbitFraction();
        float antenna_azimuth = qscat->sas.antenna.groundImpactAzimuthAngle;
        double alt, lon_d, lat_d;
        if (! qti.rTarget.GetAltLonGCLat(&alt, &lon_d, &lat_d))
        {
            fprintf(stderr, "BYUXTable::GetDeltaFreq failed\n");
            fprintf(stderr, "    Can't determine target lat/lon\n");
        }
        float longitude = (float)lon_d;
        float latitude = (float)lat_d;
        float tdf = topo_delta_f(topo, stable, beam_idx, orbit_fraction,
            antenna_azimuth, longitude, latitude);
        qti.basebandFreq += tdf;
    }

    //-----------------------------//
    // log check data if requested //
    //-----------------------------//

    if (cf)
    {
        cf->XdopplerFreq = qti.dopplerFreq;
        cf->XroundTripTime = qti.roundTripTime;
        cf->deltaFreq = qti.basebandFreq;
    }

    return(qti.basebandFreq);
}

//-----------------//
// BYUXTable::GetX //
//-----------------//

float
BYUXTable::GetX(
    int    beam_number,
    float  azimuth_angle,
    float  orbit_position,
    int    slice_number,
    float  delta_freq)
{
    int absolute_slice_number;

    // Convert from relative to absolute slice number
    if (! rel_to_abs_idx(slice_number, _numSlices, &absolute_slice_number))
    {
        fprintf(stderr, "BYUXTable::GetX Bad Slice Number\n");
        exit(1);
    }

    // Convert from orbit position to nominal orbit time
    float orbit_time = BYU_NOMINAL_ORBIT_PERIOD * orbit_position;

    // Interpolate tables
    float X = Interpolate(xnom[beam_number][absolute_slice_number], orbit_time,
        azimuth_angle);
    float A = Interpolate(a[beam_number][absolute_slice_number], orbit_time,
        azimuth_angle);
    float B = Interpolate(b[beam_number][absolute_slice_number], orbit_time,
        azimuth_angle);
    float C = Interpolate(c[beam_number][absolute_slice_number], orbit_time,
        azimuth_angle);
    float D = Interpolate(d[beam_number][absolute_slice_number], orbit_time,
        azimuth_angle);

    float delta_bin = delta_freq / FFT_BIN_SIZE;

    if (e != NULL) {
        // 5th order
        float E = Interpolate(e[beam_number][absolute_slice_number],
            orbit_time, azimuth_angle);
        float F = Interpolate(f[beam_number][absolute_slice_number],
            orbit_time, azimuth_angle);
        X += ((((F * delta_bin + E) * delta_bin + D) * delta_bin + C)
            * delta_bin + B) * delta_bin + A;
    } else {
        // 3rd order
        X += ((D * delta_bin + C) * delta_bin + B) * delta_bin + A;
    }

/*
    // Frequency Compensate
    X += A + B*delta_bin + C*delta_bin*delta_bin +
        D*delta_bin*delta_bin*delta_bin;
*/
    X = pow(10.0, 0.1 * X);

    return(X);
}

//--------------------//
// BYUXTable::GetXegg //
//--------------------//

float
BYUXTable::GetXegg(
    int    beam_number,
    float  azimuth_angle,
    float  orbit_position,
    float  delta_freq)
{
    // Convert from orbit position to nominal orbit time
    float orbit_time = BYU_NOMINAL_ORBIT_PERIOD * orbit_position;

    // Interpolate tables
    float X = Interpolate(xnomEgg[beam_number], orbit_time, azimuth_angle);
    float A = Interpolate(aEgg[beam_number], orbit_time, azimuth_angle);
    float B = Interpolate(bEgg[beam_number], orbit_time, azimuth_angle);
    float C = Interpolate(cEgg[beam_number], orbit_time, azimuth_angle);
    float D = Interpolate(dEgg[beam_number], orbit_time, azimuth_angle);

    // Frequency Compensate
    float delta_bin = delta_freq / FFT_BIN_SIZE;
    if (eEgg != NULL) {
        // 5th order
        float E = Interpolate(eEgg[beam_number], orbit_time, azimuth_angle);
        float F = Interpolate(fEgg[beam_number], orbit_time, azimuth_angle);
        X += ((((F * delta_bin + E) * delta_bin + D) * delta_bin + C)
            * delta_bin + B) * delta_bin + A;
    } else {
        // 3rd order
        X += ((D * delta_bin + C) * delta_bin + B) * delta_bin + A;
    }

/*
    X += A + B*delta_bin + C*delta_bin*delta_bin
        + D*delta_bin*delta_bin*delta_bin;
*/

    X = pow(10.0, 0.1 * X);
    return(X);
}

//------------------------//
// BYUXTable::Interpolate //
//------------------------//

float
BYUXTable::Interpolate(
    float**  table,
    float    orbit_time,
    float    azimuth_angle)
{
    // calculate floating point index
    float fazi = azimuth_angle / _azimuthStepSize;
    float ftime = orbit_time   / BYU_TIME_INTERVAL_BETWEEN_STEPS;

    // calculate indices (don't worry about range)
    int a1 = (int)fazi;
    int t1 = (int)ftime;
    int a2 = a1 + 1;
    int t2 = t1+ 1;

    // calculate coefficients
    float ca1, ca2, ct1, ct2;
    ca1 = (float)a2 - fazi;
    ca2 = fazi - (float)a1;
    if (t1 < BYU_ORBIT_POSITION_BINS - 1) {
        ct1 = (float)t2 - ftime;
        ct2 = ftime - (float)t1;
    } else {
        float end_t = BYU_NOMINAL_ORBIT_PERIOD
            / BYU_TIME_INTERVAL_BETWEEN_STEPS;
        ct1 = (end_t - ftime) / (end_t - (float)t1);
        ct2 = (ftime - (float)t1) / (end_t - (float)t1);
    }

    // wrap indices into range
    t2 %= BYU_ORBIT_POSITION_BINS;
    a2 %= BYU_AZIMUTH_BINS;

    float retval = ct1 * ca1 * table[t1][a1] + ct1 * ca2 * table[t1][a2]
        + ct2 * ca1 * table[t2][a1] + ct2 * ca2 * table[t2][a2];
    return(retval);
}

//-----------------//
// GetBYUBoresight //
//-----------------//

int
GetBYUBoresight(
    Spacecraft*  spacecraft,
    Qscat*       qscat,
    double*      look,
    double*      azim)
{
    //-----------------------------------//
    // compute the BYU nominal boresight //
    //-----------------------------------//

    *azim = qscat->sas.antenna.groundImpactAzimuthAngle -
        qscat->sas.antenna.txCenterAzimuthAngle;

    if (qscat->cds.currentBeamIdx == 0) {
        *look = qscat->cds.xRefLook[0];
        *azim += qscat->cds.xRefAzim[0];
    } else {
        *look = qscat->cds.xRefLook[1];
        *azim += qscat->cds.xRefAzim[1];
    }
    while (*azim < -pi) *azim += two_pi;
    while (*azim > pi) *azim -= two_pi;

    return(1);
}
