//=========================================================//
// Copyright  (C)1996, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id[] =
    "@(#) $Id$";

#include <stdio.h>
#include "L1AErrorChecker.h"
#include "L1Extract.h.old"
#include "Parameter.h"
#include "State.h"

//==============//
// Error Tables //
//==============//

ErrorEntry  l1_error_table_1[] =
{
    {"CSB/Beam Mismatch", ERROR_FORMAT, Err_CSB},
    {"Invalid Mode", ERROR_FORMAT, Err_Mode},
    {NULL, NULL, NULL}
};

ErrorEntry  l1_error_table_2[] =
{
    {"Receiver Protect in Wrong State", ERROR_FORMAT, Err_Rx_Pro},
    {"HVPS Backup Commanded", ERROR_FORMAT, Err_HVPS_Backup_Off},
    {"TWTA Undervoltage Trip", ERROR_FORMAT, Err_TWTA_UV_Trip},
    {"TWTA Overcurrent Trip", ERROR_FORMAT, Err_TWTA_OC_Trip},
    {"TWTA Body Overcurrent Trip", ERROR_FORMAT, Err_TWTA_Body_OC_Trip},
//  {"Binning Parameter Error (Regressive)", WARNING_FORMAT, Err_Bin_Param},
    {"Default Binning Constants in Use", WARNING_FORMAT, Err_Def_Bin_Const},
    {"Lack Startup Requirements", WARNING_FORMAT, Err_Lack_Start_Reqs},
    {"Error Queue Full", ERROR_FORMAT, Err_Error_Queue_Full},
    {"Fault Counter Change", ERROR_FORMAT, Err_Fault_Counter},
    {"ULM Unlocked", ERROR_FORMAT, Err_ULM_Unlocked},
    {"SLM Unlocked", ERROR_FORMAT, Err_SLM_Unlocked},
    {"TWT Trip Override Enabled", WARNING_FORMAT, Err_Trip_Override_En},
    {"TWT Monitor Disabled", WARNING_FORMAT, Err_TWT_Mon_Dis},
    {"TWT Monitor HVPS Shutdown Disabled", WARNING_FORMAT,
        Err_HVPS_Shutdown_Dis},
    {"Unexpected Cycle Counter", WARNING_FORMAT, Err_Cycle_Counters},
    {NULL, NULL, NULL}
};

//========================//
// L1AErrorChecker methods //
//========================//

L1AErrorChecker::L1AErrorChecker()
{
    errorTable1 = l1_error_table_1;
    errorTable2 = l1_error_table_2;

    for (int i = 0; i < 32; i++)
        errorMessageCount[i] = 0;

    return;
}

L1AErrorChecker::~L1AErrorChecker()
{
    return;
}

//----------//
// SetState //
//----------//

void
L1AErrorChecker::SetState(
    State*      state,
    State*      last_state,
    char*       dataRec)
{
    state->time.value = L1A_Itime(dataRec);
    state->time.value.ItimeToCodeA(state->time_string);
    state->time.condition = StateElement::CURRENT;

    state->hvps_backup_off.condition =
        state->StateExtract(dataRec, L1A_HVPS_Backup_Off,
        (char *)&(state->hvps_backup_off.value),
        last_state->hvps_backup_off.condition);

    state->twta_uv_trip.condition =
        state->StateExtract(dataRec, L1A_Valid_TWTA_UV_Trip,
        (char *)&(state->twta_uv_trip.value),
        last_state->twta_uv_trip.condition);

    state->twta_oc_trip.condition =
        state->StateExtract(dataRec, L1A_Valid_TWTA_OC_Trip,
        (char *)&(state->twta_oc_trip.value),
        last_state->twta_oc_trip.condition);

    state->twta_body_oc_trip.condition =
        state->StateExtract(dataRec, L1A_Valid_TWTA_Body_OC_Trip,
        (char *)&(state->twta_body_oc_trip.value),
        last_state->twta_body_oc_trip.condition);

    state->bin_param_err.condition =
        state->StateExtract(dataRec, L1A_Bin_Param_Err,
        (char *)&(state->bin_param_err.value),
        last_state->bin_param_err.condition);

    state->def_bin_const.condition =
        state->StateExtract(dataRec, L1A_Def_Bin_Const,
        (char *)&(state->def_bin_const.value),
        last_state->def_bin_const.condition);

    state->lack_start_reqs.condition =
        state->StateExtract(dataRec, L1A_Lack_Start_Reqs,
        (char *)&(state->lack_start_reqs.value),
        last_state->lack_start_reqs.condition);

    state->err_queue_full.condition =
        state->StateExtract(dataRec, L1A_Err_Queue_Full,
        (char *)&(state->err_queue_full.value),
        last_state->err_queue_full.condition);

    state->fault_counter.condition =
        state->StateExtract(dataRec, L1A_Fault_Counter,
        (char *)&(state->fault_counter.value),
        last_state->fault_counter.condition);

    state->nscat_mode.condition =
        state->StateExtract(dataRec, L1A_NSCAT_Mode,
        (char *)&(state->nscat_mode.value),
        last_state->nscat_mode.condition);

    state->twta.condition =
        state->StateExtract(dataRec, L1A_Valid_TWTA,
        (char *)&(state->twta.value),
        last_state->twta.condition);

    state->dss.condition =
        state->StateExtract(dataRec, L1A_DSS,
        (char *)&(state->dss.value),
        last_state->dss.condition);

    state->ulm_lock.condition =
        state->StateExtract(dataRec, L1A_ULM_Lock,
        (char *)&(state->ulm_lock.value),
        last_state->ulm_lock.condition);

    state->slm_lock.condition =
        state->StateExtract(dataRec, L1A_SLM_Lock,
        (char *)&(state->slm_lock.value),
        last_state->slm_lock.condition);

    state->twta_trip_override.condition =
        state->StateExtract(dataRec, L1A_TWTA_Trip_Override_Enable,
        (char *)&(state->twta_trip_override.value),
        last_state->twta_trip_override.condition);

    state->twt_mon_en.condition =
        state->StateExtract(dataRec, L1A_TWT_Mon_En,
        (char *)&(state->twt_mon_en.value),
        last_state->twt_mon_en.condition);

    state->hvps_shut_en.condition =
        state->StateExtract(dataRec, L1A_HVPS_Shut_En,
        (char *)&(state->hvps_shut_en.value),
        last_state->hvps_shut_en.condition);

    state->rx_pro.condition =
        state->StateExtract(dataRec, L1A_Rx_Pro,
        (char *)&(state->rx_pro.value),
        last_state->rx_pro.condition);

    state->cmf_fix.condition =
        state->StateExtract(dataRec, L1A_CMF_Fix,
        (char *)&(state->cmf_fix.value),
        last_state->cmf_fix.condition);

    state->cur_beam.condition =
        state->StateExtract(dataRec, L1A_Cur_Beam,
        (char *)&(state->cur_beam.value),
        last_state->cur_beam.condition);

    state->csb_3.condition =
        state->StateExtract(dataRec, L1A_CSB_3,
        (char *)&(state->csb_3.value),
        last_state->csb_3.condition);

    state->csb_2.condition =
        state->StateExtract(dataRec, L1A_CSB_2,
        (char *)&(state->csb_2.value),
        last_state->csb_2.condition);

    state->csb_1.condition =
        state->StateExtract(dataRec, L1A_CSB_1,
        (char *)&(state->csb_1.value),
        last_state->csb_1.condition);

    state->error_msg.condition =
        state->StateExtract(dataRec, L1A_Error_Msg,
        (char *)&(state->error_msg.value),
        last_state->error_msg.condition);

    state->meas_cycle.condition =
        state->StateExtract(dataRec, L1A_Meas_Cycle,
        (char *)&(state->meas_cycle.value),
        last_state->meas_cycle.condition);

    state->ant_cycle.condition =
        state->StateExtract(dataRec, L1A_Ant_Cycle,
        (char *)&(state->ant_cycle.value),
        last_state->ant_cycle.condition);

    state->beam_cycle.condition =
        state->StateExtract(dataRec, L1A_Beam_Cycle,
        (char *)&(state->beam_cycle.value),
        last_state->beam_cycle.condition);

    return;
}

//-------//
// Check //
//-------//

void
L1AErrorChecker::Check(
    FILE*   ofp,
    char*   dataRec)
{
    //-----------------------//
    // set the current state //
    //-----------------------//
 
    SetState(current, prev, dataRec);
 
    //------------------------------//
    // perform each two state check //
    //------------------------------//
 
    int index = 0;
    do
    {
        ErrorEntry* entry = errorTable2 + index;
        if (entry->name == NULL)
            break;
        else
            entry->count += entry->function(this, ofp,
                                entry->name, entry->format);
        index++;
    } while (1);

    //------------------------------//
    // perform each one state check //
    //------------------------------//
 
    index = 0;
    do
    {
        ErrorEntry* entry = errorTable1 + index;
        if (entry->name == NULL)
            break;
        else
            entry->count += entry->function(this, ofp,
                                entry->name, entry->format);
        index++;
    } while (1);
 
    //-------------------------//
    // check the error message //
    //-------------------------//

    unsigned char msg = Err_Error_Msg(this, ofp, ERROR_MSG_NAME,
        ERROR_FORMAT);
    errorMessageCount[msg]++;

    //-------------------------//
    // save the previous state //
    //-------------------------//
 
    *prev = *current;
 
    //------------------------//
    // count the data records //
    //------------------------//
 
    dataRecCount++;
    return;
}

//-----------//
// Summarize //
//-----------//

#define PERCENT(A) ((float)(A)*100.0/(float)dataRecCount)

void
L1AErrorChecker::Summarize(
    FILE*   ofp)
{
    //------------//
    // basic info //
    //------------//

    fprintf(ofp, "Number of Data Records: %d\n", dataRecCount);

    //----------------//
    // error messages //
    //----------------//

    fprintf(ofp, "\nERROR MESSAGES\n");
    fprintf(ofp, "    Frames   Percentage   Error #   Error Message\n");
    int i;
    for (i = 0; i < 32; i++)
    {
        if (errorMessageCount[i])
        {
            fprintf(ofp, "%10u   %10.4f   %7d   %s\n", errorMessageCount[i],
                PERCENT(errorMessageCount[i]), i, error_msg_map[i]);
        }
    }

    //--------------//
    // other errors //
    //--------------//

    fprintf(ofp, "\nERROR SUMMARY\n");
    fprintf(ofp, "    Frames   Percentage   Error Description\n");

    ErrorEntry* entry;
    for (i = 0; ; i++)
    {
        entry = errorTable1 + i;
        if (! entry->name)
            break;
        fprintf(ofp, "%10d   %10.4f   %s\n", entry->count,
            PERCENT(entry->count), entry->name);
    }

    for (i = 0; ; i++)
    {
        entry = errorTable2 + i;
        if (! entry->name)
            break;
        fprintf(ofp, "%10d   %10.4f   %s\n", entry->count,
            PERCENT(entry->count), entry->name);
    }

    if (! AnyErrors())
        fprintf(ofp, "\nNo errors detected.\n");

    return;
}

//---------------//
// Err_Error_Msg //
//---------------//

int
Err_Error_Msg(
    ErrorChecker*   obj,
    FILE*           ofp,
    char*           error_name,
    char*           label_format)
{
    Error_Flag_Transition(ofp, error_name,
        "Error Message", label_format,
        obj->current, &(obj->current->error_msg),
        obj->prev, &(obj->prev->error_msg),
        error_msg_map, 0);
    return (obj->current->error_msg.value);
}

//-----------//
// AnyErrors //
//-----------//
// returns non-zero if there were any errors

int
L1AErrorChecker::AnyErrors()
{
    int i;
    ErrorEntry* entry;
    for (i = 0; ; i++)
    {
        entry = errorTable1 + i;
        if (! entry->name)
            break;
        if (entry->count > 0)
            return(1);
    }
    for (i = 0; ; i++)
    {
        entry = errorTable2 + i;
        if (! entry->name)
            break;
        if (entry->count > 0)
            return(1);
    }
}

