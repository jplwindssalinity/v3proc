//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_l1aframe_c[] =
	"@(#) $Id$";

#include <memory.h>
#include <malloc.h>
#include "L1AFrame.h"
#include "Constants.h"

//==========//
// L1AFrame //
//==========//

L1AFrame::L1AFrame()
: time(0), instrumentTicks(0), orbitTicks(0), orbitStep(0),
  priOfOrbitStepChange(255), gcAltitude(0.0), gcLongitude(0.0),
  gcLatitude(0.0), gcX(0.0), gcY(0.0), gcZ(0.0), velX(0.0), velY(0.0),
  velZ(0.0), ptgr(0.0), calPosition(255), loopbackSlices(NULL),
  loopbackNoise(0.0), loadSlices(NULL), loadNoise(0.0),
  antennaPosition(NULL), science(NULL), spotNoise(NULL),
  prf_cycle_time_eu(0),
  range_gate_delay_inner(0),
  range_gate_delay_outer(0),
  range_gate_width_inner(0),
  range_gate_width_outer(0),
  transmit_pulse_width(0),
  true_cal_pulse_pos(0),
  precision_coupler_temp_eu(0),
  rcv_protect_sw_temp_eu(0),
  beam_select_sw_temp_eu(0),
  receiver_temp_eu(0),
  frame_inst_status(0),
  frame_err_status(0),
  frame_qual_flag(0),
  pulse_qual_flag(0),
  frame_time_secs(0.0),
  instrument_time(0.0),
  prf_count(0),
  specified_cal_pulse_pos(0),
  prf_cycle_time(0),
  range_gate_a_delay(0),
  range_gate_a_width(0),
  range_gate_b_delay(0),
  range_gate_b_width(0),
  pulse_width(0),
  pred_antenna_pos_count(0),
  precision_coupler_temp(0),
  rcv_protect_sw_temp(0),
  beam_select_sw_temp(0),
  receiver_temp(0),
  antennaCyclesPerFrame(0), spotsPerFrame(0), slicesPerSpot(0),
  slicesPerFrame(0)
{
    for (int i=0; i < 21; i++) frame_time[i] = 0;
    vtcw[0] = 0;
    vtcw[1] = 0;
	return;
}

L1AFrame::~L1AFrame()
{
	Deallocate();
	return;
}

//---------------//
// L1A::Allocate //
//---------------//

int
L1AFrame::Allocate(
	int		number_of_beams,
	int		antenna_cycles_per_frame,
	int		slices_per_spot)
{
	antennaCyclesPerFrame = antenna_cycles_per_frame;
	spotsPerFrame = number_of_beams * antennaCyclesPerFrame;
	slicesPerSpot = slices_per_spot;
	slicesPerFrame = spotsPerFrame * slicesPerSpot;

	//----------------------------//
	// allocate antenna positions //
	//----------------------------//

	antennaPosition = (unsigned short *)malloc(spotsPerFrame *
		sizeof(unsigned short));
	if (antennaPosition == NULL)
		return(0);

    //---------------------------//
    // allocate cal measurements //
    //---------------------------//

    loopbackSlices = (float *)malloc(slicesPerSpot * sizeof(float));
    if (loopbackSlices == NULL)
    {
        return(0);
    }
    loadSlices = (float *)malloc(slicesPerSpot * sizeof(float));
    if (loadSlices == NULL)
    {
        return(0);
    }

	//-------------------------------//
	// allocate science measurements //
	//-------------------------------//

	science = (float *)malloc(slicesPerFrame * sizeof(float));
	if (science == NULL)
	{
		return(0);
	}
	spotNoise = (float *)malloc(spotsPerFrame * sizeof(float));
	if (spotNoise == NULL)
	{
		return(0);
	}

	return(1);
}

//----------------------//
// L1AFrame::Deallocate //
//----------------------//

int
L1AFrame::Deallocate()
{
	if (antennaPosition)
		free(antennaPosition);
    if (loopbackSlices)
        free(loopbackSlices);
    if (loadSlices)
        free(loadSlices);
	if (science)
		free(science);
	if (spotNoise)
		free(spotNoise);
    antennaPosition = NULL;
    loopbackSlices = NULL;
    loadSlices = NULL;
    science = NULL;
    spotNoise = NULL;
	antennaCyclesPerFrame = 0;
	spotsPerFrame = 0;
	slicesPerSpot = 0;
	slicesPerFrame = 0;
	return(1);
}

//---------------------//
// L1AFrame::FrameSize //
//---------------------//

int
L1AFrame::FrameSize()
{
    int size = 0;
    size += sizeof(double);         // time
    size += sizeof(unsigned int);   // instrument ticks
    size += sizeof(unsigned int);   // orbit ticks
    size += sizeof(unsigned char);  // orbit step
    size += sizeof(unsigned char);  // pri of orbit step change
    size += sizeof(float);          // altitude
    size += sizeof(float);          // longitude
    size += sizeof(float);          // latitude
    size += sizeof(float);          // x
    size += sizeof(float);          // y
    size += sizeof(float);          // z
    size += sizeof(float);          // vx
    size += sizeof(float);          // vy
    size += sizeof(float);          // vz
    size += sizeof(float);          // roll
    size += sizeof(float);          // pitch
    size += sizeof(float);          // yaw
    size += sizeof(float);          // PtGr
    size += sizeof(unsigned char);  // cal position
    size += sizeof(float) * slicesPerSpot;  // loopback slices
    size += sizeof(float);          // loopback noise
    size += sizeof(float) * slicesPerSpot;  // load slices
    size += sizeof(float);          // load noise
    size += sizeof(unsigned short) * spotsPerFrame;  // antenna position
    size += sizeof(float) * slicesPerFrame;  // science data
    size += sizeof(float) * spotsPerFrame;   // spot noise

    size += sizeof(short);  // prf_cycle_time_eu
    size += sizeof(short);  // range_gate_delay_inner
    size += sizeof(short);  // range_gate_delay_outer
    size += sizeof(short);  // range_gate_width_inner
    size += sizeof(short);  // range_gate_width_outer
    size += sizeof(short);  // transmit_pulse_width
    size += sizeof(char);   // true_cal_pulse_pos
    size += sizeof(short);  // precision_coupler_temp_eu
    size += sizeof(short);  // rcv_protect_sw_temp_eu
    size += sizeof(short);  // beam_select_sw_temp_eu
    size += sizeof(short);  // receiver_temp_eu

    size += sizeof(int);    // frame_inst_status
    size += sizeof(int);    // frame_err_status
    size += sizeof(short);  // frame_qual_flag
    size += sizeof(char);   // pulse_qual_flag

    size += sizeof(char)*21;  // frame_time
    size += sizeof(double);   // frame_time_secs
    size += sizeof(double);   // instrument_time
    size += sizeof(char);   // prf_count
    size += sizeof(char);   // specified_cal_pulse_pos
    size += sizeof(char);   // prf_cycle_time
    size += sizeof(char);   // range_gate_a_delay
    size += sizeof(char);   // range_gate_a_width
    size += sizeof(char);   // range_gate_b_delay
    size += sizeof(char);   // range_gate_b_width
    size += sizeof(char);   // pulse_width
    size += sizeof(char);   // pred_antenna_pos_count
    size += sizeof(int)*2;  // vtcw
    size += sizeof(char);   // precision_coupler_temp
    size += sizeof(char);   // rcv_protect_sw_temp
    size += sizeof(char);   // beam_select_sw_temp
    size += sizeof(char);   // receiver_temp

    return(size);
}

//----------------//
// L1AFrame::Pack //
//----------------//

int
L1AFrame::Pack(
	char*	buffer)
{
	int idx = 0;
	int size;

	size = sizeof(double);
	memcpy((void *)(buffer + idx), (void *)&time, size);
	idx += size;

	size = sizeof(unsigned int);
	memcpy((void *)(buffer + idx), (void *)&instrumentTicks, size);
	idx += size;

	memcpy((void *)(buffer + idx), (void *)&orbitTicks, size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)(buffer + idx), (void *)&orbitStep, size);
	idx += size;

	memcpy((void *)(buffer + idx), (void *)&priOfOrbitStepChange, size);
	idx += size;

	size = sizeof(float);
	memcpy((void *)(buffer + idx), (void *)&gcAltitude, size);
	idx += size;

	memcpy((void *)(buffer + idx), (void *)&gcLongitude, size);
	idx += size;

	memcpy((void *)(buffer + idx), (void *)&gcLatitude, size);
	idx += size;

	memcpy((void *)(buffer + idx), (void *)&gcX, size);
	idx += size;

	memcpy((void *)(buffer + idx), (void *)&gcY, size);
	idx += size;

	memcpy((void *)(buffer + idx), (void *)&gcZ, size);
	idx += size;

	memcpy((void *)(buffer + idx), (void *)&velX, size);
	idx += size;

	memcpy((void *)(buffer + idx), (void *)&velY, size);
	idx += size;

	memcpy((void *)(buffer + idx), (void *)&velZ, size);
	idx += size;

	float tmp_float;
	tmp_float = attitude.GetRoll();
	memcpy((void *)(buffer + idx), (void *)&tmp_float, size);
	idx += size;

	tmp_float = attitude.GetPitch();
	memcpy((void *)(buffer + idx), (void *)&tmp_float, size);
	idx += size;

	tmp_float = attitude.GetYaw();
	memcpy((void *)(buffer + idx), (void *)&tmp_float, size);
	idx += size;

	memcpy((void *)(buffer +idx),(void *)&ptgr, size);
	idx += size;

    size = sizeof(unsigned char);
    memcpy((void *)(buffer + idx), (void *)&calPosition, size);
    idx += size;

    size = sizeof(float) * slicesPerSpot;
    memcpy((void *)(buffer + idx), (void *)loopbackSlices, size);
    idx += size;

    size = sizeof(float);
    memcpy((void *)(buffer + idx), (void *)&loopbackNoise, size);
    idx += size;

    size = sizeof(float) * slicesPerSpot;
    memcpy((void *)(buffer + idx), (void *)loadSlices, size);
    idx += size;

    size = sizeof(float);
    memcpy((void *)(buffer + idx), (void *)&loadNoise, size);
    idx += size;

	size = sizeof(unsigned short) * spotsPerFrame;
	memcpy((void *)(buffer + idx), (void *)antennaPosition, size);
	idx += size;

	size = sizeof(float) * slicesPerFrame;
	memcpy((void *)(buffer + idx), (void *)science, size);
	idx += size;

	size = sizeof(float) * spotsPerFrame;
	memcpy((void *)(buffer + idx), (void *)spotNoise, size);
	idx += size;

	size = sizeof(unsigned short);
	memcpy((void *)(buffer + idx), (void *)&prf_cycle_time_eu, size);
	idx += size;

	size = sizeof(unsigned short);
	memcpy((void *)(buffer + idx), (void *)&range_gate_delay_inner, size);
	idx += size;

	size = sizeof(unsigned short);
	memcpy((void *)(buffer + idx), (void *)&range_gate_delay_outer, size);
	idx += size;

	size = sizeof(unsigned short);
	memcpy((void *)(buffer + idx), (void *)&range_gate_width_inner, size);
	idx += size;

	size = sizeof(unsigned short);
	memcpy((void *)(buffer + idx), (void *)&range_gate_width_outer, size);
	idx += size;

	size = sizeof(unsigned short);
	memcpy((void *)(buffer + idx), (void *)&transmit_pulse_width, size);
	idx += size;

	size = sizeof(char);
	memcpy((void *)(buffer + idx), (void *)&true_cal_pulse_pos, size);
	idx += size;

	size = sizeof(short);
	memcpy((void *)(buffer + idx), (void *)&precision_coupler_temp_eu, size);
	idx += size;

	size = sizeof(short);
	memcpy((void *)(buffer + idx), (void *)&rcv_protect_sw_temp_eu, size);
	idx += size;

	size = sizeof(short);
	memcpy((void *)(buffer + idx), (void *)&beam_select_sw_temp_eu, size);
	idx += size;

	size = sizeof(short);
	memcpy((void *)(buffer + idx), (void *)&receiver_temp_eu, size);
	idx += size;

	size = sizeof(unsigned int);
	memcpy((void *)(buffer + idx), (void *)&frame_inst_status, size);
	idx += size;

	size = sizeof(unsigned int);
	memcpy((void *)(buffer + idx), (void *)&frame_err_status, size);
	idx += size;

	size = sizeof(unsigned short);
	memcpy((void *)(buffer + idx), (void *)&frame_qual_flag, size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)(buffer + idx), (void *)&pulse_qual_flag, size);
	idx += size;

	size = sizeof(char) * 21;
	memcpy((void *)(buffer + idx), (void *)frame_time, size);
	idx += size;

	size = sizeof(double);
	memcpy((void *)(buffer + idx), (void *)&frame_time_secs, size);
	idx += size;

	size = sizeof(double);
	memcpy((void *)(buffer + idx), (void *)&instrument_time, size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)(buffer + idx), (void *)&prf_count, size);
	idx += size;

	size = sizeof(char);
	memcpy((void *)(buffer + idx), (void *)&specified_cal_pulse_pos, size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)(buffer + idx), (void *)&prf_cycle_time, size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)(buffer + idx), (void *)&range_gate_a_delay, size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)(buffer + idx), (void *)&range_gate_a_width, size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)(buffer + idx), (void *)&range_gate_b_delay, size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)(buffer + idx), (void *)&range_gate_b_width, size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)(buffer + idx), (void *)&pulse_width, size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)(buffer + idx), (void *)&pred_antenna_pos_count, size);
	idx += size;

	size = sizeof(unsigned int)*2;
	memcpy((void *)(buffer + idx), (void *)vtcw, size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)(buffer + idx), (void *)&precision_coupler_temp, size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)(buffer + idx), (void *)&rcv_protect_sw_temp, size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)(buffer + idx), (void *)&beam_select_sw_temp, size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)(buffer + idx), (void *)&receiver_temp, size);
	idx += size;

	return(idx);
}

//------------------//
// L1AFrame::Unpack //
//------------------//

int
L1AFrame::Unpack(
	char*	buffer)
{
	int idx = 0;
	int size;

	size = sizeof(double);
	memcpy((void *)&time, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned int);
	memcpy((void *)&instrumentTicks, (void *)(buffer + idx), size);
	idx += size;

	memcpy((void *)&orbitTicks, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)&orbitStep, (void *)(buffer + idx), size);
	idx += size;

	memcpy((void *)&priOfOrbitStepChange, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(float);
	memcpy((void *)&gcAltitude, (void *)(buffer + idx), size);
	idx += size;

	memcpy((void *)&gcLongitude, (void *)(buffer + idx), size);
	idx += size;

	memcpy((void *)&gcLatitude, (void *)(buffer + idx), size);
	idx += size;

	memcpy((void *)&gcX, (void *)(buffer + idx), size);
	idx += size;

	memcpy((void *)&gcY, (void *)(buffer + idx), size);
	idx += size;

	memcpy((void *)&gcZ, (void *)(buffer + idx), size);
	idx += size;

	memcpy((void *)&velX, (void *)(buffer + idx), size);
	idx += size;

	memcpy((void *)&velY, (void *)(buffer + idx), size);
	idx += size;

	memcpy((void *)&velZ, (void *)(buffer + idx), size);
	idx += size;

	float tmp_float;
	memcpy((void *)&tmp_float, (void *)(buffer + idx), size);
	attitude.SetRoll(tmp_float);
	idx += size;

	memcpy((void *)&tmp_float, (void *)(buffer + idx), size);
	attitude.SetPitch(tmp_float);
	idx += size;

	memcpy((void *)&tmp_float, (void *)(buffer + idx), size);
	attitude.SetYaw(tmp_float);
	idx += size;

	memcpy((void *)&ptgr, (void *)(buffer + idx), size);
	idx += size;

    size = sizeof(unsigned char);
    memcpy((void *)&calPosition, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(float) * slicesPerSpot;
    memcpy((void *)loopbackSlices, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(float);
    memcpy((void *)&loopbackNoise, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(float) * slicesPerSpot;
    memcpy((void *)loadSlices, (void *)(buffer + idx), size);
    idx += size;

    size = sizeof(float);
    memcpy((void *)&loadNoise, (void *)(buffer + idx), size);
    idx += size;

	size = sizeof(unsigned short) * spotsPerFrame;
	memcpy((void *)antennaPosition, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(float) * slicesPerFrame;
	memcpy((void *)science, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(float) * spotsPerFrame;
	memcpy((void *)spotNoise, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned short);
	memcpy((void *)&prf_cycle_time_eu, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned short);
	memcpy((void *)&range_gate_delay_inner, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned short);
	memcpy((void *)&range_gate_delay_outer, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned short);
	memcpy((void *)&range_gate_width_inner, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned short);
	memcpy((void *)&range_gate_width_outer, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned short);
	memcpy((void *)&transmit_pulse_width, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(char);
	memcpy((void *)&true_cal_pulse_pos, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(short);
	memcpy((void *)&precision_coupler_temp_eu, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(short);
	memcpy((void *)&rcv_protect_sw_temp_eu, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(short);
	memcpy((void *)&beam_select_sw_temp_eu, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(short);
	memcpy((void *)&receiver_temp_eu, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned int);
	memcpy((void *)&frame_inst_status, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned int);
	memcpy((void *)&frame_err_status, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned short);
	memcpy((void *)&frame_qual_flag, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)&pulse_qual_flag, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(char) * 21;
	memcpy((void *)frame_time, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(double);
	memcpy((void *)&frame_time_secs, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(double);
	memcpy((void *)&instrument_time, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)&prf_count, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(char);
	memcpy((void *)&specified_cal_pulse_pos, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)&prf_cycle_time, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)&range_gate_a_delay, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)&range_gate_a_width, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)&range_gate_b_delay, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)&range_gate_b_width, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)&pulse_width, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)&pred_antenna_pos_count, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned int)*2;
	memcpy((void *)vtcw, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)&precision_coupler_temp, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)&rcv_protect_sw_temp, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)&beam_select_sw_temp, (void *)(buffer + idx), size);
	idx += size;

	size = sizeof(unsigned char);
	memcpy((void *)&receiver_temp, (void *)(buffer + idx), size);
	idx += size;

	return(idx);
}


//--------------------------//
// L1AFrame::WriteAscii     //
//--------------------------//

int L1AFrame::WriteAscii(FILE* ofp){
  fprintf(ofp,"\n########################Frame Info#####################\n\n");
  fprintf(ofp,"Time: %g InstrumentTicks: %d OrbitTicks %d PriOfOrbitStepChange %d\n",
	  time,instrumentTicks,orbitTicks,(int)priOfOrbitStepChange);
  fprintf(ofp,"GCAlt: %g GCLon: %g GCLat: %g GCX: %g GCY: %g GCZ: %g\n",
	  gcAltitude, gcLongitude*rtd, gcLatitude*rtd, gcX, gcY,gcZ);
  fprintf(ofp,"VelX: %g VelY: %g VelZ: %g Roll: %g Pitch: %g Yaw: %g PtGr: %g\n",
	  velX,velY,velZ,attitude.GetRoll()*rtd,attitude.GetPitch()*rtd,attitude.GetYaw()*rtd,ptgr);
  int offset=0;
  for(int c=0;c<spotsPerFrame;c++){
    fprintf(ofp,"\n    :::::::::::::::: Spot Info :::::::::::::::::::  \n\n");
    fprintf(ofp, "AntennaPos: %d SpotNoise: %g Beam:%d\n",
	    (int)antennaPosition[c],spotNoise[c],c%2);
    fprintf(ofp,"E(S+N) Slices(1-%d): ",slicesPerSpot);
    for(int s=0;s<slicesPerSpot;s++){
      offset++;
      fprintf(ofp,"%g ",science[offset]);
    }
    fprintf(ofp,"\n");
  }
  return(1);
}
