//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_l1a_c[] =
	"@(#) $Id$";

#include <memory.h>
#include <malloc.h>
#include "L1A.h"

//=====//
// L1A //
//=====//

L1A::L1A()
:	buffer(NULL), bufferSize(0), _status(OK)
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
    return(1);
}

//-----------------------//
// L1A::DeallocateBuffer //
//-----------------------//

int
L1A::DeallocateBuffer()
{
	if (buffer)
		free(buffer);
	bufferSize = 0;
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
L1A::WriteDataRecAscii(){
  if(_outputFp==NULL) return(0);
  if(!frame.WriteAscii(_outputFp)) return(0);
  return(1);
}

//--------------------------//
// L1A::WriteGSDataRec   //
//--------------------------//
int
L1A::WriteGSDataRec(void)
{
    if(_outputFp==NULL) return(0);

    // zero out the frame buffer
    char gsFrameBuffer[GS_L1A_FRAME_SIZE];
    (void)memset(&gsFrameBuffer, 0, GS_L1A_FRAME_SIZE);

    char* ptr = gsFrameBuffer;
    // write byte size header for fortran header
    int gsL1AFrameSize = GS_L1A_FRAME_SIZE - 8;
    (void)memcpy(ptr, &gsL1AFrameSize, sizeof(int)); ptr += sizeof(int);

    GSL1APcd* in_pcdP = &(gsFrame.in_pcd);
    (void)memcpy(in_pcdP->frame_time, frame.frame_time, 21);
    in_pcdP->time = frame.frame_time_secs;
    in_pcdP->instrument_time = frame.frame_time_secs;
    in_pcdP->orbit_time = (int)frame.orbitTicks;
    in_pcdP->x_pos = frame.gcX;
    in_pcdP->y_pos = frame.gcY;
    in_pcdP->z_pos = frame.gcZ;
    in_pcdP->x_vel = frame.velX;
    in_pcdP->y_vel = frame.velY;
    in_pcdP->z_vel = frame.velZ;
    in_pcdP->roll = frame.attitude.GetRoll();
    in_pcdP->pitch = frame.attitude.GetPitch();
    in_pcdP->yaw = frame.attitude.GetYaw();
    (void)memcpy(ptr, in_pcdP, sizeof(GSL1APcd)); ptr += sizeof(GSL1APcd);

    GSL1AStatus* statusP = &(gsFrame.status);
    statusP->prf_count = frame.prf_count;
    statusP->prf_cycle_time = frame.prf_cycle_time;
    statusP->range_gate_a_delay = frame.range_gate_a_delay;
    statusP->range_gate_a_width = frame.range_gate_a_width;
    statusP->range_gate_b_delay = frame.range_gate_b_delay;
    statusP->range_gate_b_width = frame.range_gate_b_width;
    statusP->pulse_width = frame.pulse_width;
    statusP->pred_antenna_pos_count = frame.pred_antenna_pos_count;
    (void)memcpy(&(statusP->vtcw), &(frame.vtcw), 8);
    (void)memcpy(ptr, statusP, sizeof(GSL1AStatus));
    ptr += sizeof(GSL1AStatus);

    GSL1AEngData* engdataP = &(gsFrame.engdata);
    engdataP->precision_coupler_temp = frame.precision_coupler_temp;
    engdataP->rcv_protect_sw_temp = frame.rcv_protect_sw_temp;
    engdataP->beam_select_sw_temp = frame.beam_select_sw_temp;
    engdataP->receiver_temp = frame.receiver_temp;
    (void)memcpy(ptr, engdataP, sizeof(GSL1AEngData));
    ptr += sizeof(GSL1AEngData);

    // pad1[2]
    (void)memset(ptr, 0, 2);
    ptr += 2;

    // SESerrflags[13]
    (void)memset(ptr, 0, 13);
    ptr += 13;

    // pad2
    (void)memset(ptr, 0, 1);
    ptr++;

    // memro[26]
    (void)memset(ptr, 0, 26);
    ptr += 26;

    // pcd[32]
    (void)memset(ptr, 0, 32);
    ptr += 32;

    GSL1AEu* in_euP = &(gsFrame.in_eu);
    in_euP->prf_cycle_time_eu = (float)frame.prf_cycle_time_eu;
    in_euP->range_gate_delay_inner = (float)frame.range_gate_delay_inner;
    in_euP->range_gate_delay_outer = (float)frame.range_gate_delay_outer;
    in_euP->range_gate_width_inner = (float)frame.range_gate_width_inner;
    in_euP->range_gate_width_outer = (float)frame.range_gate_width_outer;
    in_euP->transmit_pulse_width = (float)frame.transmit_pulse_width;
    in_euP->true_cal_pulse_pos = (int)frame.true_cal_pulse_pos;
    in_euP->precision_coupler_temp_eu = (float)frame.precision_coupler_temp_eu;
    in_euP->rcv_protect_sw_temp_eu = (float)frame.rcv_protect_sw_temp_eu;
    in_euP->beam_select_sw_temp_eu = (float)frame.beam_select_sw_temp_eu;
    in_euP->receiver_temp_eu = (float)frame.receiver_temp_eu;
    (void)memcpy(ptr, in_euP, sizeof(GSL1AEu));
    ptr += sizeof(GSL1AEu);

    GSL1ASci* in_scienceP = &(gsFrame.in_science);
    // in_science->
    // if (frame.slicesPerSpot == 10)
    (void)memcpy(ptr, in_scienceP, sizeof(GSL1ASci));
    ptr += sizeof(GSL1ASci);

    // la1_frame_inst_status
    (void)memcpy(ptr, &(frame.frame_inst_status), sizeof(int));
    ptr += sizeof(int);

    // l1a_frame_err_status
    (void)memcpy(ptr, &(frame.frame_err_status), sizeof(int));
    ptr += sizeof(int);

    // l1a_frame_qual_flag
    (void)memcpy(ptr, &(frame.frame_qual_flag), sizeof(short));
    ptr += sizeof(short);

    // l1a_pulse_qual_flag[13]
    (void)memcpy(ptr, &(frame.pulse_qual_flag), 13);
    ptr += 13;

    // pad3
    (void)memset(ptr, 0, 1);
    ptr++;

    // write byte size trailer for fortran header
    (void)memcpy(ptr, &gsL1AFrameSize, sizeof(int)); ptr += sizeof(int);

    return(Write(gsFrameBuffer, GS_L1A_FRAME_SIZE));
}
