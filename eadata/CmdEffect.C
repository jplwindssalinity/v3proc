//=========================================================
// Copyright  (C)1998, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.2   29 Jan 1999 15:03:24   sally
// added LASP commands
// 
//    Rev 1.1   19 Aug 1998 14:37:28   daffer
// cmdlp work
// 
//    Rev 1.0   22 May 1998 16:53:50   daffer
// Initial Revision
// 
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

static const char rcsid_CmdEffect_C[] =
    "@(#) $Header$";

#include <stdlib.h>
#include <string.h>
#include "CmdEffect.h"
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


// Main electronics on/off (2 out of 3 voting scheme.)
// 2 set=> electronics off
// 2 reset => electronics on.
const CmdEffect::ElectronicsE electronics_effect_k1_k2_k3[] =
    { CmdEffect::ELECTRONICS_ON, CmdEffect::ELECTRONICS_OFF,
    CmdEffect::ELECTRONICS_UNKNOWN };

// Replacements heaters on/off (off means all the rest of the
// electronics *may* be on
const CmdEffect::ReplacementHeaterE replacement_heater_effect_k4_k5_k6[] =
    { CmdEffect::REPLACEMENT_HEATERS_ON,
    CmdEffect::REPLACEMENT_HEATERS_OFF,
    CmdEffect::REPLACEMENT_HEATERS_UNKNOWN };

//CDS A/B select
const CmdEffect::CdsE cds_effect_k7_k8[] =
    { CmdEffect::CDS_A, CmdEffect::CDS_B, CmdEffect::CDS_UNKNOWN };

//TWTA/TWTA Replacement heater select (requires k4/5/6 in majority
//reset state and SES electronics on.
const CmdEffect::TwtaElectronicsE twta_electronics_effect_k9_k10[] =
    { CmdEffect::TWTA_ELECTRONICS_ON, 
      CmdEffect::TWTA_REPLACEMENT_HEATER_ON, 
      CmdEffect::TWTA_ELECTRONICS_UNKNOWN };

// TWTA 1/2 select
const CmdEffect::TwtaE twta_effect_k11_k12[] =
    { CmdEffect::TWTA_1, 
      CmdEffect::TWTA_2, 
      CmdEffect::TWTA_UNKNOWN };

//SES electronics/Ses replacement heater.
const CmdEffect::SesElectronicsE ses_electronics_effect_k13_k14[]=
      { CmdEffect::SES_ELECTRONICS_ON,
        CmdEffect::SES_REPLACEMENT_HEATER_ON,
        CmdEffect::SES_ELECTRONICS_UNKNOWN};

//Ses a/b select
const CmdEffect::SesE ses_effect_k15_k16[]=
      { CmdEffect::SES_A,
        CmdEffect::SES_B,
        CmdEffect::SES_UNKNOWN};


//SAS electronics/Sas replacement heater.
const CmdEffect::SasElectronicsE sas_electronics_effect_k17_k18[]=
      { CmdEffect::SAS_ELECTRONICS_ON,
        CmdEffect::SAS_REPLACEMENT_HEATER_ON,
        CmdEffect::SAS_ELECTRONICS_UNKNOWN};


//Sas a/b select
const CmdEffect::SasE sas_effect_k19_k20[]=
      { CmdEffect::SAS_A,
        CmdEffect::SAS_B,
        CmdEffect::SAS_UNKNOWN};


// Ses supplemental heater k21/22
const CmdEffect::SesSupplHeaterE ses_suppl_heater_effect_k21_k22[] =
    { CmdEffect::SES_SUPPL_HEATER_ON, CmdEffect::SES_SUPPL_HEATER_OFF,
    CmdEffect::SES_SUPPL_HEATER_UNKNOWN };


const char* mode_effect_map[] = { "STB", "RCV", "CAL", "WOM", "?",
    "N/A" };
const char* cds_effect_map[] = { "CDS A", "CDS B", "?" };
const char* twta_electronics_effect_map[] = 
      { "TWTA On", "TWTA Repl Htrs On", "?" };
const char* twta_effect_map[] = { "TWTA #1", "TWTA #2","?" };
const char* sas_effect_map[] = { "SAS A", "SAS B","?" };
const char* Ses_effect_map[] = { "SES A", "SES B","?" };
const char* rh_effect_map[] = { "On", "Off", "?" };
const char* sh_effect_map[] = { "On", "Off", "?" };
const char* trs_effect_map[] = { "TWTA #1",  "TWTA #2", "?" };
const char* twta_trip_ovr_effect_map[] = { "Enabled", "Disabled", "?" };
const char* twt_mon_effect_map[] = { "Off", "On", "?" };
//const char* hvps_shutdown_effect_map[] = { "Off", "ON", "?" };

//==================
// CmdEffect methods 
//==================

CmdEffect::CmdEffect()
:   mode(INITIAL_MODE), twtaTripOvrEnable(INITIAL_TWTA_TRIP_OVR),
    twtMonEnable(INITIAL_TWT_MON)
    //    hvpsShutdownEnable(INITIAL_HVPS_SHUTDOWN),
    //   tripDurationLimit(INITIAL_TRIP_DURATION_LIMIT), trs(INITIAL_TRS),
    //   antennaSequence(NULL), binningConstants(NULL)
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
    k[15] = INITIAL_K15;
    k[16] = INITIAL_K16;
    k[17] = INITIAL_K17;
    k[18] = INITIAL_K18;
    k[19] = INITIAL_K19;
    k[20] = INITIAL_K20;
    k[21] = INITIAL_K21;
    k[22] = INITIAL_K22;

    ClearCounters();

    return;
}

CmdEffect::~CmdEffect()
{
    return;
}

//---------------
// ClearCounters 
//---------------

void
CmdEffect::ClearCounters()
{
    for (int r = 0; r < RELAY_COUNT; r++)
        for (int rs = 0; rs < RELAY_EFFECT_COUNT; rs++)
            k_counter[r][rs] = 0;

//     for (int w = 0; w < TRS_EFFECT_COUNT; w++)
//         trs_counter[w] = 0;

    for (int h = 0; h < TWTAELECT_COUNT; h++)
        for (int hs = 0; hs < TWTAELECT_EFFECT_COUNT; hs++)
            twtaelect_counter[h][hs] = 0;

    return;
}

//----------
// ApplyCmd 
//----------
// given a command, ApplyCmd returns the effect of the command
// and updates the CmdEffect based on the command.

EffectE
CmdEffect::ApplyCmd(
                    Command     *command)

{
    //------------------------------------
    // make sure the command is effective 
    //------------------------------------

    if (! command->Effective())
        return(EFF_NONE);

    //----------------------------------------
    // make a temporary copy of the old effect 
    //----------------------------------------

    CmdEffect oldCmdEffect = *this;

    //-******************************
    // APPLY PULSE DISCRETE COMMANDS 
    //-******************************

    switch (command->commandId)
    {
        //---------------------------
        // power relays (k1, k2, k3) 
        //---------------------------
        case EA_CMD_SCK1SET:
            _ModRelay(1, RELAY_SET);
            return(_PowerEffect(&oldCmdEffect));
            break;
        case EA_CMD_SCK1RST:
            _ModRelay(1, RELAY_RESET);
            return(_PowerEffect(&oldCmdEffect));
            break;
        case EA_CMD_SCK2SET:
            _ModRelay(2, RELAY_SET);
            return(_PowerEffect(&oldCmdEffect));
            break;
        case EA_CMD_SCK2RST:
            _ModRelay(2, RELAY_RESET);
            return(_PowerEffect(&oldCmdEffect));
            break;
        case EA_CMD_SCK3SET:
            _ModRelay(3, RELAY_SET);
            return(_PowerEffect(&oldCmdEffect));
            break;
        case EA_CMD_SCK3RST:
            _ModRelay(3, RELAY_RESET);
            return(_PowerEffect(&oldCmdEffect));
            break;

        //---------------------------
        // replacement heater relays 
        //---------------------------

        case EA_CMD_SCK4SET:
            _ModRelay(4, RELAY_SET);
            return(_HeaterEffect(&oldCmdEffect));
            break;
        case EA_CMD_SCK4RST:
            _ModRelay(4, RELAY_RESET);
            return(_HeaterEffect(&oldCmdEffect));
            break;
        case EA_CMD_SCK5SET:
            _ModRelay(5, RELAY_SET);
            return(_HeaterEffect(&oldCmdEffect));
            break;
        case EA_CMD_SCK5RST:
            _ModRelay(5, RELAY_RESET);
            return(_HeaterEffect(&oldCmdEffect));
            break;
        case EA_CMD_SCK6SET:
            _ModRelay(6, RELAY_SET);
            return(_HeaterEffect(&oldCmdEffect));
            break;
        case EA_CMD_SCK6RST:
            _ModRelay(6, RELAY_RESET);
            return(_HeaterEffect(&oldCmdEffect));
            break;


        //------------
        // cds relays 
        //------------

        case EA_CMD_SCK7SET:
            _ModRelay(7, RELAY_SET);
            return(_CdsEffect(&oldCmdEffect));
            break;
        case EA_CMD_SCK7RST:
            _ModRelay(7, RELAY_RESET);
            return(_CdsEffect(&oldCmdEffect));
            break;
        case EA_CMD_SCK8SET:
            _ModRelay(8, RELAY_SET);
            return(_CdsEffect(&oldCmdEffect));
            break;
        case EA_CMD_SCK8RST:
            _ModRelay(8, RELAY_RESET);
            return(_CdsEffect(&oldCmdEffect));
            break;


        //---------------------
        // ses supplemental heater relays 
        //---------------------

        case EA_CMD_SCK13SET:
            _ModRelay(13, RELAY_SET);
            return(_SupplHeaterEffect(&oldCmdEffect));
            break;
        case EA_CMD_SCK13RST:
            _ModRelay(13, RELAY_RESET);
            return(_SupplHeaterEffect(&oldCmdEffect));
            break;
        case EA_CMD_SCK14SET:
            _ModRelay(14, RELAY_SET);
            return(_SupplHeaterEffect(&oldCmdEffect));
            break;
        case EA_CMD_SCK14RST:
            _ModRelay(14, RELAY_RESET);
            return(_SupplHeaterEffect(&oldCmdEffect));
            break;
        default:
            break;
    } 

    //---------------------------------
    // check that the instrument is on 
    //---------------------------------

    ElectronicsE elec_effect = ElectronicsEffect();
    if (elec_effect == ELECTRONICS_OFF)
        return(EFF_NONE);
    if (elec_effect == ELECTRONICS_UNKNOWN)
        return(EFF_UNKNOWN);


    //-********************************
    // APPLY SERIAL MAGNITUDE COMMANDS 
    //-********************************
    
    // the electronics are on!
    switch (command->commandId)
    {


        //-------------
        // SES Electronics relays 
        //-------------
        
    case EA_CMD_SCK13SET:
        _ModRelay(13, RELAY_SET);
        return(_SesElectronicsEffect(&oldCmdEffect));
        break;
    case EA_CMD_SCK13RST:
        _ModRelay(13, RELAY_RESET);
        return(_SesElectronicsEffect(&oldCmdEffect));
        break;
    case EA_CMD_SCK14SET:
        _ModRelay(14, RELAY_SET);
        return(_SesElectronicsEffect(&oldCmdEffect));
        break;
    case EA_CMD_SCK14RST:
        _ModRelay(14, RELAY_RESET);
        return(_SesElectronicsEffect(&oldCmdEffect));
        break;
        

        //-------------
        // SAS Electronics relays 
        //-------------
        
    case EA_CMD_SCK17SET:
        _ModRelay(17, RELAY_SET);
        return(_SasElectronicsEffect(&oldCmdEffect));
        break;
    case EA_CMD_SCK17RST:
        _ModRelay(17, RELAY_RESET);
        return(_SasElectronicsEffect(&oldCmdEffect));
        break;
    case EA_CMD_SCK18SET:
        _ModRelay(18, RELAY_SET);
        return(_SasElectronicsEffect(&oldCmdEffect));
        break;
    case EA_CMD_SCK18RST:
        _ModRelay(18, RELAY_RESET);
        return(_SasElectronicsEffect(&oldCmdEffect));
        break;
        

        
        //------
        // mode 
        //------
    case EA_CMD_SCMODSTB:
        mode = MODE_STB;
        return(_ModeEffect(&oldCmdEffect));
        break;
    case EA_CMD_SCMODCAL:
        mode = MODE_CAL;
        return(_ModeEffect(&oldCmdEffect));
        break;
    case EA_CMD_SCMODRCV:
        mode = MODE_RCV;
        return(_ModeEffect(&oldCmdEffect));
        break;
    case EA_CMD_SCMODWOM:
        mode = MODE_WOM;
        return(_ModeEffect(&oldCmdEffect));
        break;

        //----------
        // twta override
        //----------
    case EA_CMD_SCTOVRON:
        twtaTripOvrEnable = TWTA_TRIP_OVR_ENABLED;
        return (EFF_TWT_TRIP_OVERRIDE_ENABLE);
        break;
    case EA_CMD_SCTOVROFF:
        return (EFF_TWT_TRIP_OVERRIDE_DISABLE);
        break;

    case EA_CMD_SCTWTMDS:
    case EA_CMD_SCTWTMEN:
        return (EFF_TWTA_MONITOR_CHANGE);
        break;
        
        
        //-------------
        // twta Electronics relays 
        //-------------
        
    case EA_CMD_SCK9SET:
        _ModRelay(9, RELAY_SET);
        return(_TwtaElectronicsEffect(&oldCmdEffect));
        break;
    case EA_CMD_SCK9RST:
        _ModRelay(9, RELAY_RESET);
        return(_TwtaElectronicsEffect(&oldCmdEffect));
        break;
    case EA_CMD_SCK10SET:
        _ModRelay(10, RELAY_SET);
        return(_TwtaElectronicsEffect(&oldCmdEffect));
        break;
    case EA_CMD_SCK10RST:
        _ModRelay(10, RELAY_RESET);
        return(_TwtaElectronicsEffect(&oldCmdEffect));
        break;
        

        
        //-------------
        //  Twta selector relays 
        //-------------
    case EA_CMD_SCK11SET:
        _ModRelay(11, RELAY_SET);
        return(_TwtaEffect(&oldCmdEffect));
        break;
    case EA_CMD_SCK11RST:
        _ModRelay(11, RELAY_RESET);
        return(_TwtaEffect(&oldCmdEffect));
        break;
    case EA_CMD_SCK12SET:
        _ModRelay(12, RELAY_SET);
        return(_TwtaEffect(&oldCmdEffect));
        break;
    case EA_CMD_SCK12RST:
        _ModRelay(12, RELAY_RESET);
        return(_TwtaEffect(&oldCmdEffect));
        break;

        //-------------
        //  Sas selector relays 
        //-------------
    case EA_CMD_SCK19SET:
        _ModRelay(19, RELAY_SET);
        return(_SasEffect(&oldCmdEffect));
        break;
    case EA_CMD_SCK19RST:
        _ModRelay(19, RELAY_RESET);
        return(_SasEffect(&oldCmdEffect));
        break;
    case EA_CMD_SCK20SET:
        _ModRelay(20, RELAY_SET);
        return(_SasEffect(&oldCmdEffect));
        break;
    case EA_CMD_SCK20RST:
        _ModRelay(20, RELAY_RESET);
        return(_SasEffect(&oldCmdEffect));
        break;


        //-------------
        //  Ses selector relays 
        //-------------
    case EA_CMD_SCK15SET:
        _ModRelay(15, RELAY_SET);
        return(_SesEffect(&oldCmdEffect));
        break;
    case EA_CMD_SCK15RST:
        _ModRelay(15, RELAY_RESET);
        return(_SesEffect(&oldCmdEffect));
        break;
    case EA_CMD_SCK16SET:
        _ModRelay(16, RELAY_SET);
        return(_SesEffect(&oldCmdEffect));
        break;
    case EA_CMD_SCK16RST:
        _ModRelay(16, RELAY_RESET);
        return(_SesEffect(&oldCmdEffect));
        break;
        
    
        
        // -----------------------------------
        // Union of Parameter Table updates.
        // -----------------------------------
        
        
    case EA_CMD_SCSSTTBL:
    case EA_CMD_SCSCLTBL:
    case EA_CMD_SCSRCTBL:
    case EA_CMD_SCSWOTBL:
    case EA_CMD_SCSALLTBL:
        return(EFF_SES_PARAMS_TABLE_UPDATE);
        break;
        
    case EA_CMD_SCPSTTBL:
    case EA_CMD_SCPCLTBL:
    case EA_CMD_SCPRCTBL:
    case EA_CMD_SCPWOTBL:
        return(EFF_PRF_TABLE_UPDATE);
        break;
        
    case EA_CMD_SCBDATBL:
    case EA_CMD_SCBDBTBL:
        return (EFF_DOPPLER_TABLE_UPDATE);
        break;
        
    case EA_CMD_SCBRATBL:
    case EA_CMD_SCBRBTBL:
        return (EFF_RANGEGATE_TABLE_UPDATE);
        break;
        
        // Serial Digital Engineering Telmetry Table Update
    case EA_CMD_SCENGTBL:
        return (EFF_SER_DIG_ENG_TLM_TABLE_UPDATE);
        break;
        // Serial Digital Status Telmetry Table Update
    case EA_CMD_SCSTATBL:
        return (EFF_SER_DIG_ST_TLM_TABLE_UPDATE);
        break;

        
        // -----------------------------------
        // Union of Parameter Table CHANGEs.
        // -----------------------------------
        
        
    case EA_CMD_SCSSTSW:
    case EA_CMD_SCSCLSW:
    case EA_CMD_SCSRCSW:
    case EA_CMD_SCSWOSW:
    case EA_CMD_SCSALLSW:
        return(EFF_SES_PARAMS_TABLE_CHANGE);
        break;
        
    case EA_CMD_SCPSTSW:
    case EA_CMD_SCPCLSW:
    case EA_CMD_SCPRCSW:
    case EA_CMD_SCPWOSW:
        return(EFF_PRF_TABLE_CHANGE);
        break;

    case EA_CMD_SCBRASW:
    case EA_CMD_SCBRBSW:
        return (EFF_RANGEGATE_TABLE_CHANGE);
        break;

    case EA_CMD_SCBDASW:
    case EA_CMD_SCBDBSW:
        return (EFF_DOPPLER_TABLE_CHANGE);
        break;
        
    case EA_CMD_SCENGSW:
        return(EFF_SER_DIG_ENG_TLM_TABLE_CHANGE);
        break;

    case EA_CMD_SCSTASW:
        return(EFF_SER_DIG_ST_TLM_TABLE_CHANGE);
        break;

        //-----
        // trs 
        //-----
    case EA_CMD_SCTWTSEL:
        return(EFF_TRS_CHANGE);
        break;


        //-----
        // TWTA Low Drive Power Fault Protection
        //-----
    case EA_CMD_SCTLDFPEN:
        return (EFF_TWTA_LOWDRIVE_POWER_FP_ON);
        break;
        
    case EA_CMD_SCTLDFPDS:
        return(EFF_TWTA_LOWDRIVE_POWER_FP_OFF);
        break;
        
        
        //-----
        // Range Gate Width
        //-----
    case EA_CMD_SCRGAWID:
        return(EFF_RANGE_GATE_A_WIDTH_CHANGE);
        break;
    case EA_CMD_SCRGBWID:
        return(EFF_RANGE_GATE_B_WIDTH_CHANGE);
        break;
        
        //-----
        // Transmit Pulse Width Change
        //-----
    case EA_CMD_SCTRPWID:
        return(EFF_TRANSMIT_PULSE_WIDTH_CHANGE);
        break;
        
        
        //-----
        // Prf clock rate change
        //-----
    case EA_CMD_SCPRFCLK:
        return(EFF_PRF_CLOCK_CHANGE);
        break;
        
        //-----
        // Reciever Gain
        //-----
    case EA_CMD_SCRCVGAN:
        return(EFF_RECEIVER_GAIN_CHANGE);
        break;
        
        //-----
        // Grid Normal
        //-----
    case EA_CMD_SCGRDNOR:
        return(EFF_GRID_NORMAL);
        break;
        
        
        //-----
        // Grid Disable
        //-----
    case EA_CMD_SCGRDDS:
        return(EFF_GRID_DISABLED);
        break;
        
        //-----
        // Receive Protect On
        //-----
    case EA_CMD_SCRCVPON:
        return(EFF_RECEIVE_PROTECT_ON);
        break;
        
        //-----
        // Receive Protect Normal
        //-----
    case EA_CMD_SCRCVPNR:
        return(EFF_RECEIVE_PROTECT_NORMAL);
        break;
        
        //-----
        // Modulation on
        //-----
    case EA_CMD_SCTMDONN:
        return(EFF_MODULATION_ON);
        break;
        
        //-----
        // Modulation off
        //-----
    case EA_CMD_SCTMDOFF:
        return(EFF_MODULATION_OFF);
        break;
        
        
        //-----
        // SES Reset
        //-----
    case EA_CMD_SCSESRST:
        return(EFF_SES_RESET);
        break;
        
        //-----
        // SES Multi SEQ DLE Response Enable/Disable
        //-----
    case EA_CMD_SCSMLEN:
    case EA_CMD_SCSMLDS:
        return(EFF_SES_MULTISEQ_DLE_RESPONSE_CHANGE);
        break;
        
        
        //-----
        // SAS Multi SEQ DLE Response Enable/Disable
        //-----
    case EA_CMD_SCMSLEN:
    case EA_CMD_SCMSLDS:
        return(EFF_SAS_MULTISEQ_DLE_RESPONSE_CHANGE);
        break;
        
        
        //-----
        // SES Suppl Heater Cntrl Enabled
        //-----
    case EA_CMD_SCHTREN:
    case EA_CMD_SCHTRDS:
        return(EFF_SES_SUPPL_HEATER_CNTRL_CHANGE);
        break;
        
        //-----
        // Cal Pulse Period 
        //-----
    case EA_CMD_SCCALPER:
        return(EFF_CAL_PULSE_PERIOD_CHANGE);
        break;
        
        
        //-----
        // SES Reset/Relod Period
        //-----
    case EA_CMD_SCRSRLPR:
        return(EFF_SES_RESET_RELOAD_PERIOD);
        break;

        //-----
        // CDS Software Patch
        //-----
    case EA_CMD_SCSWPATCH:
        return(EFF_CDS_SW_PATCH);
        break;

        
        //-----
        // Software Patch Enable
        //-----
    case EA_CMD_SCSWPEN:
        return(EFF_CDS_SW_PATCH_ENABLE);
        break;
        
        //-----
        // Software Patch Disable
        //-----
    case EA_CMD_SCSWPDS:
        return(EFF_CDS_SW_PATCH_DISABLE);
        break;
        
        //-----
        // Tranmit pulse/ range gate width
        //-----
    case EA_CMD_SCGATEWID:
        return(EFF_TRANSMITPULSE_RANGEGATE_WIDTH_CHANGE);
        break;
        
        //-----
        // timer ticks
        //-----
    case EA_CMD_SCORBTIC:
        return(EFF_ORBIT_TIME_TICKS);
        break;
        
        //-----
        // Instrument Time Sync Interval
        //-----
    case EA_CMD_SCITMSYN:
        return(EFF_INST_TIME_SYNC_INTERVAL);
        break;
        
        
        //-----
        // CDS Memory Readout Start Address
        //-----
    case EA_CMD_SCMROSTR:
        return(EFF_CDS_MEMORY_RDOUT_START_ADDR);
        break;


        //-----
        // CDS Memory Readout Upper Limit
        //-----
    case EA_CMD_SCMRUPLM:
        return(EFF_CDS_MEMORY_RDOUT_UP_LIM);
        break;

        //-----
        // CDS Memory Readout Lower Limit
        //-----
    case EA_CMD_SCMRLOLM:
        return(EFF_CDS_MEMORY_RDOUT_LO_LIM);
        break;


        //-----
        // Table Readout 
        //-----
    case EA_CMD_SCTBLSTR:
        return(EFF_TABLE_READOUT_START);
        break;

        //-----
        // A/B Beam Offsets
        //-----
    case EA_CMD_SCBMOFST:
        return(EFF_BEAM_OFFSETS);
        break;

        //-----
        // SAS A/B Offsets
        //-----
    case EA_CMD_SCSASOFS:
        return(EFF_SAS_OFFSETS);
        break;


        //-----
        // SAS A spin rate=19.8
        //-----
    case EA_CMD_SCSSARST:
        return(EFF_SAS_A_SPIN_RATE_198);
        break;

        //-----
        // SAS A spin rate=18.0
        //-----
    case EA_CMD_SCSSASET:
        return(EFF_SAS_A_SPIN_RATE_180);
        break;


        //-----
        // SAS B spin rate=19.8
        //-----
    case EA_CMD_SCSSBRST:
        return(EFF_SAS_B_SPIN_RATE_198);
        break;

        //-----
        // SAS B spin rate=18.0
        //-----
    case EA_CMD_SCSSBSET:
        return(EFF_SAS_B_SPIN_RATE_180);
        break;


        //-----
        // CDS reset
        //-----
    case EA_CMD_SCCDSRST:
        return(EFF_CDS_SOFT_RESET);
        break;

        //-----
        // No op
        //-----
    case EA_CMD_SCNOOP:
        return(EFF_NONE);
        break;


    default:
        return(EFF_UNKNOWN);            
        break;
    }
    
    return(EFF_UNKNOWN);
}

//------------------
// ElectronicsEffect 
//------------------

CmdEffect::ElectronicsE
CmdEffect::ElectronicsEffect()
{
    return((ElectronicsE)Eval_3k(k[1], k[2], k[3],
        (int *)electronics_effect_k1_k2_k3));
}

//------------------------
// ReplacementHeaterEffect 
//------------------------

CmdEffect::ReplacementHeaterE
CmdEffect::ReplacementHeaterEffect()
{
    return((ReplacementHeaterE)Eval_3k(k[4], k[5], k[6],
        (int *)replacement_heater_effect_k4_k5_k6));
}

//----------
// CdsEffect 
//----------

CmdEffect::CdsE
CmdEffect::CdsEffect()
{
    return((CdsE)Eval_2k(k[7], k[8], (int *)cds_effect_k7_k8));
}

//-----------
// ModeEffect 
//-----------

CmdEffect::ModeE
CmdEffect::ModeEffect()
{
    return(mode);
}

//-----------
// TwtaEffect 
//-----------

CmdEffect::TwtaE
CmdEffect::TwtaEffect()
{
    return((TwtaE)Eval_2k(k[11], k[12], 
                (int *)twta_effect_k11_k12));
}

//-----------
// TwtaElectonicsEffect 
//-----------

CmdEffect::TwtaElectronicsE
CmdEffect::TwtaElectronicsEffect()
{
    return((TwtaElectronicsE)Eval_2k(k[9], k[10], 
                (int *)twta_electronics_effect_k9_k10));
}

//------------------
// SupplHeaterEffect 
//------------------

CmdEffect::SesSupplHeaterE
CmdEffect::SesSupplHeaterEffect()
{
    return((SesSupplHeaterE)Eval_2k(k[21], k[22],
        (int *)ses_suppl_heater_effect_k21_k22));
}

//------------------
// SesElectronicsEffect
//------------------

CmdEffect::SesElectronicsE 
CmdEffect::SesElectronicsEffect() {
    return((SesElectronicsE)Eval_2k(k[13], k[14],
        (int *)ses_electronics_effect_k13_k14));
}//CmdEffect::SesElectronicsEffect


//------------------
// SesE
//------------------

CmdEffect::SesE 
CmdEffect::SesEffect() {
    return((SesE)Eval_2k(k[15], k[16],
        (int *)ses_effect_k15_k16));
}//CmdEffect::SesEffect



//------------------
// SasElectronicsEffect
//------------------

CmdEffect::SasElectronicsE 
CmdEffect::SasElectronicsEffect() {
    return((SasElectronicsE)Eval_2k(k[17], k[18],
        (int *)sas_electronics_effect_k17_k18));
}//CmdEffect::SasElectronicsEffect


//------------------
// SasE
//------------------

CmdEffect::SasE 
CmdEffect::SasEffect() {
    return((SasE)Eval_2k(k[19], k[20],
        (int *)sas_effect_k19_k20));
}//CmdEffect::SasEffect


//----------
// TrsEffect 
//----------

// CmdEffect::TrsE
// CmdEffect::TrsEffect()
// {
//     return(trs);
// }

//-----------
// operators 
//-----------

CmdEffect&
CmdEffect::operator=(
    const CmdEffect&     other)
{
    for (int i = 0; i < RELAY_COUNT; i++)
        k[i] = other.k[i];

    mode = other.mode;
    twtaTripOvrEnable = other.twtaTripOvrEnable;
    twtMonEnable = other.twtMonEnable;
    //    hvpsShutdownEnable = other.hvpsShutdownEnable;
    //    tripDurationLimit = other.tripDurationLimit;
    //    trs = other.trs;
    //    _SetAntennaSequence(other.antennaSequence);
    //    _SetBinningConstants(other.binningConstants);
    return(*this);
}


/* _SetAntenaSequence and _SetBinningConstants are
   commented out for the nonce.
//---------------------
// _SetAntennaSequence 
//---------------------
// sets the antennaSequence string

int
CmdEffect::_SetAntennaSequence(
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
CmdEffect::_SetBinningConstants(
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

*/

//---------
// _ModTrs 
//---------
// sets the TRS (keeps track of the count)

// void
// CmdEffect::_ModTrs(
//     TrsE    trs_effect)
// {
//     if (trs != trs_effect)
//     {
//         trs = trs_effect;
//         trs_counter[trs_effect]++;
//     }
//     return;
// }

//--------------
// _PowerEffect 
//--------------
// returns the effect of changing one of the power relays (k1, k2, k3)
// also sets the mode appropriately

EffectE
CmdEffect::_PowerEffect(
    CmdEffect    *oldCmdEffect)
{
    ElectronicsE old_elec_effect = oldCmdEffect->ElectronicsEffect();
    ElectronicsE new_elec_effect = ElectronicsEffect();

    if (old_elec_effect == new_elec_effect)
    {
        if (k[1] == oldCmdEffect->k[1] && k[2] == oldCmdEffect->k[2] &&
            k[3] == oldCmdEffect->k[3])
        {
            return(EFF_NONE);       // no relays changed
        }
        else
        {
            return(EFF_MOOT);       // a relay changed, but no effect
        }
    }
    else    // the electronic effect changed
    {
        switch(new_elec_effect)
        {
            case ELECTRONICS_ON:
                _ModRelay(11, RELAY_SET);   // turn the hvps off
                _ModRelay(12, RELAY_RESET);
                mode = MODE_STB;
                twtaTripOvrEnable = TWTA_TRIP_OVR_DISABLED;
                twtMonEnable = TWT_MON_DISABLED;
                //hvpsShutdownEnable = HVPS_SHUTDOWN_ENABLED;
                //tripDurationLimit = DEFAULT_TRIP_DURATION_LIMIT;
                //_SetAntennaSequence(DEFAULT_ANTENNA_SEQUENCE);
                //_SetBinningConstants(DEFAULT_BINNING_CONSTANTS);
                return(EFF_ELECTRONICS_ON);
                break;
            case ELECTRONICS_OFF:
            {
                mode = MODE_NA;
                twtaTripOvrEnable = TWTA_TRIP_OVR_NA;
                twtMonEnable = TWT_MON_NA;
                //hvpsShutdownEnable = HVPS_SHUTDOWN_NA;
                //tripDurationLimit = TRIP_DURATION_LIMIT_NA;
                //_SetAntennaSequence(ANTENNA_SEQUENCE_NA);
                //_SetBinningConstants(BINNING_CONSTANTS_NA);
                ReplacementHeaterE rh_effect = ReplacementHeaterEffect();
                if (rh_effect == REPLACEMENT_HEATERS_ON)
                    return(EFF_RHM);
                if (rh_effect == REPLACEMENT_HEATERS_OFF)
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
CmdEffect::_HeaterEffect(
    CmdEffect    *oldCmdEffect)
{

    // first look for useless changes
    ReplacementHeaterE old_rh_effect = oldCmdEffect->ReplacementHeaterEffect();
    ReplacementHeaterE new_rh_effect = ReplacementHeaterEffect();
    if (old_rh_effect == new_rh_effect)
    {
        if (k[4] == oldCmdEffect->k[4] && k[5] == oldCmdEffect->k[5] &&
            k[6] == oldCmdEffect->k[6])
        {
            return(EFF_NONE);       // no change at all
        }
        else
        {
            return(EFF_MOOT);       // a relay changed, but no effect
        }
    }
    else        // heater effect has changed
    {
        // effect depends on power effect
        ElectronicsE elec_effect = ElectronicsEffect();
        switch(elec_effect)
        {
            case ELECTRONICS_ON:
                switch(new_rh_effect)
                {
                    case REPLACEMENT_HEATERS_ON:
                        return(EFF_REPLACEMENT_HEATERS_ON);
                        break;
                    case REPLACEMENT_HEATERS_OFF:
                        return(EFF_REPLACEMENT_HEATERS_OFF);
                        break;
                    default:
                        break;
                }
                break;
            case ELECTRONICS_OFF:
                switch(new_rh_effect)
                {
                    case REPLACEMENT_HEATERS_ON:
                        return(EFF_RHM);
                        break;
                    case REPLACEMENT_HEATERS_OFF:
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
// returns the effect of changing one of the cds selector relays (k7,
// k8)

EffectE
CmdEffect::_CdsEffect(
    CmdEffect    *oldCmdEffect)
{
    CdsE old_cds_effect = oldCmdEffect->CdsEffect();
    CdsE new_cds_effect = CdsEffect();
    if (old_cds_effect == new_cds_effect)
    {
        return(EFF_NONE);       // must not have changed a relay
    }
    else
    {
        // set the effect of cds switching (if electronics are on)
        ElectronicsE elec_effect = ElectronicsEffect();
        if (elec_effect == ELECTRONICS_ON)
        {
            mode = MODE_STB;
            twtaTripOvrEnable = TWTA_TRIP_OVR_DISABLED;
            twtMonEnable = TWT_MON_DISABLED;
            //            hvpsShutdownEnable = HVPS_SHUTDOWN_ENABLED;
            // tripDurationLimit = DEFAULT_TRIP_DURATION_LIMIT;
            //_SetAntennaSequence(DEFAULT_ANTENNA_SEQUENCE);
            //_SetBinningConstants(DEFAULT_BINNING_CONSTANTS);
        }
        switch(new_cds_effect)
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
CmdEffect::_ModeEffect(
    CmdEffect    *oldCmdEffect)
{
    ModeE old_mode_effect = oldCmdEffect->ModeEffect();
    ModeE new_mode_effect = ModeEffect();
    if (old_mode_effect == new_mode_effect)
    {
        return(EFF_NONE);       // must not have changed mode
    }
    else
    {
        switch(new_mode_effect)
        {
            case MODE_STB:
                // Standby mode turns the TwTA off.
                // which turns the  twta replacements 
                // heaters on.
                _ModRelay(9, RELAY_SET);   
                _ModRelay(10, RELAY_RESET);
                return(EFF_STB);
                break;
            case MODE_RCV:
                return(EFF_RCV);
                break;
            case MODE_CAL:
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
// returns the effect of changing one of the twta selector relays (k11,k12)
// assumes the electronics are on

EffectE
CmdEffect::_TwtaEffect(
    CmdEffect    *oldCmdEffect)
{
    TwtaE old_twta_effect = oldCmdEffect->TwtaEffect();
    TwtaE new_twta_effect = TwtaEffect();

    // twta commanding causes this...
    //    twtaTripOvrEnable = TWTA_TRIP_OVR_DISABLED;

    if (old_twta_effect == new_twta_effect)
        return(EFF_NONE);       // must not have changed a relay
    else
    {
        switch(new_twta_effect)
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
// _TwtaElectronicsEffect 
//-------------
// returns the effect of changing one of the twta power relays (k9,k10)
// assumes the electronics are on

EffectE
CmdEffect::_TwtaElectronicsEffect(
    CmdEffect    *oldCmdEffect)
{
    TwtaElectronicsE old_twtaelect_effect = 
        oldCmdEffect->TwtaElectronicsEffect();
    TwtaElectronicsE new_twtaelect_effect = TwtaElectronicsEffect();

    if (old_twtaelect_effect == new_twtaelect_effect)
        return(EFF_NONE);       // must not have changed a relay
    else
    {
        switch(new_twtaelect_effect)
        {
            case TWTA_ELECTRONICS_ON:
                twtaelect_counter[TwtaEffect()][new_twtaelect_effect]++;
                return(EFF_TWTA_ON);
                break;
            case TWTA_REPLACEMENT_HEATER_ON:
                twtaelect_counter[TwtaEffect()][new_twtaelect_effect]++;
                return(EFF_TWTA_REPL_HEATER_OFF);
                break;
            default:
                break;
        }
    }
    return(EFF_UNKNOWN);
}

//--------------------
// _SupplHeaterEffect 
//--------------------
// returns the effect of changing one of the SES Supplemental heater
// relays (k21, k22)

EffectE
CmdEffect::_SupplHeaterEffect(
    CmdEffect    *oldCmdEffect)
{
    SesSupplHeaterE old_sh_effect = oldCmdEffect->SesSupplHeaterEffect();
    SesSupplHeaterE new_sh_effect = SesSupplHeaterEffect();
    if (old_sh_effect == new_sh_effect)
    {
        return(EFF_NONE);       // must not have changed a relay
    }
    else
    {
        switch(new_sh_effect)
        {
            case SES_SUPPL_HEATER_ON:
                return(EFF_SES_SUPPL_HEATER_ON);
                break;
            case SES_SUPPL_HEATER_OFF:
                return(EFF_SES_SUPPL_HEATER_OFF);
                break;
            default:
                break;
        }
    }
    return(EFF_UNKNOWN);
}



//--------------------
// _SesElectronicsEffect 
//--------------------
// returns the effect of changing one of the Ses Electronics relays (k13, k14)

EffectE
CmdEffect::_SesElectronicsEffect(
    CmdEffect    *oldCmdEffect)
{
    SesElectronicsE old_ses_elect_effect = 
        oldCmdEffect->SesElectronicsEffect();
    SesElectronicsE new_ses_elect_effect = SesElectronicsEffect();
    if (old_ses_elect_effect == new_ses_elect_effect)
    {
        return(EFF_NONE);       // must not have changed a relay
    }
    else
    {
        switch(new_ses_elect_effect)
        {
            case SES_ELECTRONICS_ON:
                return(EFF_SES_ELECTRONICS_ON);
                break;
            case SES_REPLACEMENT_HEATER_ON:
                return(EFF_SES_REPL_HEATER_ON);
                break;
            default:
                break;
        }
    }
    return(EFF_UNKNOWN);
}//_SesElectronicsEffect


//--------------------
// _SasElectronicsEffect 
//--------------------
// returns the effect of changing one of the Sas Electronics relays (k17,k18)

EffectE
CmdEffect::_SasElectronicsEffect(
    CmdEffect    *oldCmdEffect)
{
    SasElectronicsE old_sas_elect_effect = 
        oldCmdEffect->SasElectronicsEffect();
    SasElectronicsE new_sas_elect_effect = SasElectronicsEffect();
    if (old_sas_elect_effect == new_sas_elect_effect)
    {
        return(EFF_NONE);       // must not have changed a relay
    }
    else
    {
        switch(new_sas_elect_effect)
        {
            case SAS_ELECTRONICS_ON:
                return(EFF_SAS_ELECTRONICS_ON);
                break;
            case SAS_REPLACEMENT_HEATER_ON:
                return(EFF_SAS_REPL_HEATER_ON);
                break;
            default:
                break;
        }
    }
    return(EFF_UNKNOWN);
}//_SasElectronicsEffect



//-------------
// _SesEffect 
//-------------
// returns the effect of changing one of the ses selector relays (k15,k16)
// assumes the electronics are on

EffectE
CmdEffect::_SesEffect(
    CmdEffect    *oldCmdEffect)
{
    SesE old_ses_effect = oldCmdEffect->SesEffect();
    SesE new_ses_effect = SesEffect();


    if (old_ses_effect == new_ses_effect)
        return(EFF_NONE);       // must not have changed a relay
    else
    {
        switch(new_ses_effect)
        {
            case SES_A:
                return(EFF_SES_A);
                break;
            case SES_B:
                return(EFF_SES_B);
                break;
            default:
                break;
        }
    }
    return(EFF_UNKNOWN);
}


//-------------
// _SasEffect 
//-------------
// returns the effect of changing one of the sas selector relays (k19, k20)
// assumes the electronics are on

EffectE
CmdEffect::_SasEffect(
    CmdEffect    *oldCmdEffect)
{
    SasE old_sas_effect = oldCmdEffect->SasEffect();
    SasE new_sas_effect = SasEffect();


    if (old_sas_effect == new_sas_effect)
        return(EFF_NONE);       // must not have changed a relay
    else
    {
        switch(new_sas_effect)
        {
            case SAS_A:
                return(EFF_SAS_A);
                break;
            case SAS_B:
                return(EFF_SAS_B);
                break;
            default:
                break;
        }
    }
    return(EFF_UNKNOWN);
}

/* _TrsEffect is commented out for the moment.
//------------
// _TrsEffect 
//------------
// returns the effect of commanding the TRS position
// assumes the electronics are on

EffectE
CmdEffect::_TrsEffect(
    CmdEffect    *oldCmdEffect)
{
    TrsE old_trs_effect = oldCmdEffect->TrsEffect();
    TrsE new_trs_effect = TrsEffect();

    if (old_trs_effect == new_trs_effect)
        return(EFF_NONE);       // must not have changed effect
    else
    {
        switch(new_trs_effect)
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
    return(EFF_UNKNOWN);
}

*/

//==================
// Helper Functions 
//==================

//-----------
// _ModRelay 
//-----------
// sets or resets a relay (keeps track of the count)

void
CmdEffect::_ModRelay(
    int     relay_index,
    RelayE  relay_effect)
{
    if (k[relay_index] != relay_effect)
    {
        k[relay_index] = relay_effect;
        k_counter[relay_index][relay_effect]++;
    }
    return;
}

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

    if (relay1 == CmdEffect::RELAY_SET)
        set_count++;
    else if (relay1 == CmdEffect::RELAY_RESET)
        reset_count++;

    if (relay2 == CmdEffect::RELAY_SET)
        set_count++;
    else if (relay2 == CmdEffect::RELAY_RESET)
        reset_count++;

    if (relay3 == CmdEffect::RELAY_SET)
        set_count++;
    else if (relay3 == CmdEffect::RELAY_RESET)
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
    if (relay1 == CmdEffect::RELAY_UNKNOWN || 
        relay2 == CmdEffect::RELAY_UNKNOWN)
        return(retval[UNKNOWN]);

    if (relay1 == relay2)
        return(retval[MATCH]);
    else
        return(retval[DIFFER]);
}

