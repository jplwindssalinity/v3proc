//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.5   22 May 1998 16:41:34   daffer
// Changed EFF_ enum
// 
//    Rev 1.4   24 Mar 1998 15:56:52   sally
// de-warn for GNU
// 
//    Rev 1.3   24 Mar 1998 09:23:50   sally
// de-warn for GNU C++
// 
//    Rev 1.2   09 Mar 1998 16:34:08   sally
// adapt to the new REQI format
// 
//    Rev 1.1   26 Feb 1998 09:45:42   sally
// to pacify GNU C++ compiler
// 
//    Rev 1.0   04 Feb 1998 14:14:52   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:28:55  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

static const char rcsid_CmdState_C[] =
    "@(#) $Header$";

#include <stdlib.h>
#include <string.h>
#include "CmdState.h"
#include "Command.h"
#include "Parameter.h"

//===============
// State Defines 
//===============

#define MOSTLY_RESET    0   // DO NOT CHANGE!
#define MOSTLY_SET      1   // These are indicies to the const arrays below

#define MATCH       0   // DO NOT CHANGE!
#define DIFFER      1   // These are indicies to the const arrays below

#define UNKNOWN     2   // DO NOT CHANGE!

//===============
// Lookup Tables 
//===============

const CmdState::ElectronicsE electronics_state_k1_k2_k3[] =
    { CmdState::ELECTRONICS_ON, CmdState::ELECTRONICS_OFF,
    CmdState::ELECTRONICS_UNKNOWN };

const CmdState::ReplacementHeaterE replacement_heater_state_k4_k5_k6[] =
    { CmdState::REPLACEMENT_HEATER_ENABLED,
    CmdState::REPLACEMENT_HEATER_DISABLED,
    CmdState::REPLACEMENT_HEATER_UNKNOWN };

const CmdState::CdsE cds_state_k7_k8[] =
    { CmdState::CDS_A, CmdState::CDS_B, CmdState::CDS_UNKNOWN };

const CmdState::TwtaE twta_state_k9_k10[] =
    { CmdState::TWTA_1, CmdState::TWTA_2, CmdState::TWTA_UNKNOWN };

const CmdState::HvpsE hvps_state_k11_k12[] =
    { CmdState::HVPS_ON, CmdState::HVPS_OFF, CmdState::HVPS_UNKNOWN };

const CmdState::SpareHeaterE spare_heater_state_k13_k14[] =
    { CmdState::SPARE_HEATER_ENABLED, CmdState::SPARE_HEATER_DISABLED,
    CmdState::SPARE_HEATER_UNKNOWN };

const char* mode_state_map[] = { "SBM", "ROM", "CCM", "DBM", "WOM", "?",
    "N/A" };
const char* cds_state_map[] = { "CDS A", "CDS B", "?" };
const char* twta_state_map[] = { "TWTA #1", "TWTA #2", "?" };
const char* hvps_state_map[] = { "HVPS On", "HVPS Off", "?" };
const char* rh_state_map[] = { "Enabled", "Disabled", "?" };
const char* sh_state_map[] = { "Enabled", "Disabled", "?" };
const char* trs_state_map[] = { "TWTA #1",  "TWTA #2", "?" };
const char* twta_trip_ovr_state_map[] = { "Disabled", "Enabled", "?" };
const char* twt_mon_state_map[] = { "Disabled", "Enabled", "?" };
const char* hvps_shutdown_state_map[] = { "Disabled", "Enabled", "?" };

//==================
// CmdState methods 
//==================

CmdState::CmdState()
:   mode(INITIAL_MODE), twtaTripOvrEnable(INITIAL_TWTA_TRIP_OVR),
    twtMonEnable(INITIAL_TWT_MON),
    hvpsShutdownEnable(INITIAL_HVPS_SHUTDOWN),
    tripDurationLimit(INITIAL_TRIP_DURATION_LIMIT), trs(INITIAL_TRS),
    antennaSequence(NULL), binningConstants(NULL)
{
    k[1] = INITIAL_K1;
    k[2] = INITIAL_K2;
    k[3] = INITIAL_K3;
    k[4] = INITIAL_K4;
    k[5] = INITIAL_K5;
    k[6] = INITIAL_K6;
    k[7] = INITIAL_K7;
    k[8] = INITIAL_K8;
    k[9] = INITIAL_K9;
    k[10] = INITIAL_K10;
    k[11] = INITIAL_K11;
    k[12] = INITIAL_K12;
    k[13] = INITIAL_K13;
    k[14] = INITIAL_K14;

    ClearCounters();

    return;
}

CmdState::~CmdState()
{
    return;
}

//---------------
// ClearCounters 
//---------------

void
CmdState::ClearCounters()
{
    for (int r = 0; r < RELAY_COUNT; r++)
        for (int rs = 0; rs < RELAY_STATE_COUNT; rs++)
            k_counter[r][rs] = 0;

    for (int w = 0; w < TRS_STATE_COUNT; w++)
        trs_counter[w] = 0;

    for (int h = 0; h < HVPS_COUNT; h++)
        for (int hs = 0; hs < HVPS_STATE_COUNT; hs++)
            hvps_counter[h][hs] = 0;

    return;
}

//----------
// ApplyCmd 
//----------
// given a command, ApplyCmd returns the effect of the command
// and updates the CmdState based on the command.

EffectE
CmdState::ApplyCmd(
Command*    )
#if 0
Command     *command)
#endif
{
    //------------------------------------
    // make sure the command is effective 
    //------------------------------------

#if 0 // LATER
    if (! command->Effective())
        return(EFF_NONE);

    //----------------------------------------
    // make a temporary copy of the old state 
    //----------------------------------------

    CmdState oldCmdState = *this;

    //-******************************
    // APPLY PULSE DISCRETE COMMANDS 
    //-******************************

    switch (command->commandId)
    {
        //---------------------------
        // power relays (k1, k2, k3) 
        //---------------------------

        case CMD_K1S:
            _ModRelay(1, RELAY_SET);
            return(_PowerEffect(&oldCmdState));
            break;
        case CMD_K1R:
            _ModRelay(1, RELAY_RESET);
            return(_PowerEffect(&oldCmdState));
            break;
        case CMD_K2S:
            _ModRelay(2, RELAY_SET);
            return(_PowerEffect(&oldCmdState));
            break;
        case CMD_K2R:
            _ModRelay(2, RELAY_RESET);
            return(_PowerEffect(&oldCmdState));
            break;
        case CMD_K3S:
            _ModRelay(3, RELAY_SET);
            return(_PowerEffect(&oldCmdState));
            break;
        case CMD_K3R:
            _ModRelay(3, RELAY_RESET);
            return(_PowerEffect(&oldCmdState));
            break;

        //---------------------------
        // replacement heater relays 
        //---------------------------

        case CMD_K4S:
            _ModRelay(4, RELAY_SET);
            return(_HeaterEffect(&oldCmdState));
            break;
        case CMD_K4R:
            _ModRelay(4, RELAY_RESET);
            return(_HeaterEffect(&oldCmdState));
            break;
        case CMD_K5S:
            _ModRelay(5, RELAY_SET);
            return(_HeaterEffect(&oldCmdState));
            break;
        case CMD_K5R:
            _ModRelay(5, RELAY_RESET);
            return(_HeaterEffect(&oldCmdState));
            break;
        case CMD_K6S:
            _ModRelay(6, RELAY_SET);
            return(_HeaterEffect(&oldCmdState));
            break;
        case CMD_K6R:
            _ModRelay(6, RELAY_RESET);
            return(_HeaterEffect(&oldCmdState));
            break;

        //------------
        // cds relays 
        //------------

        case CMD_K7S:
            _ModRelay(7, RELAY_SET);
            return(_CdsEffect(&oldCmdState));
            break;
        case CMD_K7R:
            _ModRelay(7, RELAY_RESET);
            return(_CdsEffect(&oldCmdState));
            break;
        case CMD_K8S:
            _ModRelay(8, RELAY_SET);
            return(_CdsEffect(&oldCmdState));
            break;
        case CMD_K8R:
            _ModRelay(8, RELAY_RESET);
            return(_CdsEffect(&oldCmdState));
            break;

        //---------------------
        // spare heater relays 
        //---------------------

        case CMD_K13S:
            _ModRelay(13, RELAY_SET);
            return(_SpareHeaterEffect(&oldCmdState));
            break;
        case CMD_K13R:
            _ModRelay(13, RELAY_RESET);
            return(_SpareHeaterEffect(&oldCmdState));
            break;
        case CMD_K14S:
            _ModRelay(14, RELAY_SET);
            return(_SpareHeaterEffect(&oldCmdState));
            break;
        case CMD_K14R:
            _ModRelay(14, RELAY_RESET);
            return(_SpareHeaterEffect(&oldCmdState));
            break;
        default:
            break;
    }

    //---------------------------------
    // check that the instrument is on 
    //---------------------------------

    ElectronicsE elec_state = ElectronicsState();
    if (elec_state == ELECTRONICS_OFF)
        return(EFF_NONE);
    if (elec_state == ELECTRONICS_UNKNOWN)
        return(EFF_UNKNOWN);

    //-********************************
    // APPLY SERIAL MAGNITUDE COMMANDS 
    //-********************************
    
    // the electronics are on!
    switch (command->commandId)
    {
        //------
        // mode 
        //------
        case CMD_SBM:
            mode = MODE_SBM;
            return(_ModeEffect(&oldCmdState));
            break;
        case CMD_ROM:
            mode = MODE_ROM;
            return(_ModeEffect(&oldCmdState));
            break;
        case CMD_CCM:
            mode = MODE_CCM;
            return(_ModeEffect(&oldCmdState));
            break;
        case CMD_DBM:
            mode = MODE_DBM;
            return(_ModeEffect(&oldCmdState));
            break;
        case CMD_WOM:
            mode = MODE_WOM;
            return(_ModeEffect(&oldCmdState));
            break;

        //-------------
        // twta relays 
        //-------------
        case CMD_K9S:
            _ModRelay(9, RELAY_SET);
            return(_TwtaEffect(&oldCmdState));
            break;
        case CMD_K9R:
            _ModRelay(9, RELAY_RESET);
            return(_TwtaEffect(&oldCmdState));
            break;
        case CMD_K10S:
            _ModRelay(10, RELAY_SET);
            return(_TwtaEffect(&oldCmdState));
            break;
        case CMD_K10R:
            _ModRelay(10, RELAY_RESET);
            return(_TwtaEffect(&oldCmdState));
            break;

        //-------------------------------------
        // twta body overcurrent trip override 
        //-------------------------------------
        case CMD_OVR:
            twtaTripOvrEnable = TWTA_TRIP_OVR_ENABLED;
            return(EFF_TWTA_TRIP_OVERRIDE_ENABLED);
            break;

        //-------------
        // hvps relays 
        //-------------
        case CMD_K11S:
            _ModRelay(11, RELAY_SET);
            return(_HvpsEffect(&oldCmdState));
            break;
        case CMD_K11R:
            _ModRelay(11, RELAY_RESET);
            return(_HvpsEffect(&oldCmdState));
            break;
        case CMD_K12S:
            _ModRelay(12, RELAY_SET);
            return(_HvpsEffect(&oldCmdState));
            break;
        case CMD_K12R:
            _ModRelay(12, RELAY_RESET);
            return(_HvpsEffect(&oldCmdState));
            break;

        //-------------------
        // binning constants 
        //-------------------
        case CMD_BIN:
            _SetBinningConstants(command->dataFilename);
            return(EFF_NEW_BINNING_CONSTANTS);
            break;

        //------------------
        // antenna sequence 
        //------------------
        case CMD_ANT:
            _SetAntennaSequence(command->dataFilename);
            return(EFF_NEW_ANTENNA_SEQUENCE);
            break;

        //-----
        // trs 
        //-----
        case CMD_TRS1:
            _ModTrs(TRS_TWTA1);
            trs = TRS_TWTA1;
            return(_TrsEffect(&oldCmdState));
            break;
        case CMD_TRS2:
            trs = TRS_TWTA1;
            return(_TrsEffect(&oldCmdState));
            break;

        //------------------
        // rfs trip monitor 
        //------------------
        case CMD_MON:
            return(EFF_RFS_MONITOR_CHANGE);
            break;
        default:
            break;
    }
#endif // LATER
    return(EFF_UNKNOWN);
}

//------------------
// ElectronicsState 
//------------------

CmdState::ElectronicsE
CmdState::ElectronicsState()
{
    return((ElectronicsE)Eval_3k(k[1], k[2], k[3],
        (int *)electronics_state_k1_k2_k3));
}

//------------------------
// ReplacementHeaterState 
//------------------------

CmdState::ReplacementHeaterE
CmdState::ReplacementHeaterState()
{
    return((ReplacementHeaterE)Eval_3k(k[4], k[5], k[6],
        (int *)replacement_heater_state_k4_k5_k6));
}

//----------
// CdsState 
//----------

CmdState::CdsE
CmdState::CdsState()
{
    return((CdsE)Eval_2k(k[7], k[8], (int *)cds_state_k7_k8));
}

//-----------
// ModeState 
//-----------

CmdState::ModeE
CmdState::ModeState()
{
    return(mode);
}

//-----------
// TwtaState 
//-----------

CmdState::TwtaE
CmdState::TwtaState()
{
    return((TwtaE)Eval_2k(k[9], k[10], (int *)twta_state_k9_k10));
}

//-----------
// HvpsState 
//-----------

CmdState::HvpsE
CmdState::HvpsState()
{
    return((HvpsE)Eval_2k(k[11], k[12], (int *)hvps_state_k11_k12));
}

//------------------
// SpareHeaterState 
//------------------

CmdState::SpareHeaterE
CmdState::SpareHeaterState()
{
    return((SpareHeaterE)Eval_2k(k[13], k[14],
        (int *)spare_heater_state_k13_k14));
}

//----------
// TrsState 
//----------

CmdState::TrsE
CmdState::TrsState()
{
    return(trs);
}

//-----------
// operators 
//-----------

CmdState&
CmdState::operator=(
    const CmdState&     other)
{
    for (int i = 0; i < RELAY_COUNT; i++)
        k[i] = other.k[i];

    mode = other.mode;
    twtaTripOvrEnable = other.twtaTripOvrEnable;
    twtMonEnable = other.twtMonEnable;
    hvpsShutdownEnable = other.hvpsShutdownEnable;
    tripDurationLimit = other.tripDurationLimit;
    trs = other.trs;
    _SetAntennaSequence(other.antennaSequence);
    _SetBinningConstants(other.binningConstants);
    return(*this);
}

//---------------------
// _SetAntennaSequence 
//---------------------
// sets the antennaSequence string

int
CmdState::_SetAntennaSequence(
    const char*     string)
{
    free(antennaSequence);  // in case it is non-NULL
    if (string == NULL)
    {
        antennaSequence = NULL;
        return(1);
    }
    else
    {
        antennaSequence = strdup(string);
        if (antennaSequence == NULL)
            return(0);
        else
            return(1);
    }
}

//----------------------
// _SetBinningConstants 
//----------------------
// sets the binningConstants string

int
CmdState::_SetBinningConstants(
    const char*     string)
{
    free(binningConstants); // in case it is non-NULL
    if (string == NULL)
    {
        binningConstants = NULL;
        return(1);
    }
    else
    {
        binningConstants = strdup(string);
        if (binningConstants == NULL)
            return(0);
        else
            return(1);
    }
}

//-----------
// _ModRelay 
//-----------
// sets or resets a relay (keeps track of the count)

void
CmdState::_ModRelay(
    int     relay_index,
    RelayE  relay_state)
{
    if (k[relay_index] != relay_state)
    {
        k[relay_index] = relay_state;
        k_counter[relay_index][relay_state]++;
    }
    return;
}

//---------
// _ModTrs 
//---------
// sets the TRS (keeps track of the count)

void
CmdState::_ModTrs(
    TrsE    trs_state)
{
    if (trs != trs_state)
    {
        trs = trs_state;
        trs_counter[trs_state]++;
    }
    return;
}

//--------------
// _PowerEffect 
//--------------
// returns the effect of changing one of the power relays (k1, k2, k3)
// also sets the mode appropriately

EffectE
CmdState::_PowerEffect(
    CmdState    *oldCmdState)
{
    ElectronicsE old_elec_state = oldCmdState->ElectronicsState();
    ElectronicsE new_elec_state = ElectronicsState();

    if (old_elec_state == new_elec_state)
    {
        if (k[1] == oldCmdState->k[1] && k[2] == oldCmdState->k[2] &&
            k[3] == oldCmdState->k[3])
        {
            return(EFF_NONE);       // no relays changed
        }
        else
        {
            return(EFF_MOOT);       // a relay changed, but no effect
        }
    }
    else    // the electronic state changed
    {
        switch(new_elec_state)
        {
            case ELECTRONICS_ON:
                _ModRelay(11, RELAY_SET);   // turn the hvps off
                _ModRelay(12, RELAY_RESET);
                mode = MODE_SBM;
                twtaTripOvrEnable = TWTA_TRIP_OVR_DISABLED;
                twtMonEnable = TWT_MON_DISABLED;
                hvpsShutdownEnable = HVPS_SHUTDOWN_ENABLED;
                tripDurationLimit = DEFAULT_TRIP_DURATION_LIMIT;
                _SetAntennaSequence(DEFAULT_ANTENNA_SEQUENCE);
                _SetBinningConstants(DEFAULT_BINNING_CONSTANTS);
                return(EFF_ELECTRONICS_ON);
                break;
            case ELECTRONICS_OFF:
            {
                mode = MODE_NA;
                twtaTripOvrEnable = TWTA_TRIP_OVR_NA;
                twtMonEnable = TWT_MON_NA;
                hvpsShutdownEnable = HVPS_SHUTDOWN_NA;
                tripDurationLimit = TRIP_DURATION_LIMIT_NA;
                _SetAntennaSequence(ANTENNA_SEQUENCE_NA);
                _SetBinningConstants(BINNING_CONSTANTS_NA);
                ReplacementHeaterE rh_state = ReplacementHeaterState();
                if (rh_state == REPLACEMENT_HEATER_ENABLED)
                    return(EFF_RHM);
                if (rh_state == REPLACEMENT_HEATER_DISABLED)
                    return(EFF_ALL_OFF);
                break;
            }
            default:
                break;
        }
    }
    return(EFF_UNKNOWN);
}

//---------------
// _HeaterEffect 
//---------------
// returns the effect of changing one of the replacement heater
// relays (k4, k5, k6)

EffectE
CmdState::_HeaterEffect(
    CmdState    *oldCmdState)
{
    // first look for useless changes
    ReplacementHeaterE old_rh_state = oldCmdState->ReplacementHeaterState();
    ReplacementHeaterE new_rh_state = ReplacementHeaterState();
    if (old_rh_state == new_rh_state)
    {
        if (k[4] == oldCmdState->k[4] && k[5] == oldCmdState->k[5] &&
            k[6] == oldCmdState->k[6])
        {
            return(EFF_NONE);       // no change at all
        }
        else
        {
            return(EFF_MOOT);       // a relay changed, but no effect
        }
    }
    else        // heater state has changed
    {
        // effect depends on power state
        ElectronicsE elec_state = ElectronicsState();
        switch(elec_state)
        {
            case ELECTRONICS_ON:
                switch(new_rh_state)
                {
                    case REPLACEMENT_HEATER_ENABLED:
                        return(EFF_REPLACEMENT_HEATER_ENABLED);
                        break;
                    case REPLACEMENT_HEATER_DISABLED:
                        return(EFF_REPLACEMENT_HEATER_DISABLED);
                        break;
                    default:
                        break;
                }
                break;
            case ELECTRONICS_OFF:
                switch(new_rh_state)
                {
                    case REPLACEMENT_HEATER_ENABLED:
                        return(EFF_RHM);
                        break;
                    case REPLACEMENT_HEATER_DISABLED:
                        return(EFF_ALL_OFF);
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
    return(EFF_UNKNOWN);
}

//------------
// _CdsEffect 
//------------
// returns the effect of changing one of the cds relays (k7, k8)

EffectE
CmdState::_CdsEffect(
    CmdState    *oldCmdState)
{
    CdsE old_cds_state = oldCmdState->CdsState();
    CdsE new_cds_state = CdsState();
    if (old_cds_state == new_cds_state)
    {
        return(EFF_NONE);       // must not have changed a relay
    }
    else
    {
        // set the effect of cds switching (if electronics are on)
        ElectronicsE elec_state = ElectronicsState();
        if (elec_state == ELECTRONICS_ON)
        {
            mode = MODE_SBM;
            twtaTripOvrEnable = TWTA_TRIP_OVR_DISABLED;
            twtMonEnable = TWT_MON_DISABLED;
            hvpsShutdownEnable = HVPS_SHUTDOWN_ENABLED;
            tripDurationLimit = DEFAULT_TRIP_DURATION_LIMIT;
            _SetAntennaSequence(DEFAULT_ANTENNA_SEQUENCE);
            _SetBinningConstants(DEFAULT_BINNING_CONSTANTS);
        }
        switch(new_cds_state)
        {
            case CDS_A:
                return(EFF_CDS_A);
                break;
            case CDS_B:
                return(EFF_CDS_A);
                break;
            default:
                break;
        }
    }
    return(EFF_UNKNOWN);
}

//-------------
// _ModeEffect 
//-------------
// returns the effect of setting the mode

EffectE
CmdState::_ModeEffect(
    CmdState    *oldCmdState)
{
    ModeE old_mode_state = oldCmdState->ModeState();
    ModeE new_mode_state = ModeState();
    if (old_mode_state == new_mode_state)
    {
        return(EFF_NONE);       // must not have changed mode
    }
    else
    {
        switch(new_mode_state)
        {
            case MODE_SBM:
                _ModRelay(11, RELAY_SET);   // turn the hvps off
                _ModRelay(12, RELAY_RESET);
                return(EFF_STB);
                break;
            case MODE_ROM:
                return(EFF_RCV);
                break;
            case MODE_CCM:
                return(EFF_CAL);
                break;
            case MODE_WOM:
                return(EFF_WOM);
                break;
            default:
                break;
        }
    }
    return(EFF_UNKNOWN);
}

//-------------
// _TwtaEffect 
//-------------
// returns the effect of changing one of the twta relays (k9, k10)
// assumes the electronics are on

EffectE
CmdState::_TwtaEffect(
    CmdState    *oldCmdState)
{
    TwtaE old_twta_state = oldCmdState->TwtaState();
    TwtaE new_twta_state = TwtaState();

    // twta commanding causes this...
    twtaTripOvrEnable = TWTA_TRIP_OVR_DISABLED;

    if (old_twta_state == new_twta_state)
        return(EFF_NONE);       // must not have changed a relay
    else
    {
        switch(new_twta_state)
        {
            case TWTA_1:
                return(EFF_TWTA_1);
                break;
            case TWTA_2:
                return(EFF_TWTA_2);
                break;
            default:
                break;
        }
    }
    return(EFF_UNKNOWN);
}

//-------------
// _HvpsEffect 
//-------------
// returns the effect of changing one of the hvps relays (k11, k12)
// assumes the electronics are on

EffectE
CmdState::_HvpsEffect(
    CmdState    *oldCmdState)
{
    HvpsE old_hvps_state = oldCmdState->HvpsState();
    HvpsE new_hvps_state = HvpsState();

    if (old_hvps_state == new_hvps_state)
        return(EFF_NONE);       // must not have changed a relay
    else
    {
        switch(new_hvps_state)
        {
            case HVPS_ON:
                hvps_counter[TwtaState()][new_hvps_state]++;
                return(EFF_HVPS_ON);
                break;
            case HVPS_OFF:
                hvps_counter[TwtaState()][new_hvps_state]++;
                return(EFF_HVPS_OFF);
                break;
            default:
                break;
        }
    }
    return(EFF_UNKNOWN);
}

//--------------------
// _SpareHeaterEffect 
//--------------------
// returns the effect of changing one of the spare heater relays (k13, k14)

EffectE
CmdState::_SpareHeaterEffect(
    CmdState    *oldCmdState)
{
    SpareHeaterE old_sh_state = oldCmdState->SpareHeaterState();
    SpareHeaterE new_sh_state = SpareHeaterState();
    if (old_sh_state == new_sh_state)
    {
        return(EFF_NONE);       // must not have changed a relay
    }
    else
    {
        switch(new_sh_state)
        {
            case SPARE_HEATER_ENABLED:
                return(EFF_SPARE_HEATER_ENABLED);
                break;
            case SPARE_HEATER_DISABLED:
                return(EFF_SPARE_HEATER_DISABLED);
                break;
            default:
                break;
        }
    }
    return(EFF_UNKNOWN);
}

//------------
// _TrsEffect 
//------------
// returns the effect of commanding the TRS position
// assumes the electronics are on

EffectE
CmdState::_TrsEffect(
    CmdState    *oldCmdState)
{
    /*
    TrsE old_trs_state = oldCmdState->TrsState();
    TrsE new_trs_state = TrsState();

    if (old_trs_state == new_trs_state)
        return(EFF_NONE);       // must not have changed state
    else
    {
        switch(new_trs_state)
        {
            case TRS_TWTA1:
                return(EFF_TRS_1);
                break;
            case TRS_TWTA2:
                return(EFF_TRS_2);
                break;
            default:
                break;
        }
    }
    */
    return(EFF_UNKNOWN);
}

//==================
// Helper Functions 
//==================

//---------
// Eval_3k 
//---------

int
Eval_3k(
    const int   relay1,
    const int   relay2,
    const int   relay3,
    const int   retval[3])  // minority, majority, unknown
{
    int set_count = 0;
    int reset_count = 0;

    if (relay1 == CmdState::RELAY_SET)
        set_count++;
    else if (relay1 == CmdState::RELAY_RESET)
        reset_count++;

    if (relay2 == CmdState::RELAY_SET)
        set_count++;
    else if (relay2 == CmdState::RELAY_RESET)
        reset_count++;

    if (relay3 == CmdState::RELAY_SET)
        set_count++;
    else if (relay3 == CmdState::RELAY_RESET)
        reset_count++;

    if (reset_count >= 2)
        return(retval[MOSTLY_RESET]);
    else if (set_count >= 2)
        return(retval[MOSTLY_SET]);
    else
        return(retval[UNKNOWN]);
}

//---------
// Eval_2k 
//---------

int
Eval_2k(
    const int   relay1,
    const int   relay2,
    const int   retval[3])  // match, differ, unknown
{
    if (relay1 == CmdState::RELAY_UNKNOWN || relay2 == CmdState::RELAY_UNKNOWN)
        return(retval[UNKNOWN]);

    if (relay1 == relay2)
        return(retval[MATCH]);
    else
        return(retval[DIFFER]);
}
