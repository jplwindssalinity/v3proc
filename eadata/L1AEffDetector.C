//=========================================================//
// Copyright  (C)1996, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//
// CM Log
//
// $Log$
// 
//    Rev 1.13   01 Sep 1998 11:15:54   daffer
// Changed 3 arg AddEffect to accomodate PBI commanding
// 
//    Rev 1.12   21 Aug 1998 11:33:22   daffer
// changed some units, change initialization on some variables.
// 
//    Rev 1.11   19 Aug 1998 14:38:46   daffer
//  Add whole bunch more effects.
// 
//    Rev 1.10   23 Jul 1998 16:13:56   sally
// pass polynomial table to extractFunc()
// 
//    Rev 1.9   22 Jun 1998 09:31:02   sally
// took out incorrect print format
// 
//    Rev 1.8   22 Jun 1998 09:25:02   sally
// took out some compile errors and warnings for GNU GCC
// 
//    Rev 1.7   03 Jun 1998 10:09:08   sally
// change parameter names and types due to LP's changes
// 
//    Rev 1.6   28 May 1998 14:59:58   daffer
// changed some casts
// 
//    Rev 1.5   28 May 1998 13:19:26   daffer
// fixed bugs in DetectEffects
// 
//    Rev 1.4   22 May 1998 16:48:46   daffer
// Added/modified code to do cmdlp/effect processing
// 
//    Rev 1.3   01 May 1998 15:30:18   daffer
// Commented out include L1Extract.h.old
// 
//    Rev 1.2   01 May 1998 15:24:28   daffer
// Added pvcs header keywords
//
// $Date$
// $Revision$
// $Author$
//
//=========================================================//

static const char rcs_id[] =
    "@(#) $Id$";

#include "L1AEffDetector.h"
#include "Itime.h"

//#include "L1Extract.h.old"




typedef struct EffEntry
{
    ParamIdE    paramId;
    UnitIdE     unitId;
};


static EffEntry Eff_Table[] = 
{
    //--------------
    // TIME and REV 
    //--------------
    { UTC_TIME, UNIT_CODE_A },        // UTC Time: 
    { INSTRUMENT_TIME, UNIT_COUNTS }, //  Inst Time: 
    { ORBIT_TIME, UNIT_COUNTS },      //  Orbit Time: 
    //    { ADEOS_TIME, UNIT_TICKS},        //  ADEOS Time: 

    //-----------------
    // TABLE           
    //-----------------
    { CURRENT_MTLM_TABLE_ID, UNIT_ID },         //    TLM Table ID: 
    { STATUS_TABLE_CHANGE_FLAGS,UNIT_DN},       //  Status Table Change Flags: 
    { STATUS_TABLE_CHANGE_FLAGS_00, UNIT_MAP},  //  Mode: 
    { STATUS_TABLE_CHANGE_FLAGS_01, UNIT_MAP},  //  EQX Missed: 
    { STATUS_TABLE_CHANGE_FLAGS_02, UNIT_MAP},  //  Soft Reset: 
    { STATUS_TABLE_CHANGE_FLAGS_03, UNIT_MAP},  //  Relay Started:        
    { STATUS_TABLE_CHANGE_FLAGS_05, UNIT_MAP},  //  SES Fault Det En/Dis: 
    { STATUS_TABLE_CHANGE_FLAGS_06, UNIT_MAP},  //  SAS Fault Det En/Dis: 
    { STATUS_TABLE_CHANGE_FLAGS_07, UNIT_MAP},  //  Hard Reset:           
    { STATUS_TABLE_CHANGE_FLAGS_08, UNIT_MAP},  //  SES Supp Htr Cntrl En/Dis:  
    { STATUS_TABLE_CHANGE_FLAGS_09, UNIT_MAP},  //  TWTA Low Drive Pow FP En/Dis:
    { STATUS_TABLE_CHANGE_FLAGS_10, UNIT_MAP},  //  TWTA mon En/Dis:      
    { STATUS_TABLE_CHANGE_FLAGS_11, UNIT_MAP},  //  ES Param tables:      
    { STATUS_TABLE_CHANGE_FLAGS_12, UNIT_MAP},  // Range Gate tables:     
    { STATUS_TABLE_CHANGE_FLAGS_13, UNIT_MAP},  //  Doppler tables:       
    { STATUS_TABLE_CHANGE_FLAGS_14, UNIT_MAP},  //  Serial Tlm tables:    
    { STATUS_TABLE_CHANGE_FLAGS_15, UNIT_MAP},  //  Mission Tlm tables:   
                                          

    //-----------------
    // LEVEL 1.0 FLAGS 
    //-----------------
    { FRAME_INST_STATUS, UNIT_DN},       //  L1A frame Inst Status Flags: 
    { FRAME_INST_STATUS_00_01, UNIT_MAP},//  Current Mode: 
    { FRAME_INST_STATUS_13, UNIT_MAP},   // Grid Normal/Off
    { FRAME_INST_STATUS_14, UNIT_MAP},   // Receive Protect Normal/ON
    { FRAME_INST_STATUS_15, UNIT_MAP},   // TWTA OverRide En/Dis:
    { FRAME_INST_STATUS_19, UNIT_MAP},   // CDS Soft Reset

    { DISCRETE_STATUS_2_04, UNIT_MAP},   // TRS Cmd Success/Failure
    { DISCRETE_STATUS_3_01, UNIT_MAP},   // SES Reset.

    { FRAME_ERR_STATUS, UNIT_DN},        //  L1A Software ERROR Status Flags: 
                                     
    { SES_CONFIG_FLAGS_03, UNIT_MAP},    // Modulation on/off
    //--------
    // ERRORS 
    //--------
    { ERROR_FLAGS, UNIT_DN}, //  Error Flags: 


    //--------
    // STATUS 
    //--------

    { OPERATIONAL_MODE, UNIT_HEX_BYTES},  //  Mode(0E=WOM,07=CBM,70=SBM,E0=ROM)
    { VALID_COMMAND_COUNT, UNIT_COUNTS},  //  Valid Command Count: 
    { INVALID_COMMAND_COUNT, UNIT_COUNTS},//  Invalid Command Count: 
    { CMD_HISTORY, UNIT_HEX_BYTES},       //  CMD History Queue: 
    { SAS_A_SPIN_RATE, UNIT_MAP},         //  SAS-A Spin Rate:  
    { SAS_B_SPIN_RATE, UNIT_MAP},         //  SAS-B Spin Rate        
    { K9_TWTA_POWER, UNIT_MAP},           //  K9 TWTA Power:         
    { K10_TWTA_POWER, UNIT_MAP},          //  K10 TWTA Power:        
    { K11_TWTA_SELECT, UNIT_MAP},         //  K11 TWTA Select        
    { K12_TWTA_SELECT, UNIT_MAP},         //  K12 TWTA Select        
    { K15_SES_SELECT, UNIT_MAP},          //  K15 SES Select:        
    { K16_SES_SELECT, UNIT_MAP},          //  K16 SES Select:        
    { K19_SAS_SELECT, UNIT_MAP},          //  K19 SAS Select:        
    { K20_SAS_SELECT, UNIT_MAP},          //  K20 SAS Select:        
    { K21_SES_SUPP_HTR_PWR, UNIT_MAP},    //  K21 SES Supp Htr Pwr:  
    { K22_SES_SUPP_HTR_PWR, UNIT_MAP},    //  K22 SES Supp Htr Pwr:  
    { RANGE_GATE_A_WIDTH, UNIT_DN },
    { RANGE_GATE_B_WIDTH, UNIT_DN },
    { TRANSMIT_PULSE_WIDTH, UNIT_SECONDS},
    { RECEIVER_GAIN, UNIT_DN},
    { PRF_CYCLE_TIME, UNIT_DN},

    { PARAM_UNKNOWN, UNIT_UNKNOWN }       //  end

}; // Eff_table

//=======================//
// L1AEffDetector methods //
//=======================//

L1AEffDetector::L1AEffDetector()
{
    numTableElements=0;
    while (Eff_Table[numTableElements].paramId != PARAM_UNKNOWN)
        numTableElements++;

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
    cmd->l1aTime = effect_time;
    cmd->effectId = effect_id;
    cmd->l1aVerify = VER_YES;
    _list->Append(cmd);
    return(1);
}

int
L1AEffDetector::AddEffect(
    const Itime     effect_time,
    const EffectE   effect_id,
    const unsigned short  effect_value )
{
    Command *cmd = new Command();
    if (! cmd)
        return(0);
    cmd->l1aTime = effect_time;
    cmd->effectId = effect_id;
    cmd->l1aVerify = VER_YES;
    cmd->effect_value = effect_value;
    _list->Append(cmd);
    return(1);
}


//---------------//
// DetectEffects //
//---------------//

EffDetector::EffDetectorStatusE 
L1AEffDetector::DetectEffects(
    TlmFileList *tlmfilelist )
{
    int nchars;
    Parameter** params = new Parameter* [numTableElements+1];
    params[numTableElements]=0;
    Parameter* param=0;
    for (int j=0; j < numTableElements; j++){
        if (Eff_Table[j].paramId == PARAM_UNKNOWN)
            continue;
        
        int32 dataType=0, startIndex=0, dataLength=0, numDimensions=1;
        if (Eff_Table[j].unitId != UNIT_UNKNOWN) {
            params[j] = ParTabAccess::GetParameter(SOURCE_L1A,
                                  Eff_Table[j].paramId, Eff_Table[j].unitId);
            if (params[j] == 0) {
                nchars = sprintf( _last_msg,
                       "param[%d], unit[%d] not found\n",
                       Eff_Table[j].paramId, Eff_Table[j].unitId);
		fprintf( stderr, "%s", _last_msg );
                _status=EFFDETECTOR_ERROR;
                return (_status);
            }
            params[j]->data = (char*)malloc(params[j]->byteSize);
        }
    }
    
    int first_time=1;
    for (TlmHdfFile* tlmFile=tlmfilelist->GetHead(); tlmFile;
         tlmFile=tlmfilelist->GetNext()) {
        int i; // loop counter
        
        // open all the datasets that are listed in the Effect table
        int32 dataType=0, startIndex=0, dataLength=0, numDimensions=1;
        for (i=0; i < numTableElements; i++){
            param = params[i];
            if (param){
                assert(param->sdsIDs != 0);
                char tempString[BIG_SIZE];
                (void)strncpy(tempString, param->sdsNames, BIG_SIZE);
                char *oneSdsName = (char*)strtok(tempString, ",");
                if (oneSdsName) {
                    param->sdsIDs[0] = tlmFile->SelectDataset(
                                          oneSdsName,  dataType,
                                          startIndex, dataLength, numDimensions);
                    if (param->sdsIDs[0] == HDF_FAIL){
                        nchars=sprintf(_last_msg, "Error in select dataset %s\n",
                                       oneSdsName);
                        fprintf(stderr,"%s",_last_msg);
                        _status=EFFDETECTOR_ERROR;
                        return (_status);
                    }
                } else {
                    nchars = sprintf( _last_msg, "NULL SDS name\n");
                    fprintf(stderr,_last_msg);
                    _status=EFFDETECTOR_ERROR;
                    return (_status);
                }
            }
        }
        
        // Loop through the HDF records.
        int32 nextIndex=HDF_FAIL;
        while(tlmFile->GetNextIndex(nextIndex) == HdfFile::OK)  {
            
            // Get the time for this record and convert it to Itime.
            param=GetParam( params, UTC_TIME, tlmFile, nextIndex);
            if (!param) return(_status);
            Itime effect_time;
            effect_time.Char6ToItime( param->data );
            
            
            // Instrument Time 
            param=GetParam( params, INSTRUMENT_TIME, tlmFile, nextIndex);
            if (!param) return(_status);
            
            if (( (int) param->data )== 0 )
                AddEffect(effect_time, EFF_ELECTRONICS_ON);
            
            
            // Valid Command Count
            static unsigned int cmd_counter;
            static unsigned int last_cmd_counter;
            param=GetParam( params, VALID_COMMAND_COUNT, 
                            tlmFile, nextIndex);
            if (!param) return(_status);
            cmd_counter=(int) *(param->data);
            if (!first_time && 
                cmd_counter != last_cmd_counter )
                AddEffect( effect_time, EFF_VALID_COMMAND_CNTR_CHANGE );
            
            
            // Command History (do this before setting
            // last_cmd_counter, in case of repeated commands)
            
            static unsigned short int *cmd_history;
            static unsigned short int last_cmd_history[4];
            param=GetParam( params, CMD_HISTORY, 
                            tlmFile, nextIndex);
            if (!param) return(_status);
            cmd_history = (unsigned short int *) param->data;
            // unsigned short int cmd1 = cmd_history[0]; //&& 0XFF00;
            if (!first_time) {
                if (last_cmd_history[0] != cmd_history[0]) {
                    AddEffect( effect_time, EFF_COMMAND_HISTORY_CHANGE, 
                               cmd_history[0] );
                }else {
                    if (last_cmd_counter != cmd_counter) 
                        AddEffect( effect_time, EFF_COMMAND_HISTORY_REPEAT);
                }
            }
            for (j=0;j<4;j++) last_cmd_history[j] = cmd_history[j];
            
            last_cmd_counter = cmd_counter;
            
            
            
            // InValid Command Count
            static unsigned int inv_cmd_counter;
            static unsigned int last_inv_cmd_counter;
            param=GetParam( params, INVALID_COMMAND_COUNT, 
                            tlmFile, nextIndex);
            if (!param) return(_status);
            inv_cmd_counter=(int) *(param->data);
            if (!first_time && 
                inv_cmd_counter != last_inv_cmd_counter )
                AddEffect( effect_time, EFF_INVALID_COMMAND_CNTR_CHANGE );
            last_inv_cmd_counter = inv_cmd_counter;
            
            
            // Operational Mode
            param = GetParam( params, OPERATIONAL_MODE, tlmFile, nextIndex);
            if (!param) return(_status);
            
            
            //------//
            // Mode //
            //------//
            
            static unsigned char mode;
            static unsigned char last_mode;
            mode = (unsigned) *param->data;
            if (!first_time && last_mode != mode) {
                switch(mode) {
                case L1_MODE_SBM:
                    AddEffect(effect_time, EFF_STB);
                    break;
                case L1_MODE_ROM:
                    AddEffect(effect_time, EFF_RCV);
                    break;
                case L1_MODE_CBM:
                    AddEffect(effect_time, EFF_CAL);
                    break;
                case L1_MODE_WOM:
                    AddEffect(effect_time, EFF_WOM);
                    break;
                default:
                    AddEffect(effect_time, EFF_UNKNOWN);
                    break;
                }
            }
            last_mode = mode;
            
            
            // TWTA 1/2 , extract k11 and k12. if k11=k12, twta1 else twta2
            static int last_twta_select;
            int twta_select, k11, k12;
            param=GetParam( params, K11_TWTA_SELECT, tlmFile, nextIndex);
            if (!param) return(_status);            
            k11=(int) *(param->data);
            
            param=GetParam( params, K12_TWTA_SELECT, tlmFile, nextIndex);
            if (!param) return(_status);            
            k12=(int) *(param->data);
            
            twta_select = (k11 == k12);
            if (!first_time&& last_twta_select != twta_select ) {
                if ( twta_select )
                    AddEffect(effect_time,EFF_TWTA_1);
                else 
                    AddEffect(effect_time,EFF_TWTA_2);
            }
            last_twta_select=twta_select;
            
            
            // twta/twta replacement heaters 
            // Extract K9 and K10. 
            // K9==K10 => Power on; off otherwise.
            static int last_twta_power;
            int twta_power, k9, k10;
            param=GetParam( params, K9_TWTA_POWER, tlmFile, nextIndex);
            if (!param) return(_status);            
            k9=(int) *(param->data);
            
            param=GetParam( params, K10_TWTA_POWER, tlmFile, nextIndex);
            if (!param) return(_status);            
            k10 = (int) *(param->data);
            
            // Check to see if they're the same.
            twta_power= (k9 == k10);
            
            if (!first_time&& last_twta_power != twta_power ) {
                if (twta_power) 
                    AddEffect(effect_time,EFF_TWTA_ON);
                else 
                    AddEffect(effect_time,EFF_TWTA_REPL_HEATER_ON);
            }
            last_twta_power=twta_power;
            
            
            // SES A/B select
            static int last_ses_select;
            int ses_select, k15, k16;
            param=GetParam( params, K15_SES_SELECT, tlmFile, nextIndex);
            if (!param) return(_status);            
            k15=(int) *(param->data);

            param=GetParam( params, K16_SES_SELECT, tlmFile, nextIndex);
            if (!param) return(_status);            
            k16=(int) *(param->data);

            ses_select = (k15 == k16);
            if (!first_time&& last_ses_select != ses_select ) {
                if ( ses_select )
                    AddEffect(effect_time,EFF_SES_A);
                else 
                    AddEffect(effect_time,EFF_SES_B);
            }
            last_ses_select=ses_select;



            // SAS A/B select
            static int last_sas_select;
            int sas_select, k19, k20;
            param=GetParam( params, K19_SAS_SELECT, tlmFile, nextIndex);
            if (!param) return(_status);            
            k19=(int) *(param->data);

            param=GetParam( params, K20_SAS_SELECT, tlmFile, nextIndex);
            if (!param) return(_status);            
            k20=(int) *(param->data);

            sas_select = (k19 == k20);
            if (!first_time&& last_sas_select != sas_select ) {
                if ( sas_select )
                    AddEffect(effect_time,EFF_SAS_A);
                else 
                    AddEffect(effect_time,EFF_SAS_B);
            }
            last_sas_select=sas_select;


            //SES Supplemental Heater Relay Status
            static int last_ses_supl_heater_status;
            int ses_supl_heater_status;
            int k21, k22;

            param=GetParam(params,K21_SES_SUPP_HTR_PWR, 
                           tlmFile, nextIndex);
            if (!param) return(_status);
            k21=(int) *(param->data);

            param=GetParam(params,K22_SES_SUPP_HTR_PWR, 
                           tlmFile, nextIndex);
            if (!param) return(_status);
            k22=(int) *(param->data);

            ses_supl_heater_status= (k21==k22);
            if (!first_time &&
                last_ses_supl_heater_status != 
                ses_supl_heater_status) {
                if (ses_supl_heater_status) 
                    AddEffect( effect_time, EFF_SES_SUPPL_HEATER_ON );
                else
                    AddEffect( effect_time, EFF_SES_SUPPL_HEATER_OFF );
            }
            last_ses_supl_heater_status=ses_supl_heater_status;
            
            // TWTA Trip overide enable/disable
            static int last_twta_trip_ovrd;
            int twta_trip_ovrd;
            param=GetParam( params, FRAME_INST_STATUS_15, 
                            tlmFile, nextIndex);
            if (!param) return(_status);

            twta_trip_ovrd = (int) *(param->data);
            if (!first_time && 
                last_twta_trip_ovrd != twta_trip_ovrd ) {
                if (twta_trip_ovrd) 
                    AddEffect(effect_time,EFF_TWT_TRIP_OVERRIDE_ENABLE);
                else 
                    AddEffect(effect_time,EFF_TWT_TRIP_OVERRIDE_DISABLE);
            }
            last_twta_trip_ovrd=twta_trip_ovrd;
            
            
            // TWTA Monitor State change
            static int last_twta_monitor;
            int twta_monitor;
            param=GetParam( params, STATUS_TABLE_CHANGE_FLAGS_10, 
                            tlmFile, nextIndex);
            if (!param) return(_status);
            twta_monitor = (int) *(param->data);
            if (!first_time&& 
                last_twta_monitor != twta_monitor ) {
                if (twta_monitor) 
                    AddEffect(effect_time,EFF_TWTA_MONITOR_CHANGE);
            }
            last_twta_monitor=twta_monitor;

            // SES Multi Sequence Data Loss Response change
            static int last_ses_multiseq_dle_response;
            int ses_multiseq_dle_response; 
            param=GetParam( params, STATUS_TABLE_CHANGE_FLAGS_05, 
                            tlmFile, nextIndex);
            if (!param) return(_status);
            ses_multiseq_dle_response = (int) *(param->data);
            if (!first_time&& 
                last_ses_multiseq_dle_response != 
                ses_multiseq_dle_response ) {
                if (ses_multiseq_dle_response) 
                    AddEffect(effect_time,EFF_SES_MULTISEQ_DLE_RESPONSE_CHANGE);
            }
            last_ses_multiseq_dle_response=ses_multiseq_dle_response;


            // SAS Multi Sequence Data Loss Response change
            static int last_sas_multiseq_dle_response;
            int sas_multiseq_dle_response;
            param=GetParam( params, STATUS_TABLE_CHANGE_FLAGS_06, 
                            tlmFile, nextIndex);
            if (!param) return(_status);
            sas_multiseq_dle_response = (int) *(param->data);
            if (!first_time&& 
                last_sas_multiseq_dle_response != 
                sas_multiseq_dle_response ) {
                if (sas_multiseq_dle_response) 
                    AddEffect(effect_time,EFF_SAS_MULTISEQ_DLE_RESPONSE_CHANGE);
            }
            last_sas_multiseq_dle_response=sas_multiseq_dle_response;


            // Ses Supplemental Heater Control change
            static int last_ses_suppl_htr_cntrl;
            int ses_suppl_htr_cntrl;
            param=GetParam( params, STATUS_TABLE_CHANGE_FLAGS_08, 
                            tlmFile, nextIndex);
            if (!param) return(_status);
            ses_suppl_htr_cntrl = (int) *(param->data);
            if (!first_time&& 
                last_ses_suppl_htr_cntrl != 
                ses_suppl_htr_cntrl ) {
                if (ses_suppl_htr_cntrl) 
                    AddEffect(effect_time,EFF_SES_SUPPL_HEATER_CNTRL_CHANGE);
            }
            last_ses_suppl_htr_cntrl=ses_suppl_htr_cntrl;


            //SAS A Spin Rate
            static int last_sas_a_spin_rate;
            int sas_a_spin_rate;
            
            param=GetParam(params,SAS_A_SPIN_RATE, 
                           tlmFile, nextIndex);
            if (!param) return(_status);
            sas_a_spin_rate=(int) *(param->data);
            if (!first_time  &&
                last_sas_a_spin_rate != 
                sas_a_spin_rate) {
                if (sas_a_spin_rate) 
                    AddEffect( effect_time, EFF_SAS_A_SPIN_RATE_198 );
                else
                    AddEffect( effect_time, EFF_SAS_A_SPIN_RATE_180 );
            }
            last_sas_a_spin_rate = sas_a_spin_rate;
                

            //SAS B Spin Rate
            static int last_sas_b_spin_rate;
            int sas_b_spin_rate;
            
            param=GetParam(params,SAS_B_SPIN_RATE, 
                           tlmFile, nextIndex);
            if (!param) return(_status);
            sas_b_spin_rate=(int) *(param->data);
            if (!first_time  &&
                last_sas_b_spin_rate != 
                sas_b_spin_rate) {
                if (sas_b_spin_rate) 
                    AddEffect( effect_time, EFF_SAS_B_SPIN_RATE_198 );
                else
                    AddEffect( effect_time, EFF_SAS_B_SPIN_RATE_180 );
            }
            last_sas_b_spin_rate = sas_b_spin_rate;
                
            // -------------------------------------
            // Status Change Flags
            // -------------------------------------

            
            // SES params table switch
            static int last_ses_params_switch;
            int ses_params_switch;
            param=GetParam(params, STATUS_TABLE_CHANGE_FLAGS_11, 
                           tlmFile, nextIndex);
            if (!param) return(_status);
            ses_params_switch=(int) *(param->data);
            if (!first_time &&
                last_ses_params_switch != ses_params_switch )
                if (ses_params_switch)
                    AddEffect( effect_time, EFF_SES_PARAMS_TABLE_CHANGE );
            last_ses_params_switch=ses_params_switch;


            // Range gate table switch
            static int last_range_gate_table_switch;
            int range_gate_table_switch;
            param=GetParam(params,STATUS_TABLE_CHANGE_FLAGS_12, 
                           tlmFile, nextIndex);
            if (!param) return(_status);
            range_gate_table_switch=(int) *(param->data);
            if (!first_time &&
                last_range_gate_table_switch != range_gate_table_switch ) 
                if (range_gate_table_switch)
                    AddEffect( effect_time, EFF_RANGEGATE_TABLE_CHANGE );
            
            last_range_gate_table_switch=range_gate_table_switch;
            
            

            // Doppler table switch
            static int last_doppler_table_switch;
            int doppler_table_switch;
            param=GetParam(params,STATUS_TABLE_CHANGE_FLAGS_13, 
                           tlmFile, nextIndex);
            if (!param) return(_status);
            doppler_table_switch=(int) *(param->data);
            if (!first_time &&
                last_doppler_table_switch != doppler_table_switch )
                if (doppler_table_switch)
                    AddEffect( effect_time, EFF_DOPPLER_TABLE_CHANGE );
            last_doppler_table_switch=doppler_table_switch;

            // Mission TLM table switch (PRF table)
            static int last_mission_tlm_table_switch;
            int mission_tlm_table_switch;
            param=GetParam(params,STATUS_TABLE_CHANGE_FLAGS_14, 
                           tlmFile, nextIndex);
            if (!param) return(_status);
            mission_tlm_table_switch=(int) *(param->data);
            if (!first_time &&
                last_mission_tlm_table_switch != mission_tlm_table_switch )
                if (mission_tlm_table_switch)
                    AddEffect( effect_time, EFF_MISSION_TELEM_TABLE_CHANGE );

            last_mission_tlm_table_switch=mission_tlm_table_switch;

            // Get gate A Width
            static int last_rangegatewidth_a;
            int rangegatewidth_a;
            param=GetParam( params, RANGE_GATE_A_WIDTH,
                            tlmFile, nextIndex);
            if (!param) return(_status);
            rangegatewidth_a = (int) *(param->data);
            if (!first_time &&
                last_rangegatewidth_a != rangegatewidth_a) 
                AddEffect( effect_time, EFF_RANGE_GATE_A_WIDTH_CHANGE );
            last_rangegatewidth_a=rangegatewidth_a;


            // Get gate B Width
            static int last_rangegatewidth_b;
            int rangegatewidth_b;
            param=GetParam( params, RANGE_GATE_B_WIDTH,
                            tlmFile, nextIndex);
            if (!param) return(_status);
            rangegatewidth_b = (int) *(param->data);
            if (!first_time &&
                last_rangegatewidth_b != rangegatewidth_b) 
                AddEffect( effect_time, EFF_RANGE_GATE_B_WIDTH_CHANGE );
            last_rangegatewidth_b=rangegatewidth_b;


            // Get Transmit Pulse width
            static float last_transmit_pulse_width;
            float transmit_pulse_width;
            param=GetParam( params, TRANSMIT_PULSE_WIDTH,
                            tlmFile, nextIndex);
            if (!param) return(_status);
            transmit_pulse_width = (float) *(param->data);
            if (!first_time &&
                last_transmit_pulse_width != transmit_pulse_width) 
                AddEffect( effect_time, EFF_TRANSMIT_PULSE_WIDTH_CHANGE );
            last_transmit_pulse_width=transmit_pulse_width;



            // TRS Cmd Success
            param=GetParam( params, DISCRETE_STATUS_2_04,
                            tlmFile, nextIndex);
            if (!param) return(_status);
            static int trs = (int) *(param->data);
            if (!first_time &&
                trs==0) 
                AddEffect( effect_time, EFF_TRS_CHANGE );
            

            // Get receiver gain
            static int last_receiver_gain;
            int receiver_gain;
            param=GetParam( params, RECEIVER_GAIN,
                            tlmFile, nextIndex);
            if (!param) return(_status);
            receiver_gain = (int) *(param->data);
            if (!first_time &&
                last_receiver_gain != receiver_gain) 
                AddEffect( effect_time, EFF_RECEIVER_GAIN_CHANGE);
            last_receiver_gain=receiver_gain;

            
            // Grid Normal/Disable
            static int last_grid;
            int grid;
            param=GetParam( params, FRAME_INST_STATUS_13,
                            tlmFile, nextIndex);
            if (!param) return(_status);
            grid = (int) *(param->data);
            if (!first_time &&
                grid != last_grid) {
                if (grid) 
                    AddEffect( effect_time, EFF_GRID_DISABLED );
                else
                    AddEffect( effect_time, EFF_GRID_NORMAL );
            }            
            last_grid=grid;

            // Receive_Protect Normal/Disable
            static int last_receive_protect;
            int receive_protect;
            param=GetParam( params, FRAME_INST_STATUS_14,
                            tlmFile, nextIndex);
            if (!param) return(_status);
            receive_protect = (int) *(param->data);
            if (!first_time &&
                receive_protect != last_receive_protect) {
                if (receive_protect) 
                    AddEffect( effect_time, EFF_RECEIVE_PROTECT_ON );
                else
                    AddEffect( effect_time, EFF_RECEIVE_PROTECT_NORMAL );
            }            
            last_receive_protect=receive_protect;

            // CDS Soft Reset
            static int last_cds_soft_reset;
            int cds_soft_reset;
            param=GetParam( params, FRAME_INST_STATUS_19,
                            tlmFile, nextIndex);
            if (!param) return(_status);
            cds_soft_reset = (int) *(param->data);
            if (!first_time &&
                cds_soft_reset != last_cds_soft_reset) {
                if (cds_soft_reset) 
                    AddEffect( effect_time, EFF_CDS_SOFT_RESET);

            }            
            last_cds_soft_reset = cds_soft_reset;

            // SES Reset
            static int last_ses_reset;
            int ses_reset;
            param=GetParam( params, DISCRETE_STATUS_3_01,
                            tlmFile, nextIndex);
            if (!param) return(_status);
            ses_reset = (int) *(param->data);
            if (!first_time &&
                ses_reset != last_ses_reset) {
                if (ses_reset) 
                    AddEffect( effect_time, EFF_SES_RESET);

            }            
            last_ses_reset = ses_reset;


            
            // Modulation
            static int last_modulation;
            int modulation ;
            param=GetParam( params, SES_CONFIG_FLAGS_03,
                            tlmFile, nextIndex);
            if (!param) return(_status);
            modulation = (int) *(param->data);
            if (!first_time &&
                modulation != last_modulation) {
                if (modulation) 
                    AddEffect( effect_time, EFF_MODULATION_ON );
                else
                    AddEffect( effect_time, EFF_MODULATION_OFF );
            }            
            last_modulation=modulation;

            // Twta Low Drive FP on/off
            static int last_twta_lowdrivefp;
            int twta_lowdrivefp;
            param=GetParam( params, STATUS_TABLE_CHANGE_FLAGS_09,
                            tlmFile, nextIndex);
                            
            if (!param) return(_status);
            twta_lowdrivefp = (int) *(param->data);
            if (!first_time &&
                twta_lowdrivefp != last_twta_lowdrivefp) {
                if (twta_lowdrivefp) 
                    AddEffect( effect_time, EFF_TWTA_LOWDRIVE_POWER_FP_ON );
                else
                    AddEffect( effect_time, EFF_TWTA_LOWDRIVE_POWER_FP_OFF );
            }            
            last_twta_lowdrivefp=twta_lowdrivefp;
            
            // Prf clock rate
            static int last_prf_cycle_time;
            int prf_cycle_time;
            param=GetParam( params, PRF_CYCLE_TIME,
                            tlmFile, nextIndex);
            if (!param) return(_status);
            prf_cycle_time = (int) *(param->data);
            if (!first_time &&
                prf_cycle_time != last_prf_cycle_time) 
              AddEffect( effect_time, EFF_PRF_CLOCK_CHANGE);
            last_prf_cycle_time=prf_cycle_time;
            

            first_time = 0;

        }
            
        // close all the datasets of this HDF file
        for (i=0; i < numTableElements; i++) {
            param = params[i];
            if (param) free ((void*)(param->data));
            if (param && param->sdsIDs != 0 && param->sdsIDs[0] != HDF_FAIL)
                (void)tlmFile->CloseDataset(param->sdsIDs[0]);
            
        }
    }
    
    return(_status = EFFDETECTOR_OK);
}

/*
//========================
// L1ApEffDetector methods 
//========================

L1ApEffDetector::L1ApEffDetector()
{
    return;
}

L1ApEffDetector::~L1ApEffDetector()
{
    return;
}

//=============
// AddEffect 
//=============

int
L1ApEffDetector::AddEffect(
    const Itime     effect_time,
    const EffectE   effect_id)
{
    Command *cmd = new Command();
    if (! cmd)
        return(0);
    cmd->l1aTime = effect_time;
    cmd->effectId = effect_id;
    cmd->l1aVerify = VER_YES;
    _list->Append(cmd);
    return(1);
}


int
L1ApEffDetector::AddEffect(
    const Itime     effect_time,
    const EffectE   effect_id,
    (char * )       effect_value )
{
    Command *cmd = new Command();
    if (! cmd)
        return(0);
    cmd->l1apTime = effect_time;
    cmd->effectId = effect_id;
    cmd->l1apVerify = VER_YES;
    cmd->effect_value = effect_value;
    _list->Append(cmd);
    return(1);
}
*/







