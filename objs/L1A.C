//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_l1a_c[] =
	"@(#) $Id$";

#include <memory.h>
#include <malloc.h>
#include "L1A.h"

#define GET_L1A_FIRST_PULSE(x) ((x & 0x00000004) >> 2)

//=====//
// L1A //
//=====//

L1A::L1A()
:	buffer(NULL), bufferSize(0), gsBuffer(NULL), _status(OK), _calPulseFP(0)
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

//--------------------------//
// L1A::WriteDataRecAscii   //
//--------------------------//
int
L1A::WriteDataRecAscii()
{
  if(_outputFp==NULL) return(0);
  if(!frame.WriteAscii(_outputFp)) return(0);
  return(1);
}

int
L1A::ReadGSDataRec(void)
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

//----------------------------//
// L1A::WriteGSDataRecAscii   //
//----------------------------//

int
L1A::WriteGSDataRecAscii()
{
    if(_outputFp==NULL) return(0);

    return(gsFrame.WriteAscii(_outputFp));

}

//-----------------------//
// L1A::WriteGSDataRec   //
//-----------------------//

int
L1A::WriteGSDataRec()
{
    if(_outputFp==NULL) return(0);
    FillGSFrame();
    if (gsFrame.Pack(gsBuffer) == 0) return 0;
    return(Write(gsBuffer, GS_L1A_FRAME_SIZE));

}

//--------------------//
// L1A::FillGSFrame   //
//--------------------//

int
L1A::FillGSFrame(void)
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

    for (int i=0; i < frame.slicesPerFrame; i++)
    {  // convert floats to ints
      *(gsFrame.in_science.power_dn[0] + i) = (int)frame.science[i];
    }
    for (int i=0; i < frame.spotsPerFrame; i++)
    {  // convert floats to ints
      gsFrame.in_science.noise_dn[i] = (int)frame.spotNoise[i];
    }

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


int
L1A::OpenCalPulseForWriting(
const char*    filename)
{
    if (filename == 0) return 0;
    _calPulseFP = fopen(filename, "w");
    return(_calPulseFP == 0 ? 0 : 1);
}

int
L1A::CloseCalPulseFile(void)
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

//--------------------------//
// L1A::WriteCalPulse       //
//--------------------------//
int
L1A::WriteGSCalPulseRec(void)
{
    int i=0;  // loop counter

    if (_calPulseFP == NULL) return(0);

    //------------------------------------------------------------------
    // The record size of the cal pulse file is now 150 bytes, without
    // any leading or trailing words (i.e. not F77-unformatted).
    //------------------------------------------------------------------

    // zero out the cal pulse buffer
    char calPulseBuffer[GS_CAL_PULSE_FRAME_SIZE];
    (void)memset(calPulseBuffer, 0, GS_CAL_PULSE_FRAME_SIZE);

    char* ptr = calPulseBuffer;
    (void)memcpy(ptr, &(frame.time), sizeof(double));
    ptr += sizeof(double);

    unsigned char tpos = (unsigned char)frame.in_eu.true_cal_pulse_pos;
    (void)memcpy(ptr, &tpos, sizeof(char));
    ptr += sizeof(char);

    // Set beam identifier based on true unit offset position and first pulse
    // identity.
    unsigned char beam_num;
    if (IS_EVEN(frame.in_eu.true_cal_pulse_pos))
    {
      // different
      if (GET_L1A_FIRST_PULSE(frame.frame_inst_status) == 0)
        beam_num = 1;
      else
        beam_num = 0;
    }
    else
    {
      // same
      if (GET_L1A_FIRST_PULSE(frame.frame_inst_status) == 0)
        beam_num = 0;
      else
        beam_num = 1;
    }
    (void)memcpy(ptr, &beam_num, sizeof(char));
    ptr += sizeof(char);
//    printf("%d %d\n",frame.in_eu.true_cal_pulse_pos,*ptr);

    int Esn;  // integer storage for floating point cal pulse data
    for (i=0; i < 12; i++)
    {
        Esn = (int)frame.loopbackSlices[i];
        (void)memcpy(ptr, &Esn, sizeof(int));
        ptr += sizeof(int);
    }
    Esn = (int)frame.loopbackNoise;
    (void)memcpy(ptr, &Esn, sizeof(int));
    ptr += sizeof(int);

    for (i=0; i < 12; i++)
    {
        Esn = (int)frame.loadSlices[i];
        (void)memcpy(ptr, &Esn, sizeof(int));
        ptr += sizeof(int);
    }
    Esn = (int)frame.loadNoise;
    (void)memcpy(ptr, &Esn, sizeof(int));
    ptr += sizeof(int);
    (void)memcpy(ptr, &(frame.in_eu.precision_coupler_temp_eu), sizeof(float));
    ptr += sizeof(float);
    (void)memcpy(ptr, &(frame.in_eu.rcv_protect_sw_temp_eu), sizeof(float));
    ptr += sizeof(float);
    (void)memcpy(ptr, &(frame.in_eu.beam_select_sw_temp_eu), sizeof(float));
    ptr += sizeof(float);
    (void)memcpy(ptr, &(frame.in_eu.receiver_temp_eu), sizeof(float));
    ptr += sizeof(float);
    (void)memcpy(ptr, &(frame.in_eu.transmit_power_inner), sizeof(float));
    ptr += sizeof(float);
    (void)memcpy(ptr, &(frame.in_eu.transmit_power_outer), sizeof(float));
    ptr += sizeof(float);
    (void)memcpy(ptr, &(frame.frame_inst_status), sizeof(int));
    ptr += sizeof(int);
    (void)memcpy(ptr, &(frame.frame_err_status), sizeof(int));
    ptr += sizeof(int);

    return(fwrite(calPulseBuffer, GS_CAL_PULSE_FRAME_SIZE, 1, _calPulseFP));
}

//--------------------------//
// L1A::WriteCalPulseAscii  //
//--------------------------//
int
L1A::WriteGSCalPulseRecAscii(void)
{
    int i=0;  // loop counter

    if (_calPulseFP == NULL) return(0);

    fprintf(_calPulseFP,"Cal Pulse Record:\n");
    char ftime[25];
    (void)memcpy(ftime, frame.frame_time, 24);
    ftime[24] = '\0';
    fprintf(_calPulseFP, "frame time string (not in actual record): %s\n",
      ftime);
    fprintf(_calPulseFP,"frame_time_secs = %g\n",frame.time);
    for (i=0; i < 12; i++)
    {
      fprintf(_calPulseFP,"loopbackSlices[%d] = %g\n",
        i,frame.loopbackSlices[i]);
    }
    fprintf(_calPulseFP,"loopbackNoise = %g\n",frame.loopbackNoise);

    for (i=0; i < 12; i++)
    {
      fprintf(_calPulseFP,"loadSlices[%d] = %g\n",
        i,frame.loadSlices[i]);
    }
    fprintf(_calPulseFP,"loadNoise = %g\n",frame.loadNoise);
    fprintf(_calPulseFP,"precision_coupler_temp_eu = %g\n",
      frame.in_eu.precision_coupler_temp_eu);
    fprintf(_calPulseFP,"rcv_protect_sw_temp_eu = %g\n",
      frame.in_eu.rcv_protect_sw_temp_eu);
    fprintf(_calPulseFP,"beam_select_sw_temp_eu = %g\n",
      frame.in_eu.beam_select_sw_temp_eu);
    fprintf(_calPulseFP,"receiver_temp_eu = %g\n",
      frame.in_eu.receiver_temp_eu);
    fprintf(_calPulseFP,"transmit_power_inner = %g\n",
      frame.in_eu.transmit_power_inner);
    fprintf(_calPulseFP,"transmit_power_outer = %g\n",
      frame.in_eu.transmit_power_outer);
    fprintf(_calPulseFP,"frame_inst_status = %d\n",
      frame.frame_inst_status);
    fprintf(_calPulseFP,"frame_err_status = %d\n",
      frame.frame_err_status);
    fprintf(_calPulseFP,"true_cal_pulse_pos = %d\n",
      frame.in_eu.true_cal_pulse_pos);

    // Set beam identifier based on true unit offset position and first pulse
    // identity.
    if (IS_EVEN(frame.in_eu.true_cal_pulse_pos))
    {
      // different
      if (GET_L1A_FIRST_PULSE(frame.frame_inst_status) == 0)
        fprintf(_calPulseFP,"beam_identifier = %d\n",1);
      else
        fprintf(_calPulseFP,"beam_identifier = %d\n",0);
    }
    else
    {
      // same
      if (GET_L1A_FIRST_PULSE(frame.frame_inst_status) == 0)
        fprintf(_calPulseFP,"beam_identifier = %d\n",0);
      else
        fprintf(_calPulseFP,"beam_identifier = %d\n",1);
    }
    fprintf(_calPulseFP,"\n");

  return(1);

}
