//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_l1a_c[] =
	"@(#) $Id$";

#include <memory.h>
#include <malloc.h>
#include "L1A.h"

#ifndef IS_EVEN
#define IS_EVEN(x) (x % 2 == 0 ? 1 : 0)
#endif

#define GET_L1A_FIRST_PULSE(x) ((x & 0x00000004) >> 2)

//=====//
// L1A //
//=====//

L1A::L1A()
:	buffer(NULL), bufferSize(0), _status(OK), _calPulseFP(0)
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
L1A::WriteDataRecAscii()
{
  if(_outputFp==NULL) return(0);
  if(!frame.WriteAscii(_outputFp)) return(0);
  return(1);
}

//----------------------------//
// L1A::WriteGSDataRecAscii   //
//----------------------------//
int
L1A::WriteGSDataRecAscii()
{
  if(_outputFp==NULL) return(0);
  if(!frame.WriteAscii(_outputFp)) return(0);

  //--------------------------------------//
  // Now write out the GS specific stuff. //
  //--------------------------------------//

  fprintf(_outputFp,"\n GS Status Block\n\n");
  fprintf(_outputFp,"prf_count = %d\n",frame.status.prf_count);
  fprintf(_outputFp,"prf_cycle_time = %d\n",frame.status.prf_cycle_time);
  fprintf(_outputFp,"range_gate_a_delay = %d\n",
    frame.status.range_gate_a_delay);
  fprintf(_outputFp,"range_gate_a_width = %d\n",
    frame.status.range_gate_a_width);
  fprintf(_outputFp,"range_gate_b_delay = %d\n",
    frame.status.range_gate_b_delay);
  fprintf(_outputFp,"range_gate_b_width = %d\n",
    frame.status.range_gate_b_width);
  fprintf(_outputFp,"pulse_width = %d\n",frame.status.pulse_width);
  fprintf(_outputFp,"pred_antenna_pos_count = %d\n",
    frame.status.pred_antenna_pos_count);
  fprintf(_outputFp,"doppler_orbit_step = %d\n",
    frame.status.doppler_orbit_step);
  fprintf(_outputFp,"prf_orbit_step_change = %d\n",
    frame.status.prf_orbit_step_change);
  double vtcw = 0.0;
  (void)memcpy(&vtcw, frame.status.vtcw, sizeof(double));
  fprintf(_outputFp,"vtcw = %g\n",vtcw);
  double corres_instr_time = 0.0;
  (void)memcpy(&corres_instr_time, frame.status.corres_instr_time,
         sizeof(double));
  fprintf(_outputFp,"corres_instr_time = %g\n",corres_instr_time);

  fprintf(_outputFp,"\n GS Engineering Block\n\n");
  fprintf(_outputFp,"precision_coupler_temp = %d\n",
    frame.engdata.precision_coupler_temp);
  fprintf(_outputFp,"rcv_protect_sw_temp = %d\n",
    frame.engdata.rcv_protect_sw_temp);
  fprintf(_outputFp,"beam_select_sw_temp = %d\n",
    frame.engdata.beam_select_sw_temp);
  fprintf(_outputFp,"receiver_temp = %d\n",
    frame.engdata.receiver_temp);

  fprintf(_outputFp,"\n GS EU Block\n\n");
  fprintf(_outputFp,"prf_cycle_time_eu = %g\n",
    frame.in_eu.prf_cycle_time_eu);
  fprintf(_outputFp,"range_gate_delay_inner = %g\n",
    frame.in_eu.range_gate_delay_inner);
  fprintf(_outputFp,"range_gate_delay_outer = %g\n",
    frame.in_eu.range_gate_delay_outer);
  fprintf(_outputFp,"range_gate_width_inner = %g\n",
    frame.in_eu.range_gate_width_inner);
  fprintf(_outputFp,"range_gate_width_outer = %g\n",
    frame.in_eu.range_gate_width_outer);
  fprintf(_outputFp,"transmit_pulse_width = %g\n",
    frame.in_eu.transmit_pulse_width);
  fprintf(_outputFp,"true_cal_pulse_pos = %d\n",
    frame.in_eu.true_cal_pulse_pos);
  fprintf(_outputFp,"transmit_power_inner = %g\n",
    frame.in_eu.transmit_power_inner);
  fprintf(_outputFp,"transmit_power_outer = %g\n",
    frame.in_eu.transmit_power_outer);
  fprintf(_outputFp,"precision_coupler_temp_eu = %g\n",
    frame.in_eu.precision_coupler_temp_eu);
  fprintf(_outputFp,"rcv_protect_sw_temp_eu = %g\n",
    frame.in_eu.rcv_protect_sw_temp_eu);
  fprintf(_outputFp,"beam_select_sw_temp_eu = %g\n",
    frame.in_eu.beam_select_sw_temp_eu);
  fprintf(_outputFp,"receiver_temp_eu = %g\n",
    frame.in_eu.receiver_temp_eu);

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
    (void)memset(in_pcdP->frame_time, 0, 24);
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

/*
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
*/
    (void)memcpy(&(gsFrame.status), &(frame.status), sizeof(GSL1AStatus));
    ptr += sizeof(GSL1AStatus);

/*
    GSL1AEngData* engdataP = &(gsFrame.engdata);
    engdataP->precision_coupler_temp = frame.precision_coupler_temp;
    engdataP->rcv_protect_sw_temp = frame.rcv_protect_sw_temp;
    engdataP->beam_select_sw_temp = frame.beam_select_sw_temp;
    engdataP->receiver_temp = frame.receiver_temp;
    (void)memcpy(ptr, engdataP, sizeof(GSL1AEngData));
*/
    (void)memcpy(&(gsFrame.engdata), &(frame.engdata), sizeof(GSL1AEngData));
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

/*
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
*/
    (void)memcpy(&(gsFrame.in_eu), &(frame.in_eu), sizeof(GSL1AEu));
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
    (void)memcpy(ptr, &(frame.frame_time_secs), sizeof(double));
    ptr += sizeof(double);

    for (i=0; i < 12; i++)
    {
        (void)memcpy(ptr, &(frame.loopbackSlices[i]), sizeof(int));
        ptr += sizeof(int);
    }
    (void)memcpy(ptr, &(frame.loopbackNoise), sizeof(int));
    ptr += sizeof(int);

    for (i=0; i < 12; i++)
    {
        (void)memcpy(ptr, &(frame.loadSlices[i]), sizeof(int));
        ptr += sizeof(int);
    }
    (void)memcpy(ptr, &(frame.loadNoise), sizeof(int));
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
    (void)memcpy(ptr, &(frame.in_eu.true_cal_pulse_pos), sizeof(char));
    ptr += sizeof(char);

    // Set beam identifier based on true unit offset position and first pulse
    // identity.
    if (IS_EVEN(frame.in_eu.true_cal_pulse_pos))
    {
      // different
      if (GET_L1A_FIRST_PULSE(frame.frame_inst_status) == 0)
        *ptr = 1;
      else
        *ptr = 0;
    }
    else
    {
      // same
      if (GET_L1A_FIRST_PULSE(frame.frame_inst_status) == 0)
        *ptr = 0;
      else
        *ptr = 1;
    }
//    printf("%d %d\n",frame.in_eu.true_cal_pulse_pos,*ptr);

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
    fprintf(_calPulseFP,"frame_time_secs = %g\n",frame.frame_time_secs);
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
