//==============================================================//
// Copyright (C) 1997-2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_l1atol1b_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include "OvwmL1AToL1B.h"
#include "CheckFrame.h"
#include "InstrumentGeom.h"
#include "OvwmSigma0.h"
#include "Ovwm.h"

//==============//
// OvwmL1AToL1B //
//==============//

OvwmL1AToL1B::OvwmL1AToL1B()
:   pulseCount(0),
    outputSigma0ToStdout(0), sliceGainThreshold(0.0), processMaxSlices(0),
    simVs1BCheckfile(NULL), Esn_echo_cal(0.0), Esn_noise_cal(0.0),
    En_echo_load(0.0), En_noise_load(0.0)
{
    return;
}

OvwmL1AToL1B::~OvwmL1AToL1B()
{
  if(_ptr_array!=NULL){
    free_array(_ptr_array,2,_max_int_range_bins);
    _ptr_array=NULL;
  }

  return;
}

// HACK should compute the defined terms
// from OVWM parameters or even from PTR table
#define MAX_RANGE_WIDTH 2.0
#define MAX_AZIMUTH_WIDTH 12.0

int OvwmL1AToL1B::AllocateIntermediateArrays()
{
  _max_int_range_bins=(int)ceil(integrationRangeWidthFactor*MAX_RANGE_WIDTH/
    integrationStepSize);
  _max_int_azim_bins=(int)ceil(integrationAzimuthWidthFactor*MAX_AZIMUTH_WIDTH/
    integrationStepSize);
  _ptr_array=(float**)make_array(sizeof(float),2,_max_int_range_bins,
                                 _max_int_azim_bins);
  if(_ptr_array==NULL){
    fprintf(stderr,"Error: OvwmSim unable to allocate _ptr_array\n");
    exit(1);
  }

  return 1;
}

//-----------------------//
// OvwmL1AToL1B::Convert //
//-----------------------//

int
OvwmL1AToL1B::Convert(
    OvwmL1A*     l1a,
    Spacecraft*  spacecraft,
    Ovwm*        ovwm,
    Ephemeris*   ephemeris,
    Topo*        topo,
    Stable*      stable,
    L1B*         l1b)
{
    //--------------//
    // unpack frame //
    //--------------//

    l1a->frame.Unpack(l1a->buffer);

    //-------------------//
    // set up check data //
    //-------------------//

    CheckFrame cf;
    if (simVs1BCheckfile)
    {
        if (! cf.Allocate(l1a->frame.maxMeasPerSpot))
        {
            fprintf(stderr,"Error allocating a CheckFrame\n");
            return(0);
        }
    }

    //-------------------//
    // set up spacecraft //
    //-------------------//

    float roll = l1a->frame.attitude.GetRoll();
    float pitch = l1a->frame.attitude.GetPitch();
    float yaw = l1a->frame.attitude.GetYaw();
    spacecraft->attitude.SetRPY(roll, pitch, yaw);

    //-------------------//
    // set up instrument //
    //-------------------//

    ovwm->cds.time = l1a->frame.time;

    //------------------------------------//
    // Extract and prepare cal pulse data //
    // Note 8 bit shift for loopback's    //
    //------------------------------------//

    Esn_echo_cal = l1a->frame.loopbackSpots[0];
    En_echo_load = l1a->frame.loadSpots[0];

    Esn_noise_cal = l1a->frame.loopbackNoise;
    En_noise_load = l1a->frame.loadNoise;

    cout << Esn_echo_cal << endl;
    cout << En_echo_load << endl;
    cout << Esn_noise_cal << endl;
    cout << En_noise_load << endl;

    //----------------------------------------------------------//
    // Estimate cal (loopback) signal and noise energies.       //
    // Kpr noise shows up in the cal signal energy.             //
    // Note that these are spot quantities (not slices).        //
    // If there isn't any cal pulse data, the preceeding values //
    // will be used.                                            //
    //----------------------------------------------------------//

    float beta = ovwm->ses.rxGainNoise / ovwm->ses.rxGainEcho;

    float Es_cal,En_cal;
    if (! Er_to_Es(beta, Esn_echo_cal, Esn_echo_cal, Esn_noise_cal,
        En_echo_load, En_noise_load, 1, &Es_cal, &En_cal))
    {
        return(0);
    }

    cout << Es_cal << endl;
    cout << En_cal << endl;

    // find out PtGr for ksig used in calXfactor

    float ptgr;

    ptgr = Es_cal/(ovwm->ses.transmitPathLoss / ovwm->ses.calibrationBias /
                   ovwm->ses.loopbackLoss / ovwm->ses.loopbackLossRatio *
                   ovwm->ses.txPulseWidth); 

    cout << "PtGr: " << ptgr << endl;

    //----------------------------//
    // ...free residual MeasSpots //
    //----------------------------//

    l1b->frame.spotList.FreeContents();

    //-----------//
    // predigest //
    //-----------//

    OrbitState* orbit_state = &(spacecraft->orbitState);
    Antenna* antenna = &(ovwm->sas.antenna);
    OvwmL1AFrame* frame = &(l1a->frame);

    //------------------//
    // for each spot... //
    //------------------//

    for (int spot_idx = 0; spot_idx < frame->spotsPerFrame; spot_idx++)
    {
        if (simVs1BCheckfile)
        {
          // cf.idx needs to start out zeroed for each spot
          cf.Initialize();
        }

        // determine the spot time
        // update cds time for this spot, it will be used in LocatePixels

        ovwm->cds.time = frame->spotTime[spot_idx];
        double time = frame->spotTime[spot_idx];

        // determine beam and beam index
        int beam_idx = frame->spotBeamIdx[spot_idx];
        ovwm->cds.currentBeamIdx = beam_idx;

        // determine starting slice index
        int base_slice_idx = spot_idx * frame->maxMeasPerSpot;

        //-------------------//
        // set up spacecraft //
        //-------------------//

/*** for OVWM, don't add Tp ***/
        //if (! ephemeris->GetOrbitState(time+0.5*ovwm->ses.txPulseWidth,
        if (! ephemeris->GetOrbitState(time, EPHEMERIS_INTERP_ORDER, orbit_state))
        {
            return(0);
        }

        //----------------//
        // set orbit step //
        //----------------//

        //unsigned int orbit_step = frame->orbitStep;
        //if (frame->priOfOrbitStepChange != 255 &&
        //    spot_idx < frame->priOfOrbitStepChange)
        //{
        //    if (orbit_step == 0)
        //        orbit_step = ORBIT_STEPS - 1;
        //    else
        //        orbit_step--;
        //}
        //qscat->cds.orbitStep = orbit_step;

        //-------------//
        // set antenna //
        //-------------//

        //unsigned short held_encoder = *(frame->antennaPosition + spot_idx);
        //qscat->cds.heldEncoder = held_encoder;
        //qscat->SetEncoderAzimuth(held_encoder, 1);
        //qscat->SetOtherAzimuths(spacecraft);

        //----------------------------//
        // range and Doppler tracking //
        //----------------------------//

//        qscat->sas.SetAzimuthWithEncoder(held_encoder);
//        qscat->SetAntennaToTxCenter(1);

        //SetDelayAndFrequency(spacecraft, ovwm);

/*** for OVWM, we have scan angle from L1A file ***/
/*** set tx freq with CmdTxDopplerEu with 0.0 ***/
/*** so hope we don't need to use the following function ***/
        //SetOrbitStepDelayAndFrequency(spacecraft, ovwm);

        ovwm->ses.CmdTxDopplerEu(0.0);

        antenna->txCenterAzimuthAngle = frame->spotScanAngle[spot_idx];
        cout << "scan angle: " << antenna->txCenterAzimuthAngle << endl;

        if (outputSigma0ToStdout)
            printf("%g ", antenna->txCenterAzimuthAngle * rtd);

        //---------------------------//
        // create a measurement spot //
        //---------------------------//

        MeasSpot* meas_spot = new MeasSpot();
        meas_spot->time = time;
        printf("Spot Time: %12.6f\n", time);

        float orbit_time = fmod(time-ovwm->cds.eqxTime, 6060.);

        cout << "orbit time: " << orbit_time << endl;

        //----------------------------------------//
        // Create slice measurements for the spot //
        //----------------------------------------//

        if (! ovwm->MakePixels(meas_spot))
            return(0);

        //----------------------------------------------------------//
        // Extract and load slice energy measurements for this spot //
        //----------------------------------------------------------//

        cout << "meas count: " << l1a->frame.dataCountSpots[spot_idx] << endl;

        float Esn_echo = 0.0;

        Meas* meas = meas_spot->GetHead();

/*** use maxMeasPerSpot close to real measurement situation ***/
/*** but it requires no removal in ovwm_sim ***/
        //for (int i=0; i < l1a->frame.dataCountSpots[spot_idx]; i++)
        for (int i=0; i < l1a->frame.maxMeasPerSpot; i++)
        {
          meas->value = l1a->frame.science[base_slice_idx + i];
          if (meas->value < 0.0)
          {
            fprintf(stderr,
              "L1AToL1B: Warning, found negative Esn in spot %d\n",spot_idx);
          }
          // Sum up ALL the signal+noise measurements
          Esn_echo += meas->value;

          meas = meas_spot->GetNext();
        }

        //-----------------------------------------------------------------//
        // Extract the spot noise measurement which applies to all slices. //
        //-----------------------------------------------------------------//

        float Esn_noise = l1a->frame.spotNoise[spot_idx];

        cout << "Total energy in signal channel: " << Esn_echo << endl;
        cout << "Noise channel: " << Esn_noise << endl;

        //---------------------//
        // locate measurements //
        //---------------------//

        ovwm->LocatePixels(spacecraft, meas_spot);

        //if (l1a->frame.maxMeasPerSpot <= 1)
        //{
        //    if (! ovwm->LocateSpot(spacecraft, meas_spot))
        //    {
        //        return(0);
        //    }
        //}
        //else
        //{
        //    if (! ovwm->LocateSliceCentroids(spacecraft, meas_spot,
        //        sliceGainThreshold, processMaxSlices))
        //    {
        //        return(0);
        //    }
        //}

        //--------------------//
        // Set the land flags //
        //--------------------//

        for (Meas* meas = meas_spot->GetHead(); meas;
            meas = meas_spot->GetNext())
        {
          double alt,lat,lon;
          if (! meas->centroid.GetAltLonGDLat(&alt, &lon, &lat))
              return(0);

          //cout << "meas cent (llh): " << alt << " " << lon*rtd << " " << lat*rtd << endl;
          // Compute Land Flag
          meas->landFlag = landMap.IsLand(lon, lat);
        }

        //----------------------------------------//
        // generate the reverse coordinate switch //
        //----------------------------------------//
        // duplicate work: forward transform already calc'd in Locate*

        CoordinateSwitch antenna_frame_to_gc =
            AntennaFrameToGC(orbit_state, &(spacecraft->attitude),
            antenna, antenna->txCenterAzimuthAngle);
        CoordinateSwitch gc_to_antenna =
            antenna_frame_to_gc.ReverseDirection();

        //int slice_count = qscat->ses.GetTotalSliceCount();

        //--------------------------------------//
        // determine measurement type from beam //
        //--------------------------------------//

        Beam* beam = ovwm->GetCurrentBeam();
        Meas::MeasTypeE meas_type = PolToMeasType(beam->polarization);

        double borelook, boreazim;

        // compute maximum gain

        beam->GetElectricalBoresight(&borelook,&boreazim);
        //cout << "look: " << borelook << endl;
        //cout << "azim: " << boreazim << endl;
        float maxgain;
        beam->GetPowerGain(borelook,boreazim,&maxgain);

        //cout << "peak gain:" << beam->peakGain << endl;
        //cout << "maxgain:" << maxgain << endl;

        // compute boresight position

        Vector3 boresight;
        boresight.SphericalSet(1.0,borelook,boreazim);

        OvwmTargetInfo oti;
        if(! ovwm->TargetInfo(&antenna_frame_to_gc,spacecraft,boresight,&oti)){
          fprintf(stderr,"Error:SetMeasurements cannot find boresight on surface\n");
          exit(1);
        }

        EarthPosition spot_centroid=oti.rTarget;

        // Compute range and azimuth coordinate switch

        Vector3 zvec=oti.rTarget.Normal(); // Z-axis is normal vector at boresight
        Vector3 yvec=zvec & oti.gcLook;            // az vector
        Vector3 xvec=yvec & zvec;                  // rng vector
        CoordinateSwitch gc_to_rangeazim(xvec,yvec,zvec);

        //cout << "GC to range az transformation:" << endl;
        //gc_to_rangeazim.Show();

        //---------------------------------------------------
        // Compute cross track/along track coordinate switch
        // It will be used to evaluate the ambiguities
        //-------------------------------------------------

        double r_a= r1_earth*1000.0;
        double r_e2=e2;
        SchToXyz sch(r_a,r_e2);//earth radius in meter and eccentricity square

        Vector3 position, velocity;

        position = orbit_state->rsat; // in km
        velocity = orbit_state->vsat; // in km/s

        position *= 1000.; // in m
        velocity *= 1000.; // in m/s

        //compute sc position 6 seconds before and after
        Vector3 position1, position2, delta_position;
        delta_position= velocity;
        delta_position *= 6.0;

        //compute two nearby sc position separated by 6*2 seconds
        position1 =position - delta_position;//6 seconds before
        position2 =position + delta_position;//6 seconds after

        //compute sc lat lon
        Vector3 r_llh,r_llh1, r_llh2;
        xyz_to_llh(r_a,r_e2,position,  r_llh);
        xyz_to_llh(r_a,r_e2,position1, r_llh1);
        xyz_to_llh(r_a,r_e2,position2, r_llh2);

        //compute heading
        double r_heading;
        geo_hdg(r_a,r_e2,r_llh1(0), r_llh1(1),r_llh2(0),r_llh2(1),r_heading);

        //set peg point
        sch.SetPegPoint(r_llh(0), r_llh(1), r_heading);

        //now we can convert surface location into sch
        //call the following two functions will do the job
        // sch.xyz_to_sch(r_xyz,r_sch)
        // or sch.sch_to_xyz(r_sch,r_xyz)

        //need boresight xyz in meter scale

        EarthPosition bore_in_meter=spot_centroid;
        bore_in_meter *=1000.0;//km to m

        Vector3 bore_sch_in_meter,bore_llh;
        xyz_to_llh(r_a,r_e2,bore_in_meter,bore_llh);
        sch.xyz_to_sch(bore_in_meter,bore_sch_in_meter);

        double bore_along, bore_cross;
        bore_along= bore_sch_in_meter(0)/1000.0;
        bore_cross= bore_sch_in_meter(1)/1000.0;
        //cout << "bs along: " << bore_along << endl;
        //cout << "bs cross: " << bore_cross << endl;

        //-------------------------------------------------------------
        //Set scan angle and beam index for ambiguity evaluation
        //-------------------------------------------------------------

        double scanangle = ovwm->sas.antenna.txCenterAzimuthAngle;
        scanangle *= rtd;

        if(scanangle<=270.0)
          scanangle = scanangle + 90.0;
        else
          scanangle = scanangle - 270.0;

        if(scanangle <0.0 || scanangle >360.0){
          fprintf(stderr,"Error:In Convert scan angle is out of range\n");
          exit(1);
        }
        unsigned int beam_id = ovwm->cds.currentBeamIdx;

        Vector3 centroid_llh, centroid_xyz_in_meter, centroid_sch_in_meter;
        double centroid_along, centroid_cross;
        double amb1_along, amb1_cross;//first ambigous point
        double amb2_along, amb2_cross;//second ambigous point

        //-------------------//
        // for each meas ... //
        //-------------------//

        int slice_i = 0;

        meas = meas_spot->GetHead();

        while(meas)
        {

            meas->measType = meas_type;
            meas->XK = 0.;

            //--------------------------//
            // ignore land if necessary //
            //--------------------------//
    
            if (meas->landFlag == 1 && simLandFlag == 0)
            {
                //cout << "land and not sim" << endl;

                // this is land, but we don't want land
                // remove this measurement, and go to the next
                meas = meas_spot->RemoveCurrent();
                delete meas; 
                meas = meas_spot->GetCurrent();
                slice_i++;
                continue;
            }

            // remove record with low magnitude

            if (meas->centroid.Magnitude() < 1000.0)
            {
              //cout << "centroid dist < 1000" << endl;

              meas=meas_spot->RemoveCurrent();
              delete meas;
              meas=meas_spot->GetCurrent();
              slice_i++;
              continue;
            }

            // remove record with low gain

            Vector3 rlook = meas->centroid - spacecraft->orbitState.rsat;
            Vector3 rlook_ant=gc_to_antenna.Forward(rlook);
            double r,theta,phi;
            rlook_ant.SphericalGet(&r,&theta,&phi);
            double gain;

            if(!beam->GetPowerGain(theta,phi,&gain)){
              gain=0;
            }
            gain/=maxgain;

            //cout << "check gain: " << gain << endl;
            //cout << "set min gain: " << minOneWayGain << endl;

            if(gain<minOneWayGain){
              //cout << "too small gain" << endl;

              meas=meas_spot->RemoveCurrent();
              delete meas;
              meas=meas_spot->GetCurrent();
              slice_i++;
              continue;
            }

            centroid_xyz_in_meter= meas->centroid;
            centroid_xyz_in_meter *=1000.0;//change km to meter
            xyz_to_llh(r_a, r_e2, centroid_xyz_in_meter, centroid_llh);
            sch.xyz_to_sch(centroid_xyz_in_meter,centroid_sch_in_meter);
            centroid_along= centroid_sch_in_meter(0)/1000.0;// km
            centroid_cross= centroid_sch_in_meter(1)/1000.0;// km

            // ambiguity table access:
            // beam number, azimuth angle
            // alongtrack wrt boresight, crosstrack wrt boresight
            // amb_along_location, amb_cross_location

            double amb1=ambigTable.GetAmbRat1(beam_id, scanangle,
                                               centroid_along-bore_along,
                                               centroid_cross-bore_cross,
                                               amb1_along,
                                               amb1_cross);

            double amb2=ambigTable.GetAmbRat2(beam_id, scanangle,
                                               centroid_along - bore_along,
                                               centroid_cross - bore_cross,
                                               amb2_along,
                                               amb2_cross);

            EarthPosition amb1pos,amb2pos;
            EarthPosition amb1pos_sch(amb1_along+bore_along,amb1_cross+bore_cross,0.0);
            EarthPosition amb2pos_sch(amb2_along+bore_along,amb2_cross+bore_cross,0.0);
            sch.sch_to_xyz(amb1pos_sch*1000.0,amb1pos);
            amb1pos=amb1pos/1000.0;
            sch.sch_to_xyz(amb2pos_sch*1000.0,amb2pos);
            amb2pos=amb2pos/1000.0;

            if (amb1==0 || amb2==0){
              if (amb1==0 && amb2==0){
                //cout << "amb too big" << endl;

                meas=meas_spot->RemoveCurrent();
                delete meas;
                meas=meas_spot->GetCurrent();
                slice_i++;
                continue;
              }
              else{
                fprintf(stderr,"Warning: Bad Ambiguity Condition 0 value for one ambig only\n");

                meas=meas_spot->RemoveCurrent();
                delete meas;
                meas=meas_spot->GetCurrent();
                slice_i++;
                continue;
              }
            }

            amb1=1/amb1;
            amb2=1/amb2;
            double amb=amb1+amb2;

            if(amb> 1/minSignalToAmbigRatio){

              //cout << "amb signal" << endl;

              meas=meas_spot->RemoveCurrent();
              delete meas;
              meas=meas_spot->GetCurrent();
              slice_i++;
              continue;
            }

            //int slice_i;
            //if (!rel_to_abs_idx(meas->startSliceIdx,slice_count,&slice_i))
            //{
            //    fprintf(stderr,"L1AToL1B::Convert, Bad slice number\n");
            //    exit(1);
            //}

            // OVWM will calculate X factor

            float x_factor = 1.0;
            float Es_pixel,En_pixel;
            float Esn_pixel = meas->value;

            //cout << Esn_pixel << endl;

            if (calXfactor) // calculate X factor
            {
                Vector3 rlook = meas->centroid - orbit_state->rsat;
                Vector3 rlook_ant=gc_to_antenna.Forward(rlook);
                double r,theta,phi;
                rlook_ant.SphericalGet(&r,&theta,&phi);

                // find the gain at the centroid
                // as we will find gain in each integration step, it seems no need to find gain here.
                double gain;
                if(!beam->GetPowerGain(theta,phi,&gain)){
                  gain=0;
                }
                gain/=maxgain;

                // compute point target response array for pixel

                Vector3 offset = meas->centroid - spot_centroid; // offset in xyz frame
                offset = gc_to_rangeazim.Forward(offset);
                float range_km = offset.GetX();
                float azimuth_km = offset.GetY();

                //cout << "eqx time: " << ovwm->cds.eqxTime << endl;
                //cout << time - ovwm->cds.eqxTime << endl;

                // scanAngle of meas is set in LocatePixels

                float scan_angle = meas->scanAngle;

                // Now set the default value, later update from Beam object
                int beam_num = ovwm->cds.currentBeamIdx+1;

                float rangewid, azimwid; // half widths

                rangewid=ptrTable.GetSemiMinorWidth(range_km, azimuth_km, scan_angle,
                                         orbit_time, beam_num)/1000.;
                azimwid=ptrTable.GetSemiMajorWidth(range_km, azimuth_km, scan_angle,
                                        orbit_time, beam_num)/1000.;

                //cout << "rng, az: " << range_km << " " << azimuth_km << endl;

/* in ovwm_sim */
/* to create all meas record */
/* assign certain values */
/* rangewid = 0.06 and azimwid = 1.0 */

          //if (fabs(range_km) >= RNG_SWATH_WIDTH/2. ||
          //    fabs(azimuth_km) >= AZ_SWATH_WIDTH/2.) {
          //  rangewid = 0.06;
          //  azimwid = 1.00;
          //  //meas->azimuth_width = 2.*azimwid;
          //}

                if (fabs(range_km) >= RNG_SWATH_WIDTH/2.) rangewid = 0.;
                if (fabs(azimuth_km) >= AZ_SWATH_WIDTH/2.) azimwid = 0.;

                // remove meas if it is out of ptr range

                if(rangewid==0. || azimwid==0.){
                  //cout << "outside ptr range" << endl;

                  meas=meas_spot->RemoveCurrent();
                  delete meas;
                  meas=meas_spot->GetCurrent();
                  slice_i++;
                  continue;
                }

                //cout << "range wid: " << rangewid << endl;
                //cout << "azimu wid: " << azimwid << endl;

                //cout << "int range factor: " << integrationRangeWidthFactor << endl;
                //cout << "int azimu factor: " << integrationAzimuthWidthFactor << endl;
                //cout << "int step size: " << integrationStepSize << endl;

                meas->azimuth_width = 2.*azimwid;

                //cout << range_km << endl;
                //cout << RNG_SWATH_WIDTH << endl;
                //cout << azimuth_km << endl;
                //cout << AZ_SWATH_WIDTH << endl;
                //if (fabs(range_km) >= RNG_SWATH_WIDTH/2.) rangewid = 0.;
                //if (fabs(azimuth_km) >= AZ_SWATH_WIDTH/2.) azimwid = 0.;

                // set up integration lengths
                int nL=ovwm->ses.numRangeLooksAveraged;
                int nrsteps=(int)ceil(integrationRangeWidthFactor*rangewid*2*nL/integrationStepSize)+1;
                int nasteps=(int)ceil(integrationAzimuthWidthFactor*azimwid*2/integrationStepSize)+1;

                //cout << integrationRangeWidthFactor*rangewid << endl;
                //cout << "rng look: " << nL << endl;
                //cout << "steps (rng, az): " << nrsteps << " " << nasteps << endl;

                if(nrsteps>_max_int_range_bins || nasteps > _max_int_azim_bins){
                  fprintf(stderr,"Error SetMeasurements too many integrations bins\n");
                  exit(1);
                }
                if(nL%2!=0){
                  fprintf(stderr,"Error SetMeasurements rangeLooksAveraged must be even\n");
                  exit(1);
                }

                // set up pixel center within integration window
                float center_azim_idx= (nasteps-1)/2.0;

                // 1 range center for each look
                // HACK assumes at most 10 looks average
                float center_range_idx[10];
                float center_range_idx_ave=(nrsteps-1)/2.0;

                for(int n=0;n<nL;n++){
                  int n2=n-nL/2;
                  //HACK Actual spacing of range looks on ground should be used
                  //     instead of rangewid
                  center_range_idx[n]=(nrsteps-1)/2.0+(2*n2+1)*rangewid/integrationStepSize;
                }

                // initialize ptresponse array
                for(int i=0;i<nrsteps;i++){
                   for(int j=0;j<nasteps;j++){
                     _ptr_array[i][j]=0;
                   }
                }

                double areaeff=0, areaeff_SL=0;

                for(int i=0;i<nrsteps;i++){
                  float ii=i;

                  for(int j=0;j<nasteps;j++){
                    float jj=j;

                    float val2=(jj-center_azim_idx)*integrationStepSize;
                    val2/=azimwid;
                    val2*=val2;

                    for(int n=0;n<nL;n++){
                      float val1=(ii-center_range_idx[n])*integrationStepSize;
                      val1/=rangewid;
                      val1*=val1;

                      _ptr_array[i][j]+=exp(-(val1+val2));
                      areaeff+=_ptr_array[i][j]*integrationStepSize*integrationStepSize;
                    }

                  } // az steps

                } // rng steps

                //cout << "eff area: " << areaeff << endl;
 
                Vector3 center_ra=gc_to_rangeazim.Forward(meas->centroid-spot_centroid);
                double r0=center_ra.Get(0);
                double a0=center_ra.Get(1);

                for(int i=0;i<nrsteps;i++){
                  double r=r0+(i-center_range_idx_ave)*integrationStepSize;

                  for(int j=0;j<nasteps;j++){
                    double a=a0+(j-center_azim_idx)*integrationStepSize;

                    Vector3 locra(r,a,0.0); 
                    EarthPosition locgc=gc_to_rangeazim.Backward(locra);
                    locgc+=spot_centroid;

                    //-----------------------------
                    // compute gain and range
                    //-----------------------------

                    Vector3 rl = locgc - spacecraft->orbitState.rsat;
                    Vector3 rl_ant=gc_to_antenna.Forward(rl);
                    double range,theta,phi;
                    rl_ant.SphericalGet(&range,&theta,&phi);
                    double gain;
                    if(!beam->GetPowerGain(theta,phi,&gain)){
                      gain=0;
                    }
                    double GatGar=gain*gain; // assumes separate transmit and receive feeds

                    //----------------------------
                    // Integrate X and Es
                    //----------------------------
                    float dX=GatGar*_ptr_array[i][j]*integrationStepSize*
                     integrationStepSize/(range*range*range*range);
                    meas->XK+=dX;

                  } // az steps

                } // rng steps

                double lambda = speed_light_kps/ ovwm->ses.txFrequency;
                double ksig=ptgr*lambda*
                  lambda*ovwm->ses.txPulseWidth*ovwm->ses.numPulses
                  /(64*pi*pi*pi*ovwm->systemLoss);

                meas->XK *= ksig;

                // meas->value is the Esn value going in
                // sigma0 coming out.

                if (! ComputeSigma0(ovwm, meas, meas->XK, Esn_pixel,
                    Esn_echo, Esn_noise, En_echo_load, En_noise_load,
                    &Es_pixel, &En_pixel))
                {
                    return(0);
                }

                //cout << "rec #, meas value and XK: " << slice_i
                //     << " " << meas-> value << " " << meas->XK << endl;

            }
            else
            {
                fprintf(stderr,
                    "L1AToL1B::Convert:No X computation algorithm set\n");
                exit(0);
            }

            meas->beamIdx = ovwm->cds.currentBeamIdx;
            meas->txPulseWidth = ovwm->ses.txPulseWidth;

            //------------------//
            // store check data //
            //------------------//

            if (simVs1BCheckfile)
            {
                Vector3 rlook = meas->centroid - spacecraft->orbitState.rsat;
                cf.R[slice_i] = (float)rlook.Magnitude();
                //if (useBYUXfactor)
                if (1)
                {
                    // Antenna gain is not computed when using BYU X factor
                    // because the X factor already includes the normalized
                    // patterns.  Thus, to see what it actually is, we need
                    // to do the geometry work here that is normally done
                    // in radar_X() when using the K-factor approach.
                    double roundTripTime = 2.0*cf.R[slice_i]/speed_light_kps;

                    Beam* beam = ovwm->GetCurrentBeam();
                    Vector3 rlook_antenna = gc_to_antenna.Forward(rlook);
                    double r, theta, phi;
                    rlook_antenna.SphericalGet(&r,&theta,&phi);
                    if (! beam->GetPowerGainProduct(theta, phi, roundTripTime,
                        ovwm->sas.antenna.spinRate, &(cf.GatGar[slice_i])))
                    {
                        cf.GatGar[slice_i] = 1.0;  // set a dummy value.
                    }
                }

                cf.idx[slice_i] = meas->startSliceIdx;
                cf.measType[slice_i] = meas->measType;
                cf.var_esn_slice[slice_i] = 0;
                cf.Es[slice_i] = Es_pixel;
                cf.En[slice_i] = En_pixel;
                cf.sigma0[slice_i] = meas->value;
                cf.XK[slice_i] = meas->XK;
                cf.centroid[slice_i] = meas->centroid;
                cf.azimuth[slice_i] = meas->eastAzimuth;
                cf.incidence[slice_i] = meas->incidenceAngle;
            }

            //--------------------------------//
            // Print calculated sigma0 values //
            // to stdout.                     //
            //--------------------------------//

            if (outputSigma0ToStdout)
                printf("%g ",meas->value);

            // duplicate low res measurements so gridding will work down to 1 km

            int numdup=(int)(meas->azimuth_width);
            if(numdup%2==0) numdup--; // make an odd number of measurements
            for(int i=-numdup/2;i<=numdup/2;i++){
              if(i==0) continue;
              //cout << "in dup: " << i << endl;
              Meas* m=new Meas;
              *m=*meas;
              m->centroid=m->centroid+yvec*float(i);
                // yvec is the azimuth direction vector
              meas_spot->InsertAfter(m);
            }

            meas = meas_spot->GetNext();

            slice_i++; // check whether we need slice_i
        }

        //------------------------//
        // Output data if enabled //
        //------------------------//

        if (simVs1BCheckfile)
        {
          FILE* fptr = fopen(simVs1BCheckfile,"a");
          if (fptr == NULL)
          {
            fprintf(stderr,"Error opening %s\n",simVs1BCheckfile);
            exit(-1);
          }
          cf.pulseCount = pulseCount;
          cf.ptgr = ovwm->ses.transmitPower * ovwm->ses.rxGainEcho;
          cf.time = time;
          cf.beamNumber = ovwm->cds.currentBeamIdx;
          cf.rsat = spacecraft->orbitState.rsat;
          cf.vsat = spacecraft->orbitState.vsat;
          cf.orbitFrac = ovwm->cds.OrbitFraction();
          cf.spinRate = ovwm->sas.antenna.spinRate;
          cf.txDoppler = ovwm->ses.txDoppler;
          cf.rxGateDelay = ovwm->ses.rxGateDelay;
          cf.attitude = spacecraft->attitude;
          cf.antennaAziTx = ovwm->sas.antenna.txCenterAzimuthAngle;
          cf.antennaAziGi = ovwm->sas.antenna.groundImpactAzimuthAngle;
          cf.EsCal = Es_cal;
          cf.alpha = 1.0/beta * En_noise_load/En_echo_load;
          cf.EsnEcho = Esn_echo;
          cf.EsnNoise = Esn_noise;
          cf.WriteDataRec(fptr);
          fclose(fptr);
        }

        //----------------------//
        // add to list of spots //
        //----------------------//

        l1b->frame.spotList.Append(meas_spot);
        pulseCount++;
    }

    if (outputSigma0ToStdout)
    {
      printf("\n");
    }

    return(1);
}

//-----------------------------//
// OvwmL1AToL1B::ComputeSigma0 //
//-----------------------------//
//
// The ComputeSigma0 method estimates sigma0 from five energy
// measurements and the tabulated X factor.
// Various outputs are put in the Meas object passed in.
// Note that the rho-factor is assumed to be 1.0. ie., we assume that
// all of the signal power falls in the slices.
//
// Inputs:
//    qscat = pointer to current Qscat object
//    meas = pointer to current measurement (holds results)
//    Xfactor = Total radar equation parameter for this slice.
//    Esn_slice = the received slice energy.
//    Esn_echo = the sum of all the slice energies for this spot.
//    Esn_noise = the noise channel measured energy.
//  En_echo_load = reference load echo channel measurement
//  En_noise_load = reference load noise channel measurement
//

int
OvwmL1AToL1B::ComputeSigma0(
    Ovwm*   ovwm,
    Meas*   meas,
    float   Xfactor,
    float   Esn_pixel,
    float   Esn_echo,
    float   Esn_noise,
    float   En_echo_load,
    float   En_noise_load,
    float*  Es_pixel,
    float*  En_pixel)
{

/* way follow QuikScat */
/* equally distributed noise */

    //--------------------------------//
    // Extract some useful quantities.
    //--------------------------------//

    double beta = ovwm->ses.rxGainNoise / ovwm->ses.rxGainEcho;

    int numPixels = ovwm->GetNumberOfPixels();

    //-------------------------------------------//
    // Estimate pixel signal and noise energies.
    //-------------------------------------------//
    
    if (! Er_to_Es(beta, Esn_pixel, Esn_echo, Esn_noise, En_echo_load,
                   En_noise_load, numPixels, Es_pixel, En_pixel))
    {
      return(0);
    }

    //------------------------------------------------------------------//
    // Compute sigma0 from estimated signal energy and X factors.
    // The resulting sigma0 should have a variance equal to Kpc^2+Kpr^2.
    // Kpc comes from Es_slice.
    // Kpr comes from 1/X (ie., from Es_cal when computing X)
    // Xfactor has units of energy because Xcal has units of Pt * Tp.
    //------------------------------------------------------------------//

    meas->value = *Es_pixel / Xfactor;
    meas->EnSlice = *En_pixel;

    //cout << "pixel s0: " << meas->value << endl;
    //cout << "pixel noise: " << meas->EnSlice << endl;
    //cout << "pixel XK: " << meas->XK << endl;

    //------------------------------------------------------------------//
    // Store the total X factor.
    //------------------------------------------------------------------//

    //meas->XK = Xfactor;

/*** result of current algorithm in ovwm_sim ***/
    //double N0 = bK*ovwm->systemTemperature;
    int nL = ovwm->ses.numRangeLooksAveraged;
 
    //*En_pixel = ovwm->ses.rxGainEcho*N0*nL;

    //*Es_pixel = meas->value - *En_pixel;

    //------------------------------------------------------------------//
    // Estimate Kpc coefficients
    //------------------------------------------------------------------//

    // Consistent with meas->numSlices=-1 case in GMF::GetVariance
    // kpm is removed 
    //float kpmtoremove=0.16;
    float s0ne=*En_pixel/meas->XK; // noise equivalent s0
    float alpha=1/(float)nL;
    meas->A=alpha+1.0;
    //meas->A=(alpha+1.0)/(1+kpmtoremove*kpmtoremove);
    meas->B=2.0*s0ne/(float)nL;
    meas->C=s0ne*s0ne/(float)nL;

    return(1);
}
