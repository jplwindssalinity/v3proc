//=========================================================//
// Copyright  (C)1995, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef STATE_H
#define STATE_H

static const char rcs_id_state_h[]="@(#) $Id$";

#include "Parameter.h"
#include "Itime.h"

//==============//
// StateElement //
//==============//

class StateElement
{
public:
    enum ConditionE
    {
        UNINITIALIZED = 0,
        CURRENT,
        HELD
    };

    StateElement();
    ~StateElement();

    ConditionE      condition;
};

class StateElement_uc : public StateElement
{
public:
    unsigned char   value;
};

class StateElement_int : public StateElement
{
public:
    unsigned int    value;
};

class StateElement_it : public StateElement
{
public:
    Itime           value;
};

//=======//
// State //
//=======//

class State
{
public:

    State();
    ~State();

    StateElement::ConditionE    StateExtract(
                            const char*                 dataRec,
                            ExtractFunc                 function,
                            char*                       data,
                            StateElement::ConditionE    prev_condition);

    char            time_string[CODEA_TIME_LEN];

    StateElement_it time;

    StateElement_uc twta;
    StateElement_uc hvps;
    StateElement_uc dss;
    StateElement_uc twta_trip_override;
    StateElement_uc cmf_fix;
    StateElement_uc nscat_mode;
    StateElement_uc hvps_shut_en;
    StateElement_uc twt_mon_en;
    StateElement_uc fault_counter;
    StateElement_uc cmd_counter;
    StateElement_uc new_bc;
    StateElement_uc new_ant_seq;
    StateElement_uc cur_beam;
    StateElement_uc csb_3;
    StateElement_uc csb_2;
    StateElement_uc csb_1;
    StateElement_uc rx_pro;
    StateElement_uc error_msg;
    StateElement_uc lack_start_reqs;
    StateElement_uc err_queue_full;
    StateElement_uc bin_param_err;
    StateElement_uc def_bin_const;
    StateElement_uc twta_body_oc_trip;
    StateElement_uc twta_oc_trip;
    StateElement_uc twta_uv_trip;
    StateElement_uc slm_lock;
    StateElement_uc ulm_lock;
    StateElement_uc hvps_backup_off;
    StateElement_uc meas_cycle;
    StateElement_uc ant_cycle;
    StateElement_uc beam_cycle;
    StateElement_uc ant_2_deploy;
    StateElement_uc ant_5_deploy;
    StateElement_uc ant_14_deploy;
    StateElement_uc ant_36_deploy;
    StateElement_uc wts_1;
    StateElement_uc wts_2;
    StateElement_uc k01;
    StateElement_uc k02;
    StateElement_uc k03;
    StateElement_uc k04;
    StateElement_uc k05;
    StateElement_uc k06;
    StateElement_uc k07;
    StateElement_uc k08;
    StateElement_uc k09;
    StateElement_uc k10;
    StateElement_uc k11;
    StateElement_uc k12;
    StateElement_uc k13;
    StateElement_uc k14;
    StateElement_uc diu_15v;

    StateElement_int    inst_time;
};

#endif // STATE_H
