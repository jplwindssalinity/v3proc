//=========================================================//
// Copyright  (C)1996, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id[] =
    "@(#) $Id$";

#include <stdio.h>

#include "Parameter.h"
#include "ErrorChecker.h"
#include "State.h"

//======================//
// ErrorChecker methods //
//======================//

ErrorChecker::ErrorChecker()
{
    current = new State;
    prev = new State;
    dataRecCount = 0;
    return;
};

ErrorChecker::~ErrorChecker()
{
    delete current;
    delete prev;
    return;
};

//-----------------//
// ReportBasicInfo //
//-----------------//

void
ReportBasicInfo(
    FILE*   ofp,
    State*  state)
{
    if (state->time.condition == StateElement::UNINITIALIZED)
        return;

//  fprintf(ofp, "  %s", state->time_string);

    fprintf(ofp, "    Mode = ");
    if (state->nscat_mode.condition == StateElement::UNINITIALIZED)
        fprintf(ofp, "?");
    else
        fprintf(ofp, "%d (%s)", state->nscat_mode.value,
            mode_map[state->nscat_mode.value]);

    fprintf(ofp, ", DSS = ");
    if (state->dss.condition == StateElement::UNINITIALIZED)
        fprintf(ofp, "?");
    else
        fprintf(ofp, "%d (%s)", state->dss.value,
            dss_map[state->dss.value]);

    fprintf(ofp, ", TWTA = ");
    if (state->twta.condition == StateElement::UNINITIALIZED)
        fprintf(ofp, "?");
    else
        fprintf(ofp, "%d (%s)", state->twta.value,
            twta_map[state->twta.value]);

    fprintf(ofp, ", Fault Counter = ");
    if (state->fault_counter.condition == StateElement::UNINITIALIZED)
        fprintf(ofp, "?");
    else
        fprintf(ofp, "%d", state->fault_counter.value);

    fprintf(ofp, "\n");

    return;
}

//-----------------------//
// Error_Flag_Transition //
//-----------------------//
// prints an error message
// returns 1 on error in frame (whether or not reported), 0 otherwise

int
Error_Flag_Transition(
    FILE*               ofp,
    char*               error_name,
    char*               parameter_name,
    char*               label_format,
    State*              current_state,
    StateElement_uc*    current_elem,
    State*              prev_state,
    StateElement_uc*    prev_elem,
    const char*         map[],
    unsigned char       ok_value)
{
    if (current_elem->condition == StateElement::UNINITIALIZED)
        return (0);     // current is invalid

    // current is valid
    if (current_elem->value == ok_value)
    {
        // current is ok
        if (prev_elem->condition != StateElement::UNINITIALIZED &&
            prev_elem->value != ok_value)
        {
            // previous is valid but bad - error cleared
            fprintf(ofp, CLEAR_FORMAT, error_name);
            fprintf(ofp, "  %s  %s = %d (%s)\n",
                prev_state->time_string, parameter_name,
                prev_elem->value, map[prev_elem->value]);
            ReportBasicInfo(ofp, prev_state);
            fprintf(ofp, "  %s  %s = %d (%s)\n",
                current_state->time_string, parameter_name,
                current_elem->value, map[current_elem->value]);
            ReportBasicInfo(ofp, current_state);
            fflush(ofp);
        }
        return(0);
    }
    else
    {
        // current is bad
        if (prev_elem->condition == StateElement::UNINITIALIZED)
        {
            // starts out as bad - let user know
            fprintf(ofp, INITIALIZE_FORMAT, error_name);
            fprintf(ofp, "  %s  %s = %d (%s)\n",
                current_state->time_string, parameter_name,
                current_elem->value, map[current_elem->value]);
            ReportBasicInfo(ofp, current_state);
            fflush(ofp);
        }
        else if (prev_elem->value == ok_value)
        {
            // previous is ok - transition to bad
            fprintf(ofp, label_format, error_name);
            fprintf(ofp, "  %s  %s = %d (%s)\n",
                prev_state->time_string, parameter_name,
                prev_elem->value, map[prev_elem->value]);
            ReportBasicInfo(ofp, prev_state);
            fprintf(ofp, "  %s  %s = %d (%s)\n",
                current_state->time_string, parameter_name,
                current_elem->value, map[current_elem->value]);
            ReportBasicInfo(ofp, current_state);
            fflush(ofp);
        }
        else if (current_elem->value != prev_elem->value)
        {
            // still bad, but a different bad
            fprintf(ofp, label_format, error_name);
            fprintf(ofp, "  %s  %s = %d (%s)\n",
                prev_state->time_string, parameter_name,
                prev_elem->value, map[prev_elem->value]);
            ReportBasicInfo(ofp, prev_state);
            fprintf(ofp, "  %s  %s = %d (%s)\n",
                current_state->time_string, parameter_name,
                current_elem->value, map[current_elem->value]);
            ReportBasicInfo(ofp, current_state);
            fflush(ofp);
        }
        return(1);  // frame is bad
    }
    return(0);      // should never get here, but let's be safe
}

//-----------------//
// Relay2_Mismatch //
//-----------------//

int
Relay2_Mismatch(
    FILE*               ofp,
    char*               error_name,
    char*               parameter_name,
    char*               relay1_name,
    char*               relay2_name,
    char*               label_format,
    State*              state,
    StateElement_uc*    parameter,
    StateElement_uc*    relay1,
    StateElement_uc*    relay2,
    const char*         map[],
    const char*         relay1_map[],
    const char*         relay2_map[],
    const unsigned char decode_map[2][2])
{
    // make sure that the parameter is current (it's from sd, anyhow)
    if (parameter->condition != StateElement::CURRENT)
        return(0);

    // make sure that the relays are initialized
    if (relay1->condition == StateElement::UNINITIALIZED ||
        relay2->condition == StateElement::UNINITIALIZED)
    {
        return (0);
    }

    // parameters are valid
    unsigned char expected_value = decode_map[relay1->value][relay2->value];

    if (parameter->value == expected_value)
    {
        return (0);
    }

    // error
    fprintf(ofp, label_format, error_name);
    fprintf(ofp, "  %s  %s = %d (%s)\n",
        state->time_string, parameter_name,
        parameter->value, map[parameter->value]);
    fprintf(ofp, "  %s = %d (%s), %s = %d (%s)\n",
        relay1_name, relay1->value, relay1_map[relay1->value],
        relay2_name, relay2->value, relay2_map[relay2->value]);
    ReportBasicInfo(ofp, state);
    return (1);
}


//=============================//
// L1 and HK Error Functions //
//=============================//

//---------------------//
// Err_HVPS_Backup_Off //
//---------------------//

int
Err_HVPS_Backup_Off(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    return Error_Flag_Transition(ofp, error_name,
        "HVPS in Backup Off Configuration", label_format,
        obj->current,
        &(obj->current->hvps_backup_off),
        obj->prev,
        &(obj->prev->hvps_backup_off),
        hvps_backup_off_map, 0);
}

//------------------//
// Err_TWTA_UV_Trip //
//------------------//

int
Err_TWTA_UV_Trip(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    return Error_Flag_Transition(ofp, error_name,
        "TWTA Undervoltage Trip", label_format,
        obj->current, &(obj->current->twta_uv_trip),
        obj->prev, &(obj->prev->twta_uv_trip),
        twta_trip_map, 0);
}

//------------------//
// Err_TWTA_OC_Trip //
//------------------//

int
Err_TWTA_OC_Trip(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    return Error_Flag_Transition(ofp, error_name,
        "TWTA Overcurrent Trip", label_format,
        obj->current, &(obj->current->twta_oc_trip),
        obj->prev, &(obj->prev->twta_oc_trip),
        twta_trip_map, 0);
}

//-----------------------//
// Err_TWTA_Body_OC_Trip //
//-----------------------//

int
Err_TWTA_Body_OC_Trip(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    return Error_Flag_Transition(ofp, error_name,
        "TWTA Body Overcurrent Trip", label_format,
        obj->current, &(obj->current->twta_body_oc_trip),
        obj->prev, &(obj->prev->twta_body_oc_trip),
        twta_trip_map, 0);
}

//---------------//
// Err_Bin_Param //
//---------------//

int
Err_Bin_Param(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    return Error_Flag_Transition(ofp, error_name,
        "Binning Parameter Error", label_format,
        obj->current, &(obj->current->bin_param_err),
        obj->prev, &(obj->prev->bin_param_err),
        bin_param_err_map, 0);
}

//-------------------//
// Err_Def_Bin_Const //
//-------------------//

int
Err_Def_Bin_Const(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    return Error_Flag_Transition(ofp, error_name,
        "Default Binning Constants", label_format,
        obj->current, &(obj->current->def_bin_const),
        obj->prev, &(obj->prev->def_bin_const),
        def_bin_const_map, 0);
}

//---------------------//
// Err_Lack_Start_Reqs //
//---------------------//

int
Err_Lack_Start_Reqs(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    return Error_Flag_Transition(ofp, error_name,
        "Lack Startup Requirements", label_format,
        obj->current, &(obj->current->lack_start_reqs),
        obj->prev, &(obj->prev->lack_start_reqs),
        lack_start_reqs_map, 0);
}

//----------------------//
// Err_Error_Queue_Full //
//----------------------//

int
Err_Error_Queue_Full(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    return Error_Flag_Transition(ofp, error_name,
        "Error Queue Full", label_format,
        obj->current, &(obj->current->err_queue_full),
        obj->prev, &(obj->prev->err_queue_full),
        err_queue_full_map, 0);
}

//-------------------//
// Err_Fault_Counter //
//-------------------//

int
Err_Fault_Counter(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    static char* parameter_name = "Fault Counter";
    char* current_time_string = obj->current->time_string;
    StateElement::ConditionE current_condition =
            obj->current->fault_counter.condition;
    unsigned char current_value = obj->current->fault_counter.value;
    char* prev_time_string = obj->prev->time_string;
    StateElement::ConditionE prev_condition =
            obj->prev->fault_counter.condition;
    unsigned char prev_value = obj->prev->fault_counter.value;

    if (current_condition != StateElement::UNINITIALIZED &&
        prev_condition != StateElement::UNINITIALIZED &&
        current_value != prev_value)
    {
        // current and previous are both valid yet different
        fprintf(ofp, label_format, error_name);
        fprintf(ofp, "  %s  %s = %d\n",
            prev_time_string, parameter_name, prev_value);
        fprintf(ofp, "  %s  %s = %d\n",
            current_time_string, parameter_name, current_value);
        ReportBasicInfo(ofp, obj->current);
        fflush(ofp);
        return(1);
    }
    return (0);
}

//----------//
// Err_Mode //
//----------//

int
Err_Mode(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    static char* parameter_name = "Mode";
    StateElement::ConditionE current_condition =
            obj->current->nscat_mode.condition;

    // mode must be current
    if (current_condition != StateElement::CURRENT)
        return(0);

    char* current_time_string = obj->current->time_string;
    unsigned char current_value = obj->current->nscat_mode.value;

    if (current_value > TLM_MODE_WOM)
    {
        // value out of range
        fprintf(ofp, label_format, error_name);
        fprintf(ofp, "  %s  %s = %d (%s)\n",
            current_time_string, parameter_name,
            current_value, mode_map[current_value]);
        ReportBasicInfo(ofp, obj->current);
        fflush(ofp);
        return(1);
    }
    return (0);
}

//------------------//
// Err_ULM_Unlocked //
//------------------//

int
Err_ULM_Unlocked(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    return Error_Flag_Transition(ofp, error_name,
        "ULM Unlocked Status (Freq. Synth. #1)", label_format,
        obj->current, &(obj->current->ulm_lock),
        obj->prev, &(obj->prev->ulm_lock),
        lock_map, 0);
}

//------------------//
// Err_SLM_Unlocked //
//------------------//

int
Err_SLM_Unlocked(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    return Error_Flag_Transition(ofp, error_name,
        "SLM Unlocked Status (Freq. Synth. #2)", label_format,
        obj->current, &(obj->current->slm_lock),
        obj->prev, &(obj->prev->slm_lock),
        lock_map, 0);
}

//----------------------//
// Err_Trip_Override_En //
//----------------------//

int
Err_Trip_Override_En(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    return Error_Flag_Transition(ofp, error_name,
        "TWTA Trip Override Enabled", label_format,
        obj->current, &(obj->current->twta_trip_override),
        obj->prev, &(obj->prev->twta_trip_override),
        twta_trip_override_map, 0);
}

//-----------------//
// Err_TWT_Mon_Dis //
//-----------------//

int
Err_TWT_Mon_Dis(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    return Error_Flag_Transition(ofp, error_name,
        "TWT Monitor Enabled", label_format,
        obj->current, &(obj->current->twt_mon_en),
        obj->prev, &(obj->prev->twt_mon_en),
        twt_mon_en_map, 1);
}

//-----------------------//
// Err_HVPS_Shutdown_Dis //
//-----------------------//

int
Err_HVPS_Shutdown_Dis(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    return Error_Flag_Transition(ofp, error_name,
        "TWT Monitor HVPS Shutdown Enabled", label_format,
        obj->current, &(obj->current->hvps_shut_en),
        obj->prev, &(obj->prev->hvps_shut_en),
        hvps_shut_en_map, 1);
}

//====================//
// L1 Error Functions //
//====================//

//------------//
// Err_Rx_Pro //
//------------//

int
Err_Rx_Pro(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    int retval;
    if (obj->prev->rx_pro.condition != StateElement::UNINITIALIZED &&
        obj->prev->nscat_mode.condition != StateElement::UNINITIALIZED &&
        obj->prev->cmf_fix.condition != StateElement::UNINITIALIZED)
    {
        // both states are valid (if prev is good, so is current)
        // check receiver protect in the previous frame
        // because it toggles one frame too early

        unsigned char expected_rx_pro;
        switch(obj->current->nscat_mode.value)
        {
        case TLM_MODE_SBM: case TLM_MODE_DBM: case TLM_MODE_CCM:
            expected_rx_pro = 1;
            break;
        case TLM_MODE_ROM: case TLM_MODE_WOM:
            if (obj->current->cmf_fix.value == TLM_FRAME_SCI)
                expected_rx_pro = 0;
            else
                expected_rx_pro = 1;
            break;
        default:
            expected_rx_pro = 1;
            break;
        } // end switch

        if (obj->prev->rx_pro.value == expected_rx_pro)
            retval = 0;
        else
        {
            fprintf(ofp, label_format, error_name);
            fprintf(ofp,
                "  %s  Rx Pro = %d (%s), Mode = %d (%s), Frame = %d (%s)\n",
                obj->prev->time_string,
                obj->prev->rx_pro.value,
                rx_pro_map[obj->prev->rx_pro.value],
                obj->prev->nscat_mode.value,
                mode_map[obj->prev->nscat_mode.value],
                obj->prev->cmf_fix.value,
                cmf_map[obj->prev->cmf_fix.value]);
            ReportBasicInfo(ofp, obj->prev);
            fprintf(ofp, "  Expected Receiver Protect = %d (%s)\n",
                expected_rx_pro, rx_pro_map[expected_rx_pro]);
            fprintf(ofp,
                "  %s  Next Frame: Mode = %d (%s), Frame = %d (%s)\n",
                obj->current->time_string,
                obj->current->nscat_mode.value,
                mode_map[obj->current->nscat_mode.value],
                obj->current->cmf_fix.value,
                cmf_map[obj->current->cmf_fix.value]);
            ReportBasicInfo(ofp, obj->current);
            retval = 1;
        }
    }
    else
    {
        retval = 0;
    }
    return (retval);
}

//---------//
// Err_CSB //
//---------//

int
Err_CSB(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{

    if (obj->current->nscat_mode.condition == StateElement::UNINITIALIZED ||
        obj->current->csb_1.condition == StateElement::UNINITIALIZED ||
        obj->current->csb_2.condition == StateElement::UNINITIALIZED ||
        obj->current->csb_3.condition == StateElement::UNINITIALIZED)
    {
        return (0); // can't check csbs
    }

    // both state are valid
    unsigned char expected_csb_1, expected_csb_2, expected_csb_3;
    switch (obj->current->nscat_mode.value)
    {
    case TLM_MODE_SBM: case TLM_MODE_DBM:
        expected_csb_1 = 0;
        expected_csb_2 = 0;
        expected_csb_3 = 0;
        break;
    case TLM_MODE_ROM: case TLM_MODE_WOM:
        if (obj->current->cmf_fix.condition == StateElement::UNINITIALIZED) {
            return (0); // can't check csbs
        }
        if (obj->current->cmf_fix.value == 1)
        {
            expected_csb_1 = 0;
            expected_csb_2 = 0;
            expected_csb_3 = 0;
        }
        else
        {
            unsigned char inv_cur_beam = ~obj->current->cur_beam.value;
            expected_csb_1 = (inv_cur_beam) & 0x1;
            expected_csb_2 = (inv_cur_beam >> 1) & 0x1;
            expected_csb_3 = (inv_cur_beam >> 2) & 0x1;
        }
        break;
    case TLM_MODE_CCM:
        expected_csb_1 = 1;
        expected_csb_2 = 1;
        expected_csb_3 = 0;
        break;
    default:
        expected_csb_1 = 0;
        expected_csb_2 = 0;
        expected_csb_3 = 0;
        break;
    } // end switch

    if (obj->current->csb_1.value == expected_csb_1 &&
        obj->current->csb_2.value == expected_csb_2 &&
        obj->current->csb_3.value == expected_csb_3)
    {
        return (0); // csbs ok
    }

    // error with csbs
    fprintf(ofp, label_format, error_name);
    fprintf(ofp, "  %s  CSB3 = %d, CSB2 = %d, CSB1 = %d\n",
        obj->current->time_string, obj->current->csb_3.value,
        obj->current->csb_2.value, obj->current->csb_1.value);
    fprintf(ofp,
        "  Beam = %2s (%1d),  Expected  CSB3 = %d, CSB2 = %d, CSB1 = %d\n",
        cur_beam_map[obj->current->cur_beam.value],
        obj->current->cur_beam.value,
        expected_csb_3, expected_csb_2, expected_csb_1);
    ReportBasicInfo(ofp, obj->current);
    fflush(ofp);
    return (1);
}

//--------------------//
// Err_Cycle_Counters //
//--------------------//

int
Err_Cycle_Counters(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    if (obj->current->beam_cycle.condition != StateElement::UNINITIALIZED &&
        obj->current->ant_cycle.condition != StateElement::UNINITIALIZED &&
        obj->current->meas_cycle.condition != StateElement::UNINITIALIZED &&
        obj->current->nscat_mode.condition != StateElement::UNINITIALIZED)
    {
        // current is valid
        if (obj->prev->beam_cycle.condition == StateElement::UNINITIALIZED ||
            obj->prev->ant_cycle.condition == StateElement::UNINITIALIZED ||
            obj->prev->meas_cycle.condition == StateElement::UNINITIALIZED ||
            obj->prev->nscat_mode.condition == StateElement::UNINITIALIZED)
        {
            return (0);     // previous is invalid
        }

        // both current and prev are valid
        // the rest of this code attempt to return (0) as soon as possible
        // if execution continues, an error is reported
        unsigned char expected_beam_cycle, expected_ant_cycle,
            expected_meas_cycle;
        char* message;
        if (obj->prev->nscat_mode.value != obj->current->nscat_mode.value)
        {
            // cycle counters should be reset for a mode change //
            expected_beam_cycle = 0;
            expected_ant_cycle = 0;
            expected_meas_cycle = 0;

            if (obj->current->beam_cycle.value == expected_beam_cycle &&
                obj->current->ant_cycle.value == expected_ant_cycle &&
                obj->current->meas_cycle.value == expected_meas_cycle)
            {
                return (0);
            }

            message = "Cycle Counter not 00/000/0 after Mode change";
        }
        else
        {
            // cycle counters should increment for same mode
            expected_beam_cycle = obj->prev->beam_cycle.value + 1;
            expected_ant_cycle = obj->prev->ant_cycle.value +
                (expected_beam_cycle / 8);
            expected_beam_cycle %= 8;
            expected_meas_cycle = obj->prev->meas_cycle.value +
                (expected_ant_cycle / 128);
            expected_ant_cycle %= 128;
            expected_meas_cycle %= 64;

            if (obj->current->beam_cycle.value == expected_beam_cycle &&
                obj->current->ant_cycle.value == expected_ant_cycle &&
                obj->current->meas_cycle.value == expected_meas_cycle)
            {
                return (0);
            }

            if (obj->current->beam_cycle.value == 0 &&
                obj->current->ant_cycle.value == 0 &&
                obj->current->meas_cycle.value == 0)
            {
                message = "Cycle Counter reset for unknown reason";
            }
            else
            {
                message = "Cycle Counter not sequential";
            }
        }
        fprintf(ofp, label_format, error_name);
        fprintf(ofp, "  %s  Cycle Counter = %02d/%03d/%01d, Mode = %s\n",
            obj->prev->time_string, obj->prev->meas_cycle.value,
            obj->prev->ant_cycle.value, obj->prev->beam_cycle.value,
            mode_map[obj->prev->nscat_mode.value]);
        ReportBasicInfo(ofp, obj->prev);
        fprintf(ofp, "  %s  Cycle Counter = %02d/%03d/%01d, Mode = %s\n",
            obj->current->time_string, obj->current->meas_cycle.value,
            obj->current->ant_cycle.value, obj->current->beam_cycle.value,
            mode_map[obj->current->nscat_mode.value]);
        fprintf(ofp, "  %s\n", message);
        ReportBasicInfo(ofp, obj->current);
        fflush(ofp);
        return (1);
    }
}

//======================//
// HK Error Functions //
//======================//

//-----------------//
// Err_Ant_2_Undep //
//-----------------//

int
Err_Ant_2_Undep(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    return Error_Flag_Transition(ofp, error_name,
        "Antenna 2 Deployment Status", label_format,
        obj->current, &(obj->current->ant_2_deploy),
        obj->prev, &(obj->prev->ant_2_deploy),
        ant_deploy_map, 1);
}

//-----------------//
// Err_Ant_5_Undep //
//-----------------//

int
Err_Ant_5_Undep(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    return Error_Flag_Transition(ofp, error_name,
        "Antenna 5 Deployment Status", label_format,
        obj->current, &(obj->current->ant_5_deploy),
        obj->prev, &(obj->prev->ant_5_deploy),
        ant_deploy_map, 1);
}

//------------------//
// Err_Ant_14_Undep //
//------------------//

int
Err_Ant_14_Undep(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    return Error_Flag_Transition(ofp, error_name,
        "Antenna 1 & 4 Deployment Status", label_format,
        obj->current, &(obj->current->ant_14_deploy),
        obj->prev, &(obj->prev->ant_14_deploy),
        ant_deploy_map, 1);
}

//------------------//
// Err_Ant_36_Undep //
//------------------//

int
Err_Ant_36_Undep(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    return Error_Flag_Transition(ofp, error_name,
        "Antenna 3 & 6 Deployment Status", label_format,
        obj->current, &(obj->current->ant_36_deploy),
        obj->prev, &(obj->prev->ant_36_deploy),
        ant_deploy_map, 1);
}

//----------------//
// Err_Relay_HVPS //
//----------------//

int
Err_Relay_HVPS(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    return (Relay2_Mismatch(ofp, error_name,
        "HVPS", "K11", "K12",
        label_format,
        obj->current, &(obj->current->hvps),
        &(obj->current->k11), &(obj->current->k12),
        hvps_map, relay_map, relay_map, k11_k12_map));
}

//--------------//
// Err_WTS_TWTA //
//--------------//

int
Err_WTS_TWTA(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    return (Relay2_Mismatch(ofp, error_name,
        "TWTA", "WTS 1", "WTS 2",
        label_format,
        obj->current, &(obj->current->twta),
        &(obj->current->wts_1), &(obj->current->wts_2),
        twta_map, wts_1_map, wts_2_map, wts1_wts2_map));
}

//----------------//
// Err_Relay_TWTA //
//----------------//

int
Err_Relay_TWTA(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    return (Relay2_Mismatch(ofp, error_name,
        "TWTA", "K09", "K10",
        label_format,
        obj->current, &(obj->current->twta),
        &(obj->current->k09), &(obj->current->k10),
        twta_map, relay_map, relay_map, k9_k10_map));
}

//---------------//
// Err_Relay_DSS //
//---------------//

int
Err_Relay_DSS(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    return (Relay2_Mismatch(ofp, error_name,
        "DSS", "K07", "K08",
        label_format,
        obj->current, &(obj->current->dss),
        &(obj->current->k07), &(obj->current->k08),
        dss_map, relay_map, relay_map, k7_k8_map));
}

//------------------//
// Err_Rep_Heat_Dis //
//------------------//

int
Err_Rep_Heat_Dis(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    if (obj->current->k04.condition == StateElement::UNINITIALIZED ||
        obj->current->k05.condition == StateElement::UNINITIALIZED ||
        obj->current->k06.condition == StateElement::UNINITIALIZED)
    {
        return(0);
    }

    // current is valid
    unsigned char rep_heat = k4_k5_k6_map[obj->current->k04.value][obj->current->k05.value][obj->current->k06.value];
    unsigned char prev_valid =
        (obj->prev->k04.condition != StateElement::UNINITIALIZED &&
        obj->prev->k05.condition != StateElement::UNINITIALIZED &&
        obj->prev->k06.condition != StateElement::UNINITIALIZED);
    unsigned char prev_rep_heat = k4_k5_k6_map[obj->prev->k04.value][obj->prev->k05.value][obj->prev->k06.value];

    if (rep_heat == SPARE_HEATER_ENABLED)
    {
        // current is ok
        if (prev_valid && prev_rep_heat != SPARE_HEATER_ENABLED)
        {
            // previous is valid but bad - error cleared
            fprintf(ofp, CLEAR_FORMAT, error_name);
            fprintf(ofp, "  %s  Replacement Heater was %s\n",
                obj->prev->time_string, heater_map[prev_rep_heat]);
            fprintf(ofp, "  K04 = %d (%s), K05 = %d (%s), K06 = %d (%s)\n",
                obj->prev->k04.value, relay_map[obj->prev->k04.value],
                obj->prev->k05.value, relay_map[obj->prev->k05.value],
                obj->prev->k06.value, relay_map[obj->prev->k06.value]);
            ReportBasicInfo(ofp, obj->prev);
            fprintf(ofp, "  %s  Replacement Heater is %s\n",
                obj->current->time_string, heater_map[rep_heat]);
            fprintf(ofp, "  K04 = %d (%s), K05 = %d (%s), K06 = %d (%s)\n",
                obj->current->k04.value, relay_map[obj->current->k04.value],
                obj->current->k05.value, relay_map[obj->current->k05.value],
                obj->current->k06.value, relay_map[obj->current->k06.value]);
            ReportBasicInfo(ofp, obj->current);
            fflush(ofp);
        }
        return(0);  // current is ok
    }
    else
    {
        // current is bad
        if (! prev_valid)
        {
            // previous is invalid - bad initialization
            fprintf(ofp, INITIALIZE_FORMAT, error_name);
            fprintf(ofp, "  %s  Replacement Heater is %s\n",
                obj->current->time_string, heater_map[rep_heat]);
            fprintf(ofp, "  K04 = %d (%s), K05 = %d (%s), K06 = %d (%s)\n",
                obj->current->k04.value, relay_map[obj->current->k04.value],
                obj->current->k05.value, relay_map[obj->current->k05.value],
                obj->current->k06.value, relay_map[obj->current->k06.value]);
            ReportBasicInfo(ofp, obj->current);
            fflush(ofp);
        }
        else if (prev_rep_heat == SPARE_HEATER_ENABLED)
        {
            // previous is ok - transition to bad
            fprintf(ofp, label_format, error_name);
            fprintf(ofp, "  %s  Replacement Heater was %s\n",
                obj->prev->time_string, heater_map[prev_rep_heat]);
            fprintf(ofp, "  K04 = %d (%s), K05 = %d (%s), K06 = %d (%s)\n",
                obj->prev->k04.value, relay_map[obj->prev->k04.value],
                obj->prev->k05.value, relay_map[obj->prev->k05.value],
                obj->prev->k06.value, relay_map[obj->prev->k06.value]);
            ReportBasicInfo(ofp, obj->prev);
            fprintf(ofp, "  %s  Replacement Heater is %s\n",
                obj->current->time_string, heater_map[rep_heat]);
            fprintf(ofp, "  K04 = %d (%s), K05 = %d (%s), K06 = %d (%s)\n",
                obj->current->k04.value, relay_map[obj->current->k04.value],
                obj->current->k05.value, relay_map[obj->current->k05.value],
                obj->current->k06.value, relay_map[obj->current->k06.value]);
            ReportBasicInfo(ofp, obj->current);
            fflush(ofp);
        }
        return(1);
    }
    return (0);     // should never get here, but let's be safe
}

//--------------------//
// Err_Spare_Heat_Dis //
//--------------------//

int
Err_Spare_Heat_Dis(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    if (obj->current->k13.condition == StateElement::UNINITIALIZED ||
        obj->current->k14.condition == StateElement::UNINITIALIZED)
    {
        return(0);
    }

    // current is valid
    unsigned char spare_heat =
        k13_k14_map[obj->current->k13.value][obj->current->k14.value];
    unsigned char prev_valid =
        (obj->prev->k13.condition != StateElement::UNINITIALIZED &&
        obj->prev->k14.condition != StateElement::UNINITIALIZED);
    unsigned char prev_spare_heat =
        k13_k14_map[obj->prev->k13.value][obj->prev->k14.value];

    if (spare_heat == SPARE_HEATER_ENABLED)
    {
        // current is ok
        if (prev_valid && prev_spare_heat != SPARE_HEATER_ENABLED)
        {
            // previous is valid but bad - error cleared
            fprintf(ofp, CLEAR_FORMAT, error_name);
            fprintf(ofp, "  %s  Spare Heater was %s\n",
                obj->prev->time_string, heater_map[prev_spare_heat]);
            fprintf(ofp, "  K13 = %d (%s), K14 = %d (%s)\n",
                obj->prev->k13.value, relay_map[obj->prev->k13.value],
                obj->prev->k14.value, relay_map[obj->prev->k14.value]);
            ReportBasicInfo(ofp, obj->prev);
            fprintf(ofp, "  %s  Spare Heater is %s\n",
                obj->current->time_string, heater_map[spare_heat]);
            fprintf(ofp, "  K13 = %d (%s), K14 = %d (%s)\n",
                obj->current->k13.value, relay_map[obj->current->k13.value],
                obj->current->k14.value, relay_map[obj->current->k14.value]);
            ReportBasicInfo(ofp, obj->current);
            fflush(ofp);
        }
        return(0);  // current is ok
    }
    else
    {
        // current is bad
        if (! prev_valid)
        {
            // bad initialization
            fprintf(ofp, INITIALIZE_FORMAT, error_name);
            fprintf(ofp, "  %s  Spare Heater is %s\n",
                obj->current->time_string, heater_map[spare_heat]);
            fprintf(ofp, "  K13 = %d (%s), K14 = %d (%s)\n",
                obj->current->k13.value, relay_map[obj->current->k13.value],
                obj->current->k14.value, relay_map[obj->current->k14.value]);
            ReportBasicInfo(ofp, obj->current);
            fflush(ofp);
        }
        else if (prev_spare_heat == SPARE_HEATER_ENABLED)
        {
            // previous is ok - transition to bad
            fprintf(ofp, label_format, error_name);
            fprintf(ofp, "  %s  Spare Heater was %s\n",
                obj->prev->time_string, heater_map[prev_spare_heat]);
            fprintf(ofp, "  K13 = %d (%s), K14 = %d (%s)\n",
                obj->prev->k13.value, relay_map[obj->prev->k13.value],
                obj->prev->k14.value, relay_map[obj->prev->k14.value]);
            ReportBasicInfo(ofp, obj->prev);
            fprintf(ofp, "  %s  Spare Heater is %s\n",
                obj->current->time_string, heater_map[spare_heat]);
            fprintf(ofp, "  K13 = %d (%s), K14 = %d (%s)\n",
                obj->current->k13.value, relay_map[obj->current->k13.value],
                obj->current->k14.value, relay_map[obj->current->k14.value]);
            ReportBasicInfo(ofp, obj->current);
            fflush(ofp);
        }
        return(1);
    }
    return (0);     // should never get here, but let's be safe
}
