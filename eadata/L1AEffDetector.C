//=========================================================//
// Copyright  (C)1996, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id[] =
    "@(#) $Id$";

#include "L1AEffDetector.h"
#include "L1Extract.h.old"
#include "State.h"

//=======================//
// L1AEffDetector methods //
//=======================//

L1AEffDetector::L1AEffDetector()
{
    return;
}

L1AEffDetector::~L1AEffDetector()
{
    return;
}

//-----------//
// AddEffect //
//-----------//

int
L1AEffDetector::AddEffect(
    const Itime     effect_time,
    const EffectE   effect_id)
{
    Command *cmd = new Command();
    if (! cmd)
        return(0);
    cmd->l1Time = effect_time;
    cmd->effectId = effect_id;
    cmd->l1Verify = VER_YES;
    _list->Append(cmd);
    return(1);
}

//---------------//
// DetectEffects //
//---------------//

int
L1AEffDetector::DetectEffects(
    const char*     data_rec)
{
    //------//
    // Time //
    //------//

    Itime effect_time = L1A_Itime(data_rec);

    //----------------------------//
    // Command Counter Increments //
    //----------------------------//

/*
    static unsigned char cmd_counter;
    static unsigned char last_cmd_counter;
    static char valid_cmd_counter = 0;
    static int serial_magnitude_commands = 0;
    if (L1A_Cmd_Counter(data_rec, (char *)&cmd_counter))
    {
        if (valid_cmd_counter)
        {
            int new_cmds = (32 + cmd_counter - last_cmd_counter) % 32;
        }
        valid_cmd_counter = 1;
        last_cmd_counter = cmd_counter;
        serial_magnitude_commands += new_cmds;
    }
*/

    //---------//
    // All Off //
    //---------//

    // not detectable

    //----------------//
    // Electronics On //
    //----------------//

    static unsigned int inst_time;
    if (L1A_Inst_Time(data_rec, (char *)&inst_time))
    {
        if (inst_time == 0)
            AddEffect(effect_time, EFF_ELECTRONICS_ON);
    }

    //-------------------------//
    // Replacement Heater Mode //
    //-------------------------//

    // not detectable

    //-------------------------------------//
    // Replacement Heater Enabled/Disabled //
    //-------------------------------------//

    // not detectable

    //---------//
    // DSS A/B //
    //---------//

    static unsigned char dss;
    static unsigned char last_dss;
    static char valid_dss = 0;
    if (L1A_DSS(data_rec, (char *)&dss))
    {
        if (valid_dss && last_dss != dss)
        {
            switch(dss)
            {
                case TLM_DSS_A:
                    AddEffect(effect_time, EFF_DSS_A);
                    break;
                case TLM_DSS_B:
                    AddEffect(effect_time, EFF_DSS_B);
                    break;
                default:
                    AddEffect(effect_time, EFF_UNKNOWN);
                    break;
            }
        }
        valid_dss = 1;
        last_dss = dss;
    }

    //-------------------------------//
    // Spare Heater Enabled/Disabled //
    //-------------------------------//

    // not detectable

    //------//
    // Mode //
    //------//

    static unsigned char mode;
    static unsigned char last_mode;
    static char valid_mode = 0;
    if (L1A_NSCAT_Mode(data_rec, (char *)&mode))
    {
        if (valid_mode && last_mode != mode)
        {
            switch(mode)
            {
                case TLM_MODE_SBM:
                    AddEffect(effect_time, EFF_SBM);
                    break;
                case TLM_MODE_ROM:
                    AddEffect(effect_time, EFF_ROM);
                    break;
                case TLM_MODE_CCM:
                    AddEffect(effect_time, EFF_CCM);
                    break;
                case TLM_MODE_DBM:
                    AddEffect(effect_time, EFF_DBM);
                    break;
                case TLM_MODE_WOM:
                    AddEffect(effect_time, EFF_WOM);
                    break;
                default:
                    AddEffect(effect_time, EFF_UNKNOWN);
                    break;
            }
        }
        valid_mode = 1;
        last_mode = mode;
    }

    //----------//
    // TWTA 1/2 //
    //----------//

    static unsigned char twta;
    static unsigned char last_twta;
    static char valid_twta = 0;
    if (L1A_Valid_TWTA(data_rec, (char *)&twta))
    {
        if (valid_twta && last_twta != twta)
        {
            switch(twta)
            {
                case TLM_TWTA_1:
                    AddEffect(effect_time, EFF_TWTA_1);
                    break;
                case TLM_TWTA_2:
                    AddEffect(effect_time, EFF_TWTA_2);
                    break;
                default:
                    AddEffect(effect_time, EFF_UNKNOWN);
                    break;
            }
        }
        valid_twta = 1;
        last_twta = twta;
    }

    //-------------//
    // HVPS On/Off //
    //-------------//

    static unsigned char hvps;
    static unsigned char last_hvps;
    static char valid_hvps = 0;
    if (L1A_HVPS(data_rec, (char *)&hvps))
    {
        if (valid_hvps && last_hvps != hvps)
        {
            switch(hvps)
            {
                case TLM_HVPS_ON:
                    AddEffect(effect_time, EFF_HVPS_ON);
                    break;
                case TLM_HVPS_OFF:
                    AddEffect(effect_time, EFF_HVPS_OFF);
                    break;
                default:
                    AddEffect(effect_time, EFF_UNKNOWN);
                    break;
            }
        }
        valid_hvps = 1;
        last_hvps = hvps;
    }

    //-------------------//
    // Binning Constants //
    //-------------------//

    static unsigned char new_bc;
    static unsigned char last_new_bc;
    static char valid_new_bc = 0;
    if (L1A_New_BC(data_rec, (char *)&new_bc))
    {
        if (valid_new_bc && last_new_bc != new_bc)
        {
            AddEffect(effect_time, EFF_NEW_BINNING_CONSTANTS);
        }
        valid_new_bc = 1;
        last_new_bc = new_bc;
    }

    //------------------//
    // Antenna Sequence //
    //------------------//

    static unsigned char new_ant_seq;
    static unsigned char last_new_ant_seq;
    static char valid_new_ant_seq = 0;
    if (L1A_New_Ant_Seq(data_rec, (char *)&new_ant_seq))
    {
        if (valid_new_ant_seq && last_new_ant_seq != new_ant_seq)
        {
            AddEffect(effect_time, EFF_NEW_ANTENNA_SEQUENCE);
        }
        valid_new_ant_seq = 1;
        last_new_ant_seq = new_ant_seq;
    }

    //---------//
    // WTS 1/2 //
    //---------//

    // not detectable

    //----------------------------//
    // TWTA Body Overcurrent Trip //
    //----------------------------//

    static unsigned char twta_trip_ovr_en;
    static unsigned char last_twta_trip_ovr_en;
    static char valid_twta_trip_ovr_en = 0;
    if (L1A_TWTA_Trip_Override_Enable(data_rec, (char *)&twta_trip_ovr_en))
    {
        if (valid_twta_trip_ovr_en &&
            twta_trip_ovr_en == TLM_TWTA_TRIP_OVERRIDE_EN &&
            last_twta_trip_ovr_en == TLM_TWTA_TRIP_OVERRIDE_DIS)
        {
            AddEffect(effect_time, EFF_TWTA_TRIP_OVERRIDE_ENABLED);
        }
        valid_twta_trip_ovr_en = 1;
        last_twta_trip_ovr_en = twta_trip_ovr_en;
    }

    //--------------------//
    // RFS Monitor Change //
    //--------------------//

    static unsigned char rfs_monitor;
    static unsigned char last_rfs_monitor;
    static char valid_rfs_monitor = 0;

    static unsigned char hvps_shutdown;
    static unsigned char last_hvps_shutdown;
    static char valid_hvps_shutdown = 0;

    static unsigned short trip_duration;
    static unsigned short last_trip_duration;
    static char valid_trip_duration = 0;

    int found_cmd = 0;
    if (L1A_TWT_Mon_En(data_rec, (char *)&rfs_monitor))
    {
        if (valid_rfs_monitor && last_rfs_monitor != rfs_monitor)
        {
            AddEffect(effect_time, EFF_RFS_MONITOR_CHANGE);
            found_cmd = 1;
        }
        valid_rfs_monitor = 1;
        last_rfs_monitor = rfs_monitor;
    }

    if (L1A_HVPS_Shut_En(data_rec, (char *)&hvps_shutdown))
    {
        if (valid_hvps_shutdown && last_hvps_shutdown != hvps_shutdown &&
            found_cmd == 0)
        {
            AddEffect(effect_time, EFF_RFS_MONITOR_CHANGE);
            found_cmd = 1;
        }
        valid_hvps_shutdown = 1;
        last_hvps_shutdown = hvps_shutdown;
    }

    if (L1A_Trip_Duration(data_rec, (char *)&trip_duration))
    {
        if (valid_trip_duration && last_trip_duration != trip_duration &&
            found_cmd == 0)
        {
            AddEffect(effect_time, EFF_RFS_MONITOR_CHANGE);
            found_cmd = 1;
        }
        valid_trip_duration = 1;
        last_trip_duration = trip_duration;
    }

    return(1);
}

//========================//
// NrtEffDetector methods //
//========================//

NrtEffDetector::NrtEffDetector()
{
    return;
}

NrtEffDetector::~NrtEffDetector()
{
    return;
}

//-----------//
// AddEffect //
//-----------//

int
NrtEffDetector::AddEffect(
    const Itime     effect_time,
    const EffectE   effect_id)
{
    Command *cmd = new Command();
    if (! cmd)
        return(0);
    cmd->nrtTime = effect_time;
    cmd->effectId = effect_id;
    cmd->nrtVerify = VER_YES;
    _list->Append(cmd);
    return(1);
}
