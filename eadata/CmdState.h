//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.0   04 Feb 1998 14:14:54   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:28:10  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef CMDSTATE_H
#define CMDSTATE_H

static const char rcsid_CmdState_h[] =
    "@(#) $Header$";

#include "Command.h"
#include "Parameter.h"

//=========
// DEFINES 
//=========

#define TRIP_DURATION_LIMIT_NA      -1
#define DEFAULT_TRIP_DURATION_LIMIT 1069
#define DEFAULT_ANTENNA_SEQUENCE    "1V 6V 2V 5V 2H 5H 3V 4V (default)"
#define DEFAULT_BINNING_CONSTANTS   "(default)"
#define ANTENNA_SEQUENCE_NA         "(N/A)"
#define BINNING_CONSTANTS_NA        "(N/A)"

#define RELAY_COUNT                 15      // relay 0 is wasted
#define RELAY_STATE_COUNT           2       // set and reset
#define WTS_STATE_COUNT             2       // 1 and 2
#define HVPS_COUNT                  2       // corresponds to the TWTA's
#define HVPS_STATE_COUNT            2       // on and off

//===========
// CONSTANTS 
//===========

extern const char* mode_state_map[];
extern const char* dss_state_map[];
extern const char* twta_state_map[];
extern const char* hvps_state_map[];
extern const char* rh_state_map[];
extern const char* sh_state_map[];
extern const char* wts_state_map[];
extern const char* twta_trip_ovr_state_map[];
extern const char* twt_mon_state_map[];
extern const char* hvps_shutdown_state_map[];

//==========================
// Initial Instrument State 
//==========================

#define INITIAL_K1                  RELAY_SET
#define INITIAL_K2                  RELAY_SET
#define INITIAL_K3                  RELAY_SET
#define INITIAL_K4                  RELAY_SET
#define INITIAL_K5                  RELAY_SET
#define INITIAL_K6                  RELAY_SET
#define INITIAL_K7                  RELAY_SET
#define INITIAL_K8                  RELAY_SET
#define INITIAL_K9                  RELAY_SET
#define INITIAL_K10                 RELAY_SET
#define INITIAL_K11                 RELAY_SET
#define INITIAL_K12                 RELAY_SET
#define INITIAL_K13                 RELAY_SET
#define INITIAL_K14                 RELAY_SET
#define INITIAL_MODE                MODE_NA
#define INITIAL_TWTA_TRIP_OVR       TWTA_TRIP_OVR_NA
#define INITIAL_TWT_MON             TWT_MON_NA
#define INITIAL_HVPS_SHUTDOWN       HVPS_SHUTDOWN_NA
#define INITIAL_TRIP_DURATION_LIMIT TRIP_DURATION_LIMIT_NA
#define INITIAL_WTS                 WTS_TWTA1

//---------------------------------------------------------
// The CmdState class contains the state of the instrument as it
// directly pertains to commands.  Thus, any command only directly
// affects the state variables contained within the CmdState class.
// The CmdState class is used to evaluate the state of the instrument
// given a command.  It it also used to derive effects from commands.
//---------------------------------------------------------

class CmdState
{
public:
    enum ElectronicsE { ELECTRONICS_ON, ELECTRONICS_OFF,
        ELECTRONICS_UNKNOWN };
    enum ReplacementHeaterE { REPLACEMENT_HEATER_ENABLED,
        REPLACEMENT_HEATER_DISABLED, REPLACEMENT_HEATER_UNKNOWN };
    enum SpareHeaterE { SPARE_HEATER_ENABLED, SPARE_HEATER_DISABLED,
        SPARE_HEATER_UNKNOWN };
    enum ModeE { MODE_SBM, MODE_ROM, MODE_CCM, MODE_DBM, MODE_WOM,
        MODE_UNKNOWN, MODE_NA };
    enum DssE { DSS_A, DSS_B, DSS_UNKNOWN };
    enum HvpsE { HVPS_ON, HVPS_OFF, HVPS_UNKNOWN };
    enum TwtaE { TWTA_1, TWTA_2, TWTA_UNKNOWN };
    enum RelayE { RELAY_SET, RELAY_RESET, RELAY_UNKNOWN };
    enum TwtaTripOvrE { TWTA_TRIP_OVR_DISABLED, TWTA_TRIP_OVR_ENABLED,
        TWTA_TRIP_OVR_UNKNOWN, TWTA_TRIP_OVR_NA };
    enum TwtMonE { TWT_MON_DISABLED, TWT_MON_ENABLED,
        TWT_MON_UNKNOWN, TWT_MON_NA };
    enum HvpsShutdownE { HVPS_SHUTDOWN_DISABLED, HVPS_SHUTDOWN_ENABLED,
        HVPS_SHUTDOWN_UNKNOWN, HVPS_SHUTDOWN_NA };
    enum WtsE { WTS_TWTA1, WTS_TWTA2, WTS_UNKNOWN };

    CmdState();
    ~CmdState();

    const char*     GetAntennaSequence() { return(antennaSequence); };
    const char*     GetBinningConstants() { return(binningConstants); };
    TwtaTripOvrE    GetTwtaBocTrip() { return(twtaTripOvrEnable); };
    TwtMonE         GetTwtMon() { return(twtMonEnable); };
    HvpsShutdownE   GetHvpsShutdown() { return(hvpsShutdownEnable); };
    int             GetTripDurationLimit() { return(tripDurationLimit); };
    void            ClearCounters();
    unsigned int    GetRelayCount(int relay_index, RelayE relay_state)
                        { return (k_counter[relay_index][relay_state]); };
    unsigned int    GetWtsCount(WtsE wts_state)
                        { return (wts_counter[wts_state]); };
    unsigned int    GetHvpsCount(TwtaE hvps_index, HvpsE hvps_state)
                        { return (hvps_counter[hvps_index][hvps_state]); };

    EffectE         ApplyCmd(Command *command);

    ElectronicsE            ElectronicsState();
    ReplacementHeaterE      ReplacementHeaterState();
    DssE                    DssState();
    ModeE                   ModeState();
    TwtaE                   TwtaState();
    HvpsE                   HvpsState();
    SpareHeaterE            SpareHeaterState();
    WtsE                    WtsState();

    CmdState&   operator=(const CmdState& other);

private:
    int         _SetAntennaSequence(const char* string);
    int         _SetBinningConstants(const char* string);
    void        _ModRelay(int relay_index, RelayE relay_state);
    void        _ModWts(WtsE wts_state);

    EffectE     _PowerEffect(CmdState *oldCmdState);
    EffectE     _HeaterEffect(CmdState *oldCmdState);
    EffectE     _DssEffect(CmdState *oldCmdState);
    EffectE     _ModeEffect(CmdState *oldCmdState);
    EffectE     _TwtaEffect(CmdState *oldCmdState);
    EffectE     _HvpsEffect(CmdState *oldCmdState);
    EffectE     _SpareHeaterEffect(CmdState *oldCmdState);
    EffectE     _WtsEffect(CmdState *oldCmdState);

    //----------------------
    // the instrument state 
    //----------------------

    RelayE          k[RELAY_COUNT]; // relay k[0] is wasted
    ModeE           mode;
    TwtaTripOvrE    twtaTripOvrEnable;
    TwtMonE         twtMonEnable;
    HvpsShutdownE   hvpsShutdownEnable;
    int             tripDurationLimit;
    WtsE            wts;
    char*           antennaSequence;
    char*           binningConstants;

    //----------
    // counters 
    //----------

    unsigned int    k_counter[RELAY_COUNT][RELAY_STATE_COUNT];
    unsigned int    wts_counter[WTS_STATE_COUNT];
    unsigned int    hvps_counter[HVPS_COUNT][HVPS_STATE_COUNT];
};

//-----------------------
// relay state functions 
//-----------------------

int     Eval_3k(const int relay1, const int relay2, const int relay3,
            const int retval[3]);
int     Eval_2k(const int relay1, const int relay2,
            const int retval[3]);

//===============
// Lookup Tables 
//===============

extern const CmdState::ReplacementHeaterE replacement_heater_state_k4_k5_k6[];
extern const CmdState::DssE dss_state_k7_k8[];
extern const CmdState::SpareHeaterE spare_heater_state_k13_k14[];
extern const CmdState::TwtaE twta_state_k9_k10[];
extern const CmdState::HvpsE hvps_state_k11_k12[];


#endif
