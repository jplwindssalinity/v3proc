//=========================================================//
// Copyright  (C)1996, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//
// CM Log
//
// $Log$
// 
//    Rev 1.1   23 Jun 1999 11:50:44   sally
// took out unused var "effectid"
// 
//    Rev 1.0   25 Mar 1999 13:57:56   daffer
// Initial Revision
// $Date$
// $Revision$
// $Author$
//
//=========================================================//

static const char rcs_id[] =
    "@(#) $Id$";

#include "Hk2EffDetector.h"
#include "Itime.h"
#include "EALog.h"
#include "Parameter.h"
#include "ParTab.h"
#include "State.h"
#include "TlmHdfFile.h"

typedef struct EffEntry
{
    ParamIdE    paramId;
    UnitIdE     unitId;
};


static EffEntry Eff_Table[] = 
{
    
    { UTC_TIME, UNIT_AUTOTIME }, // Hk2 time (char 6)
    { OPERATIONAL_MODE, UNIT_MAP },
    { VALID_COMMAND_COUNT, UNIT_DN},
    { INVALID_COMMAND_COUNT, UNIT_DN },
    { SES_TRS_CMD_SUCC, UNIT_MAP },
    { SES_RX_PROTECT, UNIT_MAP },
    { GRID_INHIBIT, UNIT_MAP },
    { TWT_BODY_OC_TRIP, UNIT_MAP },
    { SAS_B_SPIN_RATE, UNIT_MAP },
    { SAS_A_SPIN_RATE, UNIT_MAP },
    { K9_TWTA_POWER, UNIT_MAP },
    { K10_TWTA_POWER, UNIT_MAP },
    { K11_TWTA_SELECT, UNIT_MAP },
    { K12_TWTA_SELECT, UNIT_MAP },
    { K15_SES_SELECT, UNIT_MAP },
    { K16_SES_SELECT, UNIT_MAP },
    { K19_SAS_SELECT, UNIT_MAP },
    { K20_SAS_SELECT, UNIT_MAP },
    { K21_SES_SUPP_HTR_PWR, UNIT_MAP },
    { K22_SES_SUPP_HTR_PWR, UNIT_MAP },
    { PARAM_UNKNOWN, UNIT_UNKNOWN }       //  end

};



//=======================//
// Hk2EffDetector methods //
//=======================//

Hk2EffDetector::Hk2EffDetector()
{
    numTableElements=0;
    while (Eff_Table[numTableElements].paramId != PARAM_UNKNOWN)
        numTableElements++;
    return;
}

Hk2EffDetector::~Hk2EffDetector()
{
    return;
}

//-----------//
// Create Effect Entry
//-----------//

Command *
Hk2EffDetector::CreateEffectEntry(
    const Itime     effect_time,
    const EffectE   effect_id)
{
    Command *cmd = new Command();
    if (cmd) {
        cmd->hk2Time = effect_time;
        cmd->effectId = effect_id;
        cmd->hk2Verify = VER_YES;
    }
    return(cmd);
}



//-----------//
// AddEffect //
//-----------//
int
Hk2EffDetector::AddEffect(
                          Command *cmd)
{
    _list->Append(cmd);
    return(1);
}


//-----------//
// AddEffect //
//-----------//

int
Hk2EffDetector::AddEffect(
    const Itime     effect_time,
    const EffectE   effect_id)
{
    Command *cmd = new Command();
    if (! cmd)
        return(0);
    cmd->hk2Time = effect_time;
    cmd->effectId = effect_id;
    cmd->hk2Verify = VER_YES;
    _list->Append(cmd);
    return(1);
}

int
Hk2EffDetector::AddEffect(
    const Itime     effect_time,
    const EffectE   effect_id,
    const unsigned short  effect_value )
{
    Command *cmd = new Command();
    if (! cmd)
        return(0);
    cmd->hk2Time = effect_time;
    cmd->effectId = effect_id;
    cmd->hk2Verify = VER_YES;
    cmd->effect_value = effect_value;
    _list->Append(cmd);
    return(1);
}


//---------------//
// DetectEffects //
//---------------//

EffDetector::EffDetectorStatusE 
Hk2EffDetector::DetectEffects(
    TlmFileList *tlmfilelist )
{
    Command *effect = 0;
    Itime effect_time;

    int nchars;
    Parameter** params = new Parameter* [numTableElements+1];
    params[numTableElements]=0;
    Parameter* param=0;

    static int cmd_history_change = 0;

    for (int j=0; j < numTableElements; j++) {
        if (Eff_Table[j].paramId == PARAM_UNKNOWN)
            continue;
        
        int32 dataType=0, startIndex=0, dataLength=0, numDimensions=1;
        if (Eff_Table[j].unitId != UNIT_UNKNOWN) {
            params[j] = ParTabAccess::GetParameter(SOURCE_HK2,
                                  Eff_Table[j].paramId, Eff_Table[j].unitId);
            if (params[j] == 0) {
                nchars = sprintf( _last_msg,
                       "param[%d], unit[%d] not found\n",
                       Eff_Table[j].paramId, Eff_Table[j].unitId);
		fprintf( stderr, "%s", _last_msg );
                return (_status=EFFDETECTOR_ERROR);
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
        for (i=0; i < numTableElements; i++) {
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
                        return (_status =EFFDETECTOR_ERROR);
                    }
                } else {
                    nchars = sprintf( _last_msg, "NULL SDS name\n");
                    fprintf(stderr,_last_msg);
                    return (_status=EFFDETECTOR_ERROR);
                }
            }
        }
        
        // Loop through the HDF records.
        int32 nextIndex=HDF_FAIL;
        while(tlmFile->GetNextIndex(nextIndex) == HdfFile::OK)  {

            // Get the time for this record and convert it to Itime.
            param=GetParam( params, UTC_TIME, 
                            tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, "Error extracting UTC_TIME\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
            effect_time.Char6ToItime( param->data );
            
            // Valid Command Count
            static unsigned int cmd_counter;
            static unsigned int last_cmd_counter;
            param=GetParam( params, VALID_COMMAND_COUNT, 
                            tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr,"Error extracting VALID_COMMAND_COUNT\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
            cmd_counter=(int) *(param->data);
            if (!first_time && 
                cmd_counter != last_cmd_counter )
                AddEffect( effect_time, EFF_VALID_COMMAND_CNTR_CHANGE );
            
            
            // InValid Command Count
            static unsigned int inv_cmd_counter;
            static unsigned int last_inv_cmd_counter;
            param=GetParam( params, INVALID_COMMAND_COUNT, 
                            tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, "Error extracting INVALID_COMMAND_COUNT\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
            inv_cmd_counter=(int) *(param->data);
            if (!first_time && 
                inv_cmd_counter != last_inv_cmd_counter )
                AddEffect( effect_time, EFF_INVALID_COMMAND_CNTR_CHANGE );


            
            // Operational Mode
            param = GetParam( params, OPERATIONAL_MODE, 
                              tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, "Error extracting OPERATIONAL_MODE\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
            
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
            param=GetParam( params, K11_TWTA_SELECT, 
                            tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, "Error extracting K11_TWTA_SELECT\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);       
            }     
            k11=(int) *(param->data);
            
            param=GetParam( params, K12_TWTA_SELECT, 
                            tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, "Error extracting K12_TWTA_SELECT\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);       
            }     
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
            param=GetParam( params, K9_TWTA_POWER, 
                            tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, "Error extracting K9_TWTA_POWER\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);     
            }       
            k9=(int) *(param->data);
            
            param=GetParam( params, K10_TWTA_POWER, 
                            tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, "Error extracting K10_TWTA_POWER\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);      
            }      
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
            param=GetParam( params, K15_SES_SELECT, 
                            tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, "Error extracting K15_SES_SELECT\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);      
            }      
            k15=(int) *(param->data);
            
            param=GetParam( params, K16_SES_SELECT, 
                            tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, "Error extracting K16_SES_SELECT\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);      
            }      
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
            param=GetParam( params, K19_SAS_SELECT, 
                            tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, "Error extracting K19_SAS_SELECT\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);      
            }      
            k19=(int) *(param->data);
            
            param=GetParam( params, K20_SAS_SELECT, 
                            tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, "Error extracting K20_SAS_SELECT\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);      
            }      
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
            if (!param) {
                fprintf(stderr, "Error extracting K21_SES_SUPP_HTR_PWR\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
            k21=(int) *(param->data);
            
            param=GetParam(params,K22_SES_SUPP_HTR_PWR, 
                           tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, "Error extracting K22_SES_SUPP_HTR_PWR\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
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

            //SAS A Spin Rate
            static int last_sas_a_spin_rate;
            int sas_a_spin_rate;
            
            param=GetParam(params,SAS_A_SPIN_RATE, 
                           tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, "Error extracting SAS_A_SPIN_RATE\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
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
            if (!param) {
                fprintf(stderr, "Error extracting SAS_B_SPIN_RATE\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
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
                


            // TRS Cmd Success
            param=GetParam( params, SES_TRS_CMD_SUCC,
                            tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, "Error extracting SES_TRS_CMD_SUCC\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
            static int trs = (int) *(param->data);
            if (!first_time &&
                trs==0) 
                AddEffect( effect_time, EFF_TRS_CHANGE );
        
            
            // Grid Normal/Disable
            static int last_grid;
            int grid;
            param=GetParam( params, GRID_INHIBIT,
                            tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, "Error extracting GRID_INHIBIT\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
            grid = (int) *(param->data);
            if (!first_time &&
                grid != last_grid) {
                if (grid) 
                    AddEffect( effect_time, EFF_GRID_DISABLED );
                else
                    AddEffect( effect_time, EFF_GRID_NORMAL );
            }            
            last_grid=grid;

            last_cmd_counter = cmd_counter;
            last_inv_cmd_counter = inv_cmd_counter;
            first_time = 0;

        
            
            // TWT Body Overcurrent Trip on/off
            static int last_twt_trip_ovrd;
            int twt_trip_ovrd;
            param=GetParam( params, TWT_BODY_OC_TRIP,
                            tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, "Error extracting TWT_BODY_OC_TRIP\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
            twt_trip_ovrd = (int) *(param->data);
            if (!first_time &&
                twt_trip_ovrd != last_twt_trip_ovrd) {
                if (twt_trip_ovrd) 
                    AddEffect( effect_time,  EFF_TWT_TRIP_OVERRIDE_ENABLE );
                else
                    AddEffect( effect_time,  EFF_TWT_TRIP_OVERRIDE_DISABLE );
            }            
            last_twt_trip_ovrd=twt_trip_ovrd;

            // Receive_Protect Normal/Disable
            static int last_receive_protect;
            int receive_protect;
            param=GetParam( params, SES_RX_PROTECT,
                            tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, "Error extracting SES_RX_PROTECT\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
            receive_protect = (int) *(param->data);
            if (!first_time &&
                receive_protect != last_receive_protect) {
                if (receive_protect) 
                    AddEffect( effect_time, EFF_RECEIVE_PROTECT_ON );
                else
                    AddEffect( effect_time, EFF_RECEIVE_PROTECT_NORMAL );
            }            
            last_receive_protect=receive_protect;


            last_cmd_counter = cmd_counter;
            last_inv_cmd_counter = inv_cmd_counter;
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








