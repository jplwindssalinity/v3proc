//=========================================================
// Copyright  (C)1998, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.1   19 Aug 1998 14:36:50   daffer
// cmdlp work
// 
//    Rev 1.0   22 May 1998 16:53:48   daffer
// Initial Revision
// 
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef CmdEffect_H
#define CmdEffect_H

static const char rcsid_CmdEffect_h[] =
    "@(#) $Header$";

#include "Command.h"
#include "Parameter.h"

//=========
// DEFINES 
//=========

// #define TRIP_DURATION_LIMIT_NA      -1
// #define DEFAULT_TRIP_DURATION_LIMIT 1069
// #define DEFAULT_ANTENNA_SEQUENCE    "1V 6V 2V 5V 2H 5H 3V 4V (default)"
// #define DEFAULT_BINNING_CONSTANTS   "(default)"
// #define ANTENNA_SEQUENCE_NA         "(N/A)"
// #define BINNING_CONSTANTS_NA        "(N/A)"

#define RELAY_COUNT                 22      // relay 0 is wasted
#define RELAY_EFFECT_COUNT           2       // set and reset
//#define TRS_EFFECT_COUNT             2       // 1 and 2
#define TWTAELECT_COUNT              2       // corresponds to the TWTA's
#define TWTAELECT_EFFECT_COUNT       2       // on and off

//===========
// CONSTANTS 
//===========

extern const char* mode_effect_map[];
extern const char* cds_effect_map[];
extern const char* twta_effect_map[];
extern const char* hvps_effect_map[];
extern const char* rh_effect_map[];
extern const char* sh_effect_map[];
extern const char* trs_effect_map[];
extern const char* twta_trip_ovr_effect_map[];
extern const char* twt_mon_effect_map[];
extern const char* hvps_shutdown_effect_map[];

//==========================
// Initial Instrument Effect 
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
#define INITIAL_K15                 RELAY_SET
#define INITIAL_K16                 RELAY_SET
#define INITIAL_K17                 RELAY_SET
#define INITIAL_K18                 RELAY_SET
#define INITIAL_K19                 RELAY_SET
#define INITIAL_K20                 RELAY_SET
#define INITIAL_K21                 RELAY_SET
#define INITIAL_K22                 RELAY_SET
#define INITIAL_MODE                MODE_NA
#define INITIAL_TWTA_TRIP_OVR       TWTA_TRIP_OVR_NA
#define INITIAL_TWT_MON             TWT_MON_NA
#define INITIAL_TRIP_DURATION_LIMIT TRIP_DURATION_LIMIT_NA

//---------------------------------------------------------
// The CmdEffect class contains the state of the instrument as it
// directly pertains to commands.  Thus, any command only directly
// affects the state variables contained within the CmdEffect class.
// The CmdEffect class is used to evaluate the state of the instrument
// given a command.  It it also used to derive effects from commands.
//---------------------------------------------------------

class CmdEffect
{
public:
    enum ElectronicsE { ELECTRONICS_ON, 
                        ELECTRONICS_OFF,
                        ELECTRONICS_UNKNOWN };

    enum ReplacementHeaterE { REPLACEMENT_HEATERS_ON,
                              REPLACEMENT_HEATERS_OFF, 
                              REPLACEMENT_HEATERS_UNKNOWN };

    enum SesElectronicsE { SES_ELECTRONICS_ON, 
                           SES_REPLACEMENT_HEATER_ON, 
                           SES_ELECTRONICS_UNKNOWN};

    enum SasElectronicsE { SAS_ELECTRONICS_ON, 
                           SAS_REPLACEMENT_HEATER_ON, 
                           SAS_ELECTRONICS_UNKNOWN};

    enum TwtaElectronicsE { TWTA_ELECTRONICS_ON, 
                            TWTA_REPLACEMENT_HEATER_ON, 
                            TWTA_ELECTRONICS_UNKNOWN };

    enum SesSupplHeaterE { SES_SUPPL_HEATER_ON, 
                           SES_SUPPL_HEATER_OFF,
                           SES_SUPPL_HEATER_UNKNOWN };

    enum SesE { SES_A, SES_B, SES_UNKNOWN };
    enum SasE { SAS_A, SAS_B, SAS_UNKNOWN };

    enum ModeE { MODE_STB, MODE_RCV, MODE_CAL, MODE_WOM,
        MODE_UNKNOWN, MODE_NA };

    enum CdsE { CDS_A, CDS_B, CDS_UNKNOWN };

    enum TwtaE { TWTA_1, TWTA_2, TWTA_UNKNOWN };
    enum RelayE { RELAY_SET, RELAY_RESET, RELAY_UNKNOWN };
    enum TwtaTripOvrE { TWTA_TRIP_OVR_DISABLED, 
                        TWTA_TRIP_OVR_ENABLED,
                        TWTA_TRIP_OVR_UNKNOWN, 
                        TWTA_TRIP_OVR_NA };
    enum TwtMonE { TWT_MON_DISABLED, 
                   TWT_MON_ENABLED,
                   TWT_MON_UNKNOWN, 
                   TWT_MON_NA };

    enum TwtaLowDrivePowerFPE { TWT_LDPFP_ENABLED,
                                TWT_LDPFP_DISABLED,
                                TWT_LDPFP_UNKNOWN,
                                TWT_LDPFP_NA};

    //    enum HvpsShutdownE { HVPS_SHUTDOWN_DISABLED, HVPS_SHUTDOWN_ENABLED,
    //        HVPS_SHUTDOWN_UNKNOWN, HVPS_SHUTDOWN_NA };

    // enum TrsE { TRS_TWTA1, TRS_TWTA2, TRS_UNKNOWN };

    CmdEffect();
    ~CmdEffect();

    TwtaTripOvrE    GetTwtaBocTrip() { return(twtaTripOvrEnable); };
    TwtMonE         GetTwtMon() { return(twtMonEnable); };
//    int             GetTripDurationLimit() { return(tripDurationLimit); };
    void            ClearCounters();
//     unsigned int    GetRelayCount(int relay_index, RelayE relay_effect)
//                         { return (k_counter[relay_index][relay_effect]); };
//     unsigned int    GetTrsCount(TrsE trs_effect)
//                         { return (trs_counter[trs_effect]); };
//     unsigned int    GetHvpsCount(TwtaE hvps_index, HvpsE hvps_effect)
//                    { return (hvps_counter[hvps_index][hvps_effect]); };
    //    const char*     GetAntennaSequence() { return(antennaSequence); };
    //    const char*     GetBinningConstants() { return(binningConstants); };
    //    HvpsShutdownE   GetHvpsShutdown() { return(hvpsShutdownEnable); };

    EffectE         ApplyCmd(Command *command);

    ElectronicsE            ElectronicsEffect();
    SesElectronicsE         SesElectronicsEffect();
    SasElectronicsE         SasElectronicsEffect();
    TwtaElectronicsE        TwtaElectronicsEffect();
    ReplacementHeaterE      ReplacementHeaterEffect();
    SesE                    SesEffect();
    SasE                    SasEffect();
    CdsE                    CdsEffect();
    ModeE                   ModeEffect();
    TwtaE                   TwtaEffect();
    SesSupplHeaterE         SesSupplHeaterEffect();
    //    TrsE                    TrsEffect();

    CmdEffect&   operator=(const CmdEffect& other);
    //    HvpsE                   HvpsEffect();

private:
    void        _ModRelay(int relay_index, RelayE relay_effect);
    //    void        _ModTrs(TrsE trs_effect);

    EffectE     _PowerEffect  (CmdEffect *oldCmdEffect);
      // All 3 replacement heaters
    EffectE     _HeaterEffect (CmdEffect *oldCmdEffect); 
      // SES select
    EffectE     _SesEffect    (CmdEffect *oldCmdEffect);
      // SAS select
    EffectE     _SasEffect    (CmdEffect *oldCmdEffect);
      // CDS select
    EffectE     _CdsEffect    (CmdEffect *oldCmdEffect);
      // Mode
    EffectE     _ModeEffect   (CmdEffect *oldCmdEffect);
      // TWTA Select
    EffectE     _TwtaEffect   (CmdEffect *oldCmdEffect);
      // TWTA on/ TWTA Replacement heaters on.
    EffectE     _TwtaElectronicsEffect(CmdEffect *oldCmdEffect);
      // SES on/ SES Replacement heaters on.
    EffectE     _SesElectronicsEffect(CmdEffect *oldCmdEffect);
      // SAS on/ SAS Replacement heaters on.
    EffectE     _SasElectronicsEffect(CmdEffect *oldCmdEffect);
      // SES Supplemental heater on/off
    EffectE     _SupplHeaterEffect    (CmdEffect *oldCmdEffect);
    EffectE     _TrsEffect             (CmdEffect *oldCmdEffect);

    //    int         _SetAntennaSequence(const char* string);
    //    int         _SetBinningConstants(const char* string);

    //----------------------
    // the instrument effect 
    //----------------------

    RelayE          k[RELAY_COUNT]; // relay k[0] is wasted
    ModeE           mode;
    TwtaTripOvrE    twtaTripOvrEnable;
    TwtMonE         twtMonEnable;
//     SesElectronicsE ses_electrics;
//     SasElectronicsE sas_electrics;
//     TwtaElectronicsE twta_electrics;
//     SasE            sas;
//     SesE            ses;
//     CdsE            cds;
//     Twta            twta;
    
    //    int             tripDurationLimit;
    //    TrsE            trs;

    //char*           antennaSequence;
    //char*           binningConstants;
    //    HvpsShutdownE   hvpsShutdownEnable;

    //----------
    // counters 
    //----------

    unsigned int    k_counter[RELAY_COUNT][RELAY_EFFECT_COUNT];
//    unsigned int    trs_counter[TRS_EFFECT_COUNT];
    unsigned int    twtaelect_counter[TWTAELECT_COUNT][TWTAELECT_EFFECT_COUNT];
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

// extern const CmdEffect::ReplacementHeaterE replacement_heater_effect_k4_k5_k6[];
// extern const CmdEffect::CdsE cds_effect_k7_k8[];
// extern const CmdEffect::SupplHeaterE ses_suppl_heater_effect_k21_k22[];
// extern const CmdEffect::TwtaE twta_effect_k11_k12[];
// extern const CmdEffect::HvpsE twta_electronics_effect_k9_k10[];


#endif
