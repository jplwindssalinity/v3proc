//==============================================================//
// Copyright (C) 1997-2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_l1a_c[] =
    "@(#) $Id$";

#include <memory.h>
#include <malloc.h>
#include "L1A.h"
#include "Misc.h"

#define GET_L1A_FIRST_PULSE(x) ((x & 0x00000004) >> 2)

//=====//
// L1A //
//=====//

L1A::L1A()
:    buffer(NULL), bufferSize(0), gsBuffer(NULL), _status(OK), _calPulseFP(0)
{
    return;
}

L1A::~L1A()
{
    DeallocateBuffer();
    return;
}

//---------------------//
// L1A::AllocateBuffer //
//---------------------//

int
L1A::AllocateBuffer()
{
    bufferSize = frame.FrameSize();
    buffer = (char *)malloc(bufferSize);
    if (buffer == NULL)
    {
        bufferSize = 0;
        return(0);
    }
    gsBuffer = (char*) malloc(GS_L1A_FRAME_SIZE);
    if (gsBuffer == NULL) return 0;
    return(1);
}

//-----------------------//
// L1A::DeallocateBuffer //
//-----------------------//

int
L1A::DeallocateBuffer()
{
    if (buffer)
    {
        free(buffer);
        buffer = 0;
    }
    bufferSize = 0;
    if (gsBuffer)
    {
        free(gsBuffer);
        gsBuffer = 0;
    }
    return(1);
}

//------------------//
// L1A::ReadDataRec //
//------------------//

int
L1A::ReadDataRec()
{
    if (! Read(buffer, bufferSize))
    {
        if (EndOfFile())
        {
            // end of file, leave status alone (typically status is OK)
            return(0);
        }
        else
        {
            // an error occurred
            _status = ERROR_READING_FRAME;
            return(0);
        }
    }
    return(1);
}

//-------------------//
// L1A::WriteDataRec //
//-------------------//

int
L1A::WriteDataRec()
{
    return(Write(buffer, bufferSize));
}

//------------------------//
// L1A::WriteDataRecAscii //
//------------------------//

int
L1A::WriteDataRecAscii()
{
  if(_outputFp==NULL) return(0);
  if(!frame.WriteAscii(_outputFp)) return(0);
  return(1);
}

//--------------------//
// L1A::ReadGSDataRec //
//--------------------//

int
L1A::ReadGSDataRec()
{
    if (! Read(gsBuffer, GS_L1A_FRAME_SIZE))
    {
        if (EndOfFile())
        {
            // end of file, leave status alone (typically status is OK)
            return(0);
        }
        else
        {
            // an error occurred
            _status = ERROR_READING_FRAME;
            return(0);
        }
    }
    return(1);
}

//--------------------------//
// L1A::WriteGSDataRecAscii //
//--------------------------//

int
L1A::WriteGSDataRecAscii()
{
    if(_outputFp==NULL) return(0);

    return(gsFrame.WriteAscii(_outputFp));

}

//---------------------//
// L1A::WriteGSDataRec //
//---------------------//

int
L1A::WriteGSDataRec()
{
    if(_outputFp==NULL) return(0);
    FillGSFrame();
    if (gsFrame.Pack(gsBuffer) == 0) return 0;
    return(Write(gsBuffer, GS_L1A_FRAME_SIZE));

}

//------------------//
// L1A::FillGSFrame //
//------------------//

int
L1A::FillGSFrame()
{
    //----------------------------------------------//
    // Transfer data from standard frame to gsFrame //
    //----------------------------------------------//

    GSL1APcd* in_pcdP = &(gsFrame.in_pcd);
    (void)memcpy(in_pcdP->frame_time, frame.frame_time, 24);
    in_pcdP->time = frame.time;
    // assumes fractional part of instrument_time (5th byte) is zero
    in_pcdP->instrument_time = (double)frame.instrumentTicks;
    in_pcdP->orbit_time = (int)frame.orbitTicks;
    in_pcdP->x_pos = frame.gcX*1000;
    in_pcdP->y_pos = frame.gcY*1000;
    in_pcdP->z_pos = frame.gcZ*1000;
    in_pcdP->x_vel = frame.velX*1000;
    in_pcdP->y_vel = frame.velY*1000;
    in_pcdP->z_vel = frame.velZ*1000;
    in_pcdP->roll = frame.attitude.GetRoll();
    in_pcdP->pitch = frame.attitude.GetPitch();
    in_pcdP->yaw = frame.attitude.GetYaw();

    (void)memcpy(&(gsFrame.status), &(frame.status), sizeof(GSL1AStatus));

    (void)memcpy(&(gsFrame.engdata), &(frame.engdata), sizeof(GSL1AEngData));

    (void)memset(&(gsFrame.SESerrflags), 0, 13);
    (void)memset(&(gsFrame.memro), 0, 26);
    (void)memset(&(gsFrame.pcd), 0, 32);

    (void)memcpy(&(gsFrame.in_eu), &(frame.in_eu), sizeof(GSL1AEu));

    (void)memcpy(gsFrame.in_science.antenna_position,
                 frame.antennaPosition, sizeof(short)*frame.spotsPerFrame);

    // Cal measurements need to be converted to floats (but this isn't used!)
    for (int i=0; i < frame.slicesPerSpot; i++)
    {  // convert floats to ints
      gsFrame.in_science.loop_back_cal_A_power[i]=(int)frame.loopbackSlices[i];
      gsFrame.in_science.loop_back_cal_B_power[i]=(int)frame.loopbackSlices[i];
      gsFrame.in_science.load_cal_A_power[i]=(int)frame.loadSlices[i];
      gsFrame.in_science.load_cal_B_power[i]=(int)frame.loadSlices[i];
    }
    gsFrame.in_science.loop_back_cal_noise = (int)frame.loopbackNoise;
    gsFrame.in_science.load_cal_noise = (int)frame.loadNoise;

/*
    (void)memcpy(gsFrame.in_science.loop_back_cal_A_power,
                 frame.loopbackSlices, sizeof(float)*frame.slicesPerSpot);
    (void)memcpy(gsFrame.in_science.loop_back_cal_B_power,
                 frame.loopbackSlices, sizeof(float)*frame.slicesPerSpot);
    (void)memcpy(&gsFrame.in_science.loop_back_cal_noise,
                 &frame.loopbackNoise, sizeof(float));
    (void)memcpy(gsFrame.in_science.load_cal_A_power,
                 frame.loadSlices, sizeof(float)*frame.slicesPerSpot);
    (void)memcpy(gsFrame.in_science.load_cal_B_power,
                 frame.loadSlices, sizeof(float)*frame.slicesPerSpot);
    (void)memcpy(&gsFrame.in_science.load_cal_noise,
                 &frame.loadNoise, sizeof(float));
*/

    (void)memcpy(gsFrame.in_science.power_dn,
                 frame.science, sizeof(unsigned int)*frame.slicesPerFrame);
    (void)memcpy(gsFrame.in_science.noise_dn,
                 frame.spotNoise, sizeof(unsigned int)*frame.spotsPerFrame);

/*
    for (int i=0; i < frame.slicesPerFrame; i++)
    {  // convert floats to ints
      *(gsFrame.in_science.power_dn[0] + i) = (int)frame.science[i];
    }
    for (int i=0; i < frame.spotsPerFrame; i++)
    {  // convert floats to ints
      gsFrame.in_science.noise_dn[i] = (int)frame.spotNoise[i];
    }
*/

    // la1_frame_inst_status
    (void)memcpy(&(gsFrame.l1a_frame_inst_status),
                   &(frame.frame_inst_status), sizeof(int));

    // l1a_frame_err_status
    (void)memcpy(&(gsFrame.l1a_frame_err_status),
                   &(frame.frame_err_status), sizeof(int));

    // l1a_frame_qual_flag
    (void)memcpy(&(gsFrame.l1a_frame_qual_flag),
                   &(frame.frame_qual_flag), sizeof(short));

    // l1a_pulse_qual_flag[13]
    (void)memcpy(&(gsFrame.l1a_pulse_qual_flag), &(frame.pulse_qual_flag), 13);

    return(1);
} // L1A::FillGSFrame


//-----------------------------//
// L1A::OpenCalPulseForWriting //
//-----------------------------//

int
L1A::OpenCalPulseForWriting(
    const char*  filename)
{
    if (filename == 0)
        return 0;
    _calPulseFP = fopen(filename, "w");
    return(_calPulseFP == 0 ? 0 : 1);
}

//------------------------//
// L1A::CloseCalPulseFile //
//------------------------//

int
L1A::CloseCalPulseFile()
{
    if (_calPulseFP == 0)
        return 0;
    else
    {
        if (fclose(_calPulseFP) == 0)
        {
            _calPulseFP = 0;
            return 1;
        }
        else return 0;
    }
}

//-------------------------//
// L1A::WriteGSCalPulseRec //
//-------------------------//
// totally rewritten by JNH 2001.03.29 (untested too!)

int
L1A::WriteGSCalPulseRec()
{
    if (_calPulseFP == NULL)
        return(0);

    L1A_Calibration_Pulse_Type cpr;

    // frame time
    cpr.frame_time_cal_secs = frame.time;

    // true cal pulse position
    cpr.true_cal_pulse_pos = (char)frame.in_eu.true_cal_pulse_pos;

    // beam identifier (I don't quite understand the code that follows)
    if (IS_EVEN(frame.in_eu.true_cal_pulse_pos))
    {
        // different
        if (GET_L1A_FIRST_PULSE(frame.frame_inst_status) == 0)
            cpr.beam_identifier = 1;
        else
            cpr.beam_identifier = 0;
    }
    else
    {
        // same
        if (GET_L1A_FIRST_PULSE(frame.frame_inst_status) == 0)
            cpr.beam_identifier = 0;
        else
            cpr.beam_identifier = 1;
    }

    // loop back cal slice powers
    // load cal slice powers
    for (int i = 0; i < SLICES_PER_PULSE; i++)
    {
        cpr.loop_back_cal_power[i] = *(frame.loopbackSlices + i);
        cpr.load_cal_power[i] = *(frame.loadSlices + i);
    }

    // loop back cal noise
    // load cal noise
    cpr.loop_back_cal_noise = frame.loopbackNoise;
    cpr.load_cal_noise = frame.loadNoise;

    // we don't have the rotary joint temp, i'll just use the coupler
    cpr.rj_temp_eu = frame.in_eu.precision_coupler_temp_eu;
    cpr.precision_coupler_temp_eu = frame.in_eu.precision_coupler_temp_eu;
    cpr.rcv_protect_sw_temp_eu = frame.in_eu.rcv_protect_sw_temp_eu;
    cpr.beam_select_sw_temp_eu = frame.in_eu.beam_select_sw_temp_eu;
    cpr.receiver_temp_eu = frame.in_eu.receiver_temp_eu;
    cpr.transmit_power_inner = frame.in_eu.transmit_power_inner;
    cpr.transmit_power_outer = frame.in_eu.transmit_power_outer;
    cpr.frame_inst_status = frame.frame_inst_status;
    cpr.frame_err_status = frame.frame_err_status;
    cpr.frame_qual_flag = frame.frame_inst_status;

    return(fwrite(&cpr, sizeof(L1A_Calibration_Pulse_Type), 1, _calPulseFP));
}

//------------------------//
// L1A::ReadGSCalPulseRec //
//------------------------//
// This poor method is now broken. :-(

int
L1A::ReadGSCalPulseRec(
    FILE*  calfile)
{
    if (calfile == NULL)
        return(0);

    //------------------------------------------------------------------
    // The record size of the cal pulse file is now 150 bytes, without
    // any leading or trailing words (i.e. not F77-unformatted).
    //------------------------------------------------------------------

    int s;
    if ((s = fread(&(frame.time), sizeof(double), 1, calfile)) != 1)
    {
        if (feof(calfile))
            return(0);
        fprintf(stderr, "Error reading calpulse frame time\n");
        return(0);
    }

    unsigned char tpos;
    if (fread(&tpos, sizeof(char), 1, calfile) != 1)
    {
        fprintf(stderr, "Error reading calpulse true_cal_pulse_pos \n");
        return(0);
    }
    frame.in_eu.true_cal_pulse_pos = tpos;

    unsigned char beam_num;
    if (fread(&beam_num, sizeof(char), 1, calfile) != 1)
    {
        fprintf(stderr, "Error reading calpulse beam_num\n");
        return(0);
    }
//    frame.frame_inst_status = need to set correct bit for first pulse;


    if ((s=fread((frame.loopbackSlices), sizeof(unsigned int),
              frame.slicesPerSpot, calfile)) != frame.slicesPerSpot)
    {
        fprintf(stderr, "Error reading calpulse loopbackSlices\n");
        return(0);
    }

    if (fread(&(frame.loopbackNoise), sizeof(int), 1, calfile) != 1)
    {
        fprintf(stderr, "Error reading calpulse loopbackNoise\n");
        return(0);
    }

    if ((s=fread((frame.loadSlices), sizeof(unsigned int),
              frame.slicesPerSpot, calfile)) != frame.slicesPerSpot)
    {
        fprintf(stderr, "Error reading calpulse loadSlices\n");
        return(0);
    }

    if (fread(&(frame.loadNoise), sizeof(int), 1, calfile) != 1)
    {
        fprintf(stderr, "Error reading calpulse loadNoise\n");
        return(0);
    }

    if (fread(&(frame.in_eu.precision_coupler_temp_eu), sizeof(float),
              1, calfile) != 1)
    {
        fprintf(stderr,
            "Error reading calpulse frame precision_coupler_temp\n");
        return(0);
    }
    if (fread(&(frame.in_eu.rcv_protect_sw_temp_eu), sizeof(float),
              1, calfile) != 1)
    {
        fprintf(stderr, "Error reading calpulse rcv_protect_sw_temp\n");
        return(0);
    }
    if (fread(&(frame.in_eu.beam_select_sw_temp_eu), sizeof(float),
              1, calfile) != 1)
    {
        fprintf(stderr, "Error reading calpulse beam_select_sw_temp\n");
        return(0);
    }
    if (fread(&(frame.in_eu.receiver_temp_eu), sizeof(float),
              1, calfile) != 1)
    {
        fprintf(stderr, "Error reading calpulse receiver_temp\n");
        return(0);
    }
    if (fread(&(frame.in_eu.transmit_power_inner), sizeof(float),
              1, calfile) != 1)
    {
        fprintf(stderr, "Error reading calpulse transmit_power_inner\n");
        return(0);
    }
    if (fread(&(frame.in_eu.transmit_power_outer), sizeof(float),
              1, calfile) != 1)
    {
        fprintf(stderr, "Error reading calpulse transmit_power_outer\n");
        return(0);
    }
    if (fread(&(frame.frame_inst_status), sizeof(int),
              1, calfile) != 1)
    {
        fprintf(stderr, "Error reading calpulse frame_inst_status\n");
        return(0);
    }
    if (fread(&(frame.frame_err_status), sizeof(int),
              1, calfile) != 1)
    {
        fprintf(stderr, "Error reading calpulse frame_err_status\n");
        return(0);
    }
    int dummy; // 4 bytes at end are not used.
    if (fread(&(dummy), sizeof(int),
              1, calfile) != 1)
    {
        fprintf(stderr, "Error reading calpulse dummy\n");
        return(0);
    }

    return(1);
}

//------------------------------//
// L1A::WriteGSCalPulseRecAscii //
//------------------------------//

int
L1A::WriteGSCalPulseRecAscii()
{
    int i=0;  // loop counter

    if (_calPulseFP == NULL) return(0);

    fprintf(_calPulseFP, "Cal Pulse Record:\n");
    char ftime[25];
    (void)memcpy(ftime, frame.frame_time, 24);
    ftime[24] = '\0';
    fprintf(_calPulseFP, "frame time string (not in actual record): %s\n",
      ftime);
    fprintf(_calPulseFP, "frame_time_secs = %g\n",frame.time);
    for (i=0; i < 12; i++)
    {
      fprintf(_calPulseFP, "loopbackSlices[%d] = %d\n",
        i,frame.loopbackSlices[i]);
    }
    fprintf(_calPulseFP, "loopbackNoise = %d\n",frame.loopbackNoise);

    for (i=0; i < 12; i++)
    {
      fprintf(_calPulseFP, "loadSlices[%d] = %d\n",
        i,frame.loadSlices[i]);
    }
    fprintf(_calPulseFP, "loadNoise = %d\n",frame.loadNoise);
    fprintf(_calPulseFP, "precision_coupler_temp_eu = %g\n",
      frame.in_eu.precision_coupler_temp_eu);
    fprintf(_calPulseFP, "rcv_protect_sw_temp_eu = %g\n",
      frame.in_eu.rcv_protect_sw_temp_eu);
    fprintf(_calPulseFP, "beam_select_sw_temp_eu = %g\n",
      frame.in_eu.beam_select_sw_temp_eu);
    fprintf(_calPulseFP, "receiver_temp_eu = %g\n",
      frame.in_eu.receiver_temp_eu);
    fprintf(_calPulseFP, "transmit_power_inner = %g\n",
      frame.in_eu.transmit_power_inner);
    fprintf(_calPulseFP, "transmit_power_outer = %g\n",
      frame.in_eu.transmit_power_outer);
    fprintf(_calPulseFP, "frame_inst_status = %d\n",
      frame.frame_inst_status);
    fprintf(_calPulseFP, "frame_err_status = %d\n",
      frame.frame_err_status);
    fprintf(_calPulseFP, "true_cal_pulse_pos = %d\n",
      frame.in_eu.true_cal_pulse_pos);

    // Set beam identifier based on true unit offset position and first pulse
    // identity.
    if (IS_EVEN(frame.in_eu.true_cal_pulse_pos))
    {
      // different
      if (GET_L1A_FIRST_PULSE(frame.frame_inst_status) == 0)
        fprintf(_calPulseFP, "beam_identifier = %d\n",1);
      else
        fprintf(_calPulseFP, "beam_identifier = %d\n",0);
    }
    else
    {
      // same
      if (GET_L1A_FIRST_PULSE(frame.frame_inst_status) == 0)
        fprintf(_calPulseFP, "beam_identifier = %d\n",0);
      else
        fprintf(_calPulseFP, "beam_identifier = %d\n",1);
    }
    fprintf(_calPulseFP, "\n");

    return(1);
}
