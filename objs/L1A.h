//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef L1A_H
#define L1A_H

static const char rcs_id_l1a_h[] =
	"@(#) $Id$";

#include "BaseFile.h"
#include "L1AFrame.h"
#include "L1AGSFrame.h"


//======================================================================
// CLASSES
//		L1A
//======================================================================

//======================================================================
// CLASS
//		L1A
//
// DESCRIPTION
//		The L1A object allows for the easy writing, reading, and
//		manipulating of Level 1A data.
//======================================================================

class L1A : public BaseFile
{
public:

	//------//
	// enum //
	//------//

	enum StatusE { OK, ERROR_READING_FRAME, ERROR_UNKNOWN };

	//--------------//
	// construction //
	//--------------//

	L1A();
	~L1A();

	int		AllocateBuffer();
	int		DeallocateBuffer();

	//---------------------//
	// setting and getting //
	//---------------------//

	StatusE		GetStatus() { return(_status); };

	//--------------//
	// input/output //
	//--------------//

    int  ReadDataRec();
    int  WriteDataRec();
    int  WriteDataRecAscii();

    int  ReadGSDataRec(void);
    int  WriteGSDataRec(void);
    int  FillGSFrame(void);
    int  WriteGSDataRecAscii(void);

	int  OpenCalPulseForWriting(const char* filename);
    int  ReadGSCalPulseRec(FILE* calfile);
    int  WriteGSCalPulseRec(void);
    int  WriteGSCalPulseRecAscii(void);
    int  CloseCalPulseFile(void);

	//-----------//
	// variables //
	//-----------//

	char*		buffer;
	int			bufferSize;
	L1AFrame	frame;

	char*		gsBuffer;
    L1AGSFrame  gsFrame;
//    GSCalPulse  calPulse;

protected:

	//-----------//
	// variables //
	//-----------//

	StatusE		_status;

    FILE*       _calPulseFP;
};

//-------------------------------------//
// this is for calibration pulse files //
//-------------------------------------//

#define SLICES_PER_PULSE  12

typedef struct l1a_calibration_pulse_type
{
    double        frame_time_cal_secs;
    char          true_cal_pulse_pos;
    char          beam_identifier;
    unsigned int  loop_back_cal_power[SLICES_PER_PULSE];
    unsigned int  loop_back_cal_noise;
    unsigned int  load_cal_power[SLICES_PER_PULSE];
    unsigned int  load_cal_noise;
    float         rj_temp_eu;
    float         precision_coupler_temp_eu;
    float         rcv_protect_sw_temp_eu;
    float         beam_select_sw_temp_eu;
    float         receiver_temp_eu;
    float         transmit_power_inner;
    float         transmit_power_outer;
    unsigned int  frame_inst_status;
    unsigned int  frame_err_status;
    short         frame_qual_flag;
} L1A_Calibration_Pulse_Type;

#endif
