//=========================================================//
// Copyright  (C)1996, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//
// CM Log
//
// $Log$
// 
//    Rev 1.14   25 Mar 1999 14:11:42   daffer
// Uploadable tables!
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
#include "UpldTbl.h"
#include "UpldTblList.h"

#define MAX_NUM_TABLE_CMD_ENTRIES 5


// struct TableIds
// {
//     EAUpldTblE activeId;
//     EAUpldTblE inactiveId;
// };

// struct UpldTblMapEntry
// {
//     char      Type[20];
//     int       NumEntries;
//     TableIds tblids[MAX_NUM_TABLE_CMD_ENTRIES];
// };

// const UpldTblMapEntry L1AEffDetectUpldTblMap[] = 
// {
//     {"PRF",     4,
//      { 
//          {EA_TBL_INACT_PRFEXTSTANDBY, EA_TBL_ACT_PRFEXTSTANDBY},
//          {EA_TBL_INACT_PRFEXTWOM,     EA_TBL_ACT_PRFEXTWOM},
//          {EA_TBL_INACT_PRFEXTCAL,     EA_TBL_ACT_PRFEXTCAL},
//          {EA_TBL_INACT_PRFEXTRCV,     EA_TBL_ACT_PRFEXTRCV}
//      } 
//     },
//     {"SES",       5, 
//      { 
//          {EA_TBL_INACT_SESALL,        EA_TBL_ACT_SESALL},
//          {EA_TBL_INACT_SESSTANDBY,    EA_TBL_ACT_SESSTANDBY},
//          {EA_TBL_INACT_SESWOM,        EA_TBL_ACT_SESWOM},
//          {EA_TBL_INACT_SESCAL,        EA_TBL_ACT_SESCAL},
//          {EA_TBL_INACT_SESRCV,        EA_TBL_ACT_SESRCV}
//      }
//     },
//     {"SERDIGENG",  1,
//      { {EA_TBL_INACT_SERDIGENGTLM,  EA_TBL_ACT_SERDIGENGTLM} }
//     },
//     {"SERDIGSTATUS",  1,
//      { { EA_TBL_INACT_SERDIGSTATLM,  EA_TBL_ACT_SERDIGSTATLM}}
//     },
//     {"DOPPLER",       2,
//      {
//          {EA_TBL_INACT_DOPPLER_A,     EA_TBL_ACT_DOPPLER_A},
//          {EA_TBL_INACT_DOPPLER_B,    EA_TBL_ACT_DOPPLER_B}
//      }
//     },
//     {"RANGEGATE", 2,
//      {
//          {EA_TBL_INACT_RANGE_A,      EA_TBL_ACT_RANGE_A},
//          {EA_TBL_INACT_RANGE_B,      EA_TBL_ACT_RANGE_B}
//      }
//     }
// };

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
    { TABLE_READOUT_TYPE, UNIT_DN},
    { TABLE_READOUT_OFFSET, UNIT_DN},
    { TABLE_READOUT_DATA, UNIT_DN},
    { PARAM_UNKNOWN, UNIT_UNKNOWN }       //  end

}; // Eff_table

//=======================//
// L1AEffDetector methods //
//=======================//

L1AEffDetector::L1AEffDetector(const char *qpa_directory, 
                               const char *qpf_directory, 
                               const char *uqpx_directory,
                               Itime start_time, Itime end_time )
{
    numTableElements=0;
    while (Eff_Table[numTableElements].paramId != PARAM_UNKNOWN)
        numTableElements++;

    _qpx_list = new UpldTblList (qpa_directory, qpf_directory, 
                                 start_time, end_time);
    _qpa_directory  = strdup(qpa_directory);
    _qpf_directory  = strdup(qpf_directory);
    _uqpx_directory = strdup(uqpx_directory);

    if (_qpx_list->GetStatus() != UpldTblList::UPLDTBLLIST_OK || 
        _qpf_directory == NULL || 
        _qpa_directory == NULL ||
        _uqpx_directory == NULL)
        _status=EFFDETECTOR_ERROR;

    return;
}

L1AEffDetector::~L1AEffDetector()
{
    if (_qpx_list != NULL )
        delete _qpx_list;

    return;
}

//-----------//
// Create Effect Entry
//-----------//

Command *
L1AEffDetector::CreateEffectEntry(
    const Itime     effect_time,
    const EffectE   effect_id)
{
    Command *cmd = new Command();
    if (cmd) {
        cmd->l1aTime = effect_time;
        cmd->effectId = effect_id;
        cmd->l1aVerify = VER_YES;
    }
    return(cmd);
}



//-----------//
// AddEffect //
//-----------//
int
L1AEffDetector::AddEffect(
                          Command *cmd)
{
    _list->Append(cmd);
    return(1);
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
    UpldTbl *tbl=new UpldTbl(); // table currently being douwnloaded
                                // as distinguished from the list of 
                                // tables in the QPF/QPA directories.

    UpldTblList *tbllist = new UpldTblList(); // list of downloaded tables

    tbl->SetDirectorySearchList( GetQpaDirectory(), GetQpfDirectory() );
    tbl->SetUqpxDir(GetUqpxDirectory());

    Command *effect = 0;

    // int num_tbl_entries, nn, *filled; 
    UpldTbl *junktbl=0, *match=0, *qpxmatch=0;

    int tableswitch=0; // 1 if command is a table switch
    int tableupload=0; // 1 if command is a table upload

    int ignore_current_table=0; // If the current table is being
                                     // uploaded, ignore it.
    EAUpldTbl_TypeE table_upload_type; // Table type of uploaded table 
                                       // mentioned in cmd_history for 
                                       // current frame
    EAUpldTbl_TypeE table_switch_type; // Table type of switched table 
                                       // mentioned in cmd_history for 
                                       // current frame

    EffectE effectid;
    Itime effect_time;

    //    CmdList *switchCmdList = new CmdList();
    //    CmdList *uploadCmdList = new CmdList();

    int nchars;
    Parameter** params = new Parameter* [numTableElements+1];
    params[numTableElements]=0;
    Parameter* param=0;

    static int cmd_history_change = 0;

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
            tableswitch=0;
            tableupload=0;

            // Get the time for this record and convert it to Itime.
            param=GetParam( params, UTC_TIME, 
                            tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, "Error extracting UTC_TIME\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
            effect_time.Char6ToItime( param->data );
            
            
            // Instrument Time 
            param=GetParam( params, INSTRUMENT_TIME, 
                            tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, "Error extracting INSTRUMENT_TIME\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
            
            if (( (int) param->data )== 0 )
                AddEffect(effect_time, EFF_ELECTRONICS_ON);
            
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
            
            
            // Command History (do this before setting
            // last_cmd_counter, in case of repeated commands)
            
            static unsigned short int *cmd_history;
            static unsigned short int last_cmd_history[4];
            param=GetParam( params, CMD_HISTORY, 
                            tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, "Error extracting CMD_HISTORY\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
            cmd_history = (unsigned short int *) param->data;

            cmd_history_change=0;
            for (i=0;i<4;i++)
                if (cmd_history[i] != last_cmd_history[i]) {
                    cmd_history_change=1;
                    break;
                }

            if (!first_time) {
                if (last_cmd_history[0] != cmd_history[0]) {
                    AddEffect( effect_time, EFF_COMMAND_HISTORY_CHANGE, 
                               cmd_history[0] );
                }else {
                    if (last_cmd_counter != cmd_counter) 
                        AddEffect( effect_time, EFF_COMMAND_HISTORY_REPEAT);
                }
            }


            
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



            // Nomenclature for Table upload algorithm.  'Current
            // table' or 'Current' means the table that is in the
            // process of being read out of memory in the current
            // frame. 
            //
            // When the table id changes, we stop and close out the
            // processing on what had been, until that point, the
            // 'current' table, and we continue calling it such until
            // finished. Then the new table type becomes the current
            // table.

            // As far as the instrument telemetry is concerned, this
            // table is identified by it's ID, which tells both what
            // type the table is and whether it is active or
            // inactive. I prefer to think of a table as a ordered
            // pair, where the the first number is the table type,
            // (e.g. Wind Obs SES table) and the second is it's
            // activity state. In this usage 'current table' means
            // 'current table type' which, were it not for table upload
            // and switch commands, would be exactly equivalent to the
            // 'table ID' that occurs in telemetery.
            //
            // The 'sibling' of a table is the same table type, but
            // opposite activity state.

            // Finally, the CDS Constants table, (table=15) is not
            // verifiable in the same way as all the other tables, so
            // we're just going to skip it for now.


            table_upload_type=EA_TBLTYPE_UNKNOWN;
            if (cmd_history_change) 
                table_upload_type=tbl->TableUpload(cmd_history[0]);

            effectid = tbl->TableTypeToEffectId( table_upload_type,
                                                 cmd_history[0] );
            tableupload=(table_upload_type != EA_TBLTYPE_UNKNOWN);

            if (tableupload) {

                effect=CreateEffectEntry( effect_time, 
                                          effectid );
                if (effect == NULL) {
                    fprintf(stderr,"Error creating effect entry\n");
                    return(_status=EFFDETECTOR_ERROR);
                }
                
                // The current frame has an upload in it.  If the
                // table currently being read out of memory is of the
                // same type as the upload, and it's the 'inactive'
                // table, then it's the one being changed by the
                // upload. Ergo, we can no longer trust it. Set it to
                // 'undefined' and set the ignore flag, so that we
                // won't do anything when this table finishes reading
                // out.

                // Whether or not the current table is begin uploaded,
                // find the QPA/F file that matches the table being
                // uploaded and create a effect entry for it.

                if ( 
                    (table_upload_type  == tbl->GetTableType()) &&
                    (tbl->GetActivity() == ACT_INACTIVE) ) {
                    // Current table.
                    tbl->_defined = 0;
                    ignore_current_table=1;

                    // Check to see if this table is in the table
                    // readback list.  If it is and there is a partial
                    // match between the current table the one in the
                    // list, merge them and use the merged table to
                    // find the QPA/F.

                    // If the current doesn't agree with the one in
                    // the readback list, or there is none in the
                    // readback list, use the current table.

                    for (UpldTbl *tbl2=tbllist->GetHead(); 
                         tbl2;
                         tbl2=tbllist->GetNext()) {
                        if (tbl2->GetTableType() == tbl->GetTableType()) {
                            if (tbl->CompareTable(tbl2) == 
                                EA_UPLDTBL_TABLE_PARTIAL_MATCH) 
                                tbl->MergeTable(tbl2);
                            break;
                        }
                    }
                    
                    EAUpldTbl_Table_StatusE tmpstatus = 
                        FindNearestQpx( tbl, qpxmatch );
                    if (qpxmatch != NULL) {
                        (void) tbl->SetFilename(qpxmatch->GetFilename());
                        (void) tbl->SetDirectory(qpxmatch->GetDirectory());
                        char table_string[MAX_FILENAME_LEN+1];
                        (void) snprintf( table_string, 
                                 MAX_FILENAME_LEN, "%s/%s",
                                 tbl->GetDirectory(),
                                 tbl->GetFilename() );
                        effect->qpx_filename = strdup(table_string);
                        if (tmpstatus==EA_UPLDTBL_TABLE_MATCH ||
                            tmpstatus==EA_UPLDTBL_TABLE_PARTIAL_MATCH )
                            tbl->SetStatus(EA_UPLDTBL_OK);
                        else
                            tbl->SetStatus(EA_UPLDTBL_ERROR);
                    }
                    effect->table_status=tmpstatus;
                    effect->tableid=tbl->GetTableId();
                    effect->table_type=table_upload_type;
                    effect->tbl=tbl;
                    AddEffect(effect);

                } else {

                    // Not the current table.
                    
                    // Search through the upldlist and find this
                    // table. Use it to search through qpx_list
                    
                    for (junktbl=tbllist->GetHead(); junktbl;
                         junktbl=tbllist->GetNext() ) {
                        if (junktbl->GetTableType() == table_upload_type &&
                            junktbl->GetActivity() == ACT_INACTIVE) {
                            match=tbllist->RemoveCurrent();
                            break;
                        }
                    }
                    if (match != NULL) {
                        // So, this table has been readback! 
                        // Find this table in the qpxlist
                        EAUpldTbl_Table_StatusE status2 = 
                            FindNearestQpx( match, qpxmatch );
                        if (qpxmatch != NULL) {
                            (void) match->SetFilename(qpxmatch->GetFilename());
                            (void) match->SetDirectory(qpxmatch->GetDirectory());
                            char table_string[MAX_FILENAME_LEN+1];
                            (void) snprintf( table_string, 
                                     MAX_FILENAME_LEN, "%s/%s",
                                     qpxmatch->GetDirectory(),
                                     qpxmatch->GetFilename() );
                            effect->qpx_filename = strdup(table_string);
                            if (status2==EA_UPLDTBL_TABLE_MATCH ||
                                status2==EA_UPLDTBL_TABLE_PARTIAL_MATCH ) {
                                match->SetStatus(EA_UPLDTBL_OK);
                                effect->tableid=match->GetTableId();
                                effect->table_type=match->GetTableType();
                            } else 
                                match->SetStatus(EA_UPLDTBL_ERROR);
                        }
                        effect->table_status=status2;
                        effect->tbl=match;
                        AddEffect(effect);
                    } else {
                        // We haven't seen this table yet. So we can't
                        // write out which qpx it matches.
                        effect->effectId=EFF_UNKNOWABLE_QPX;
                        effect->table_type=table_upload_type;
                        effect->tbl=0;
                        AddEffect(effect);
                    }
                } // Test on whether uploaded table == current table.
            } //come from if (tableupload)

       
            // Check for table switch commands
            table_switch_type=EA_TBLTYPE_UNKNOWN;
            if (cmd_history_change)
                table_switch_type=tbl->TableSwitch(cmd_history[0]);
            tableswitch = (table_switch_type != EA_TBLTYPE_UNKNOWN);

            if (tableswitch) {
                effectid= 
                    tbl->TableTypeToEffectId(
                               table_switch_type, 
                               cmd_history[0] );
                effect=CreateEffectEntry( effect_time, 
                                          effectid );
                effect->table_type=table_switch_type;
                AddEffect(effect);

                if (table_switch_type == tbl->GetTableType()) {
                    // Current table is being switched.  So, what was,
                    // until the frame before this one, the 'inactive'
                    // table for this type, will now be the active
                    // table; and vice versa for the active table. Toggle
                    // the activity state of the 'current' table
                  tbl->ToggleActivity();
                  // Find the sibling in the list, if any, and toggle
                  // it's activity.
                  UpldTbl *tmptbl1= 
                      tbllist->FindMatchingTable( tbl );
                  if (tmptbl1 != NULL )
                      tmptbl1->ToggleActivity();
                } else {
                    // The table being switched is not the current table.
                    // Go through the table list and switch activity state
                    // for both tables of this type.  
                    for (UpldTbl *tmptbl=tbllist->GetHead(); tmptbl;
                         tmptbl=tbllist->GetNext() ) {
                        if ( tmptbl->GetTableType()==table_switch_type)
                            tmptbl->ToggleActivity();
                    }
                }

            }
            
            
            
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

            // Wait to set last_mode until after we check for changes 
            // in the uploaded tables

     
            //=======================
            // Uploaded Tables
            //=======================


            // Readout Type
            EAUpldTbl_TypeE table_type;
            EAUpldTblE tableid;
            param=GetParam( params, TABLE_READOUT_TYPE,
                            tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, "Error extracting TABLE_READOUT_TYPE\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
            tableid = (EAUpldTblE) *(param->data);
            table_type = (EAUpldTbl_TypeE) (((int) tableid) % 16);
            ActivityStateE activity=(ActivityStateE) ( ((int) tableid) / 16);
            if (first_time) {
                if (table_type != EA_TBLTYPE_CDS) {
                    if (tbl->SetTableType( table_type ) != EA_UPLDTBL_OK) {
                        fprintf(stderr,"L1AEffDetector: Error setting table type\n");
                        return (_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
                    }
                    ignore_current_table=0;
                    tbl->_defined=1;
                    tbl->_activity_state=activity;
                    (void) tbl->SetTableId(tableid);
                    tbl->_first_seen=effect_time;
                } else
                    ignore_current_table=1;

            } else {
                if (table_type != tbl->GetTableType()) {
                    if (tbl->GetTableType() != EA_TBLTYPE_CDS) {
                        // New Table! 
                        // Check to see the table type/activity pair
                        // we've just finished reading out from memory is
                        // in our list of tables seen so far.
                        UpldTbl *matching_tbl = 
                            tbllist->FindMatchingTable(tbl); 
                        if (matching_tbl == NULL ) {
                            // No, it isn't there yet. 
                            // But should we put this one in?
                            if (!ignore_current_table) {
                                //        if (tbl->_defined) {
                                // Reallocate the table 
                                // (in case it's a variable table)
                                if (tbl->ReallocTable() != EA_UPLDTBL_OK ) {
                                    fprintf(stderr, 
                                            "Error reallocating table\n");
                                    return (_status=
                                            EFFDETECTOR_ERROR_EXTRACT_DATA);
                                }
                                // append the table to our list.
                                tbl->_last_seen = effect_time;
                                tbllist->Append( tbl );
                                // } 
                            } else
                                ignore_current_table=0;
                            
                        } else {
                            // The current table is already in the list.
                            if (!ignore_current_table) {
                                // replace the table in 'tbllist'
                                // with the current table
                                Itime first_seen = matching_tbl->_first_seen;
                                matching_tbl=tbllist->RemoveCurrent();
                                tbl->_first_seen=first_seen;
                                tbllist->Append(tbl);
                                //                                } 
                                
                            }
                            
                            ignore_current_table=0;
                            tbl->_defined=1;
                        }
                        tbl=new UpldTbl(table_type, activity);
                        (void) tbl->SetDirectorySearchList( 
                                                GetQpaDirectory(),
                                                GetQpfDirectory());
                        (void) tbl->SetTableId( (EAUpldTblE) tableid);
                        tbl->_first_seen=effect_time;
                        if (tbl->GetStatus() !=  EA_UPLDTBL_OK) {
                            fprintf(stderr,"Error setting table id\n");
                            return (_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
                        }
                        if (table_type == EA_TBLTYPE_CDS)
                            ignore_current_table=1; 
                    } else { // come from if (tbl->GetTableType()!= CDS table)
                        ignore_current_table = 0;
                        tbl=new UpldTbl(table_type, activity);
                        (void) tbl->SetDirectorySearchList( 
                                                GetQpaDirectory(),
                                                GetQpfDirectory());
                        (void) tbl->SetTableId( (EAUpldTblE) tableid);
                        tbl->_first_seen=effect_time;
                        if (tbl->GetStatus() !=  EA_UPLDTBL_OK) {
                            fprintf(stderr,"Error setting table id\n");
                            return (_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
                        }
                    }
                    

                    
                }
            } // else of if (firsttime )
            
            if (!ignore_current_table) {
                 
                // Readout Offset
                static unsigned int table_offset;
                param=GetParam( params, TABLE_READOUT_OFFSET,
                                tlmFile, nextIndex);
                if (!param) {
                    fprintf(stderr,
                            "Error extracting TABLE_READOUT_OFFSET\n");
                    return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
                }
                table_offset=*( (unsigned short int *) param->data);
                
                // Readout Data 
                static unsigned short int table_data[2];
                param=GetParam( params, TABLE_READOUT_DATA,
                                tlmFile, nextIndex);
                if (!param) {
                    fprintf(stderr,
                         "Error extracting TABLE_READOUT_DATA\n");
                    return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
                }
                table_data[0] =  *( (unsigned short int *) param->data);
                table_data[1] = *(  (unsigned short int *) param->data+1);
                
                // The offset is 'ram location' which is in bytes. We
                // maintain the table as a table of short ints. Divide the
                // offset by 2.
                
                table_offset /= 2;
                tbl->AddToTable( table_offset, 2, table_data );
            }            

            tbl->_last_seen=effect_time;


            // -------------------------------------
            // Table Switch Flags.
            // -------------------------------------

            // SES and Mission Tlm tables change with mode. Don't
            // consider these legitimate table switches, but report
            // the changes anyway.
            
            // All other tables only change as a result of a table
            // switch command

            static int real_switch ;
            real_switch=0;


            // SES params table switch
            static int last_ses_params_switch;
            int ses_params_switch;
            param=GetParam(params, STATUS_TABLE_CHANGE_FLAGS_11, 
                           tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, 
                  "Error extracting STATUS_TABLE_CHANGE_FLAGS_11\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
            ses_params_switch=(int) *(param->data);
            if (!first_time &&
                last_ses_params_switch != ses_params_switch )
                if (ses_params_switch) {
                    effect=CreateEffectEntry( effect_time, 
                                              EFF_SES_PARAMS_TABLE_CHANGE );
                    if (!effect) {
                        fprintf( stderr,
                           "Error creating Effect Entry (SES_PARAMS_TABLE)\n");
                        return(_status=EFFDETECTOR_ERROR);
                    }

                    // Find out which SES table switched. Set the
                    // table_type of the resulting 'effect'
                    
                    table_type=tbl->FindTableType( 
                                (unsigned short *) cmd_history );
                    if (table_type==EA_TBLTYPE_UNKNOWN)
                        fprintf(stderr,
                           "Unknown SES Param Table Change!\n");
                        //return (_status=EFFDETECTOR_UNKNOWN_TABLE);
                     effect->table_type=table_type;
                     AddEffect(effect);

                    
                }

            last_ses_params_switch=ses_params_switch;


            // Range gate table switch
            static int last_range_gate_table_switch;
            int range_gate_table_switch;
            param=GetParam(params,STATUS_TABLE_CHANGE_FLAGS_12, 
                           tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, 
                        "Error extracting STATUS_TABLE_CHANGE_FLAGS_12\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
            range_gate_table_switch=(int) *(param->data);
            if (!first_time &&
                last_range_gate_table_switch != range_gate_table_switch ) 
                if (range_gate_table_switch) {
                    effect=CreateEffectEntry( effect_time, 
                                              EFF_RANGEGATE_TABLE_CHANGE );
                    if (!effect) {
                        fprintf(stderr,
                           "Error creating Effect Entry (Rangegate_table_change)\n");
                        return(_status=EFFDETECTOR_ERROR);
                    }
                    // Find out which Range gate table switched. Set the
                    // tableid of the 'effect' to the active table id.
                    table_type=tbl->FindTableType( cmd_history);
                    if (table_type==EA_TBLTYPE_UNKNOWN) {
                        fprintf(stderr,"Unknown RangeGate Table Change!\n");
                        //return (_status=EFFDETECTOR_UNKNOWN_TABLE);
                    }
                    effect->table_type=table_type;
                    AddEffect(effect);
                    
                }
            last_range_gate_table_switch=range_gate_table_switch;
            
            
            
            // Doppler table switch
            static int last_doppler_table_switch;
            int doppler_table_switch;
            param=GetParam(params,STATUS_TABLE_CHANGE_FLAGS_13, 
                           tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, 
                        "Error extracting STATUS_TABLE_CHANGE_FLAGS_13\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
            doppler_table_switch=(int) *(param->data);
            if (!first_time &&
                last_doppler_table_switch != doppler_table_switch )
                if (doppler_table_switch) {
                    effect=CreateEffectEntry( effect_time, 
                                              EFF_DOPPLER_TABLE_CHANGE );
                    // Find out which Doppler table switched. Set the
                    // tableid of the 'effect' to the active table id.
                    table_type=tbl->FindTableType( cmd_history);
                    if (table_type==EA_TBLTYPE_UNKNOWN) {
                        fprintf(stderr,
                           "Unknown Doppler Table Change!\n");
                        //return (_status=EFFDETECTOR_UNKNOWN_TABLE);
                    }
                    effect->table_type=table_type;
                    AddEffect(effect);
                }
            last_doppler_table_switch=doppler_table_switch;



            // Mission TLM table switch (PRF table)
            static int last_prf_table_switch;
            int prf_table_switch;
            param=GetParam(params,STATUS_TABLE_CHANGE_FLAGS_14, 
                           tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, 
                        "Error extracting STATUS_TABLE_CHANGE_FLAGS_14\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
            prf_table_switch=(int) *(param->data);
            if (!first_time &&
                last_prf_table_switch != prf_table_switch )
                if (prf_table_switch) {
                    effect = CreateEffectEntry( effect_time, 
                                                EFF_PRF_TABLE_CHANGE );
                    // Find out which PRF  table switched. Set the
                    // tableid of the 'effect' to the active table id.
                    table_type=tbl->FindTableType( cmd_history);
                    if (table_type==EA_TBLTYPE_UNKNOWN) {
                        fprintf(stderr,
                          "Unknown PRF Table Change!\n");
                    }
                        //return (_status=EFFDETECTOR_UNKNOWN_TABLE);
                    effect->table_type = table_type;
                    AddEffect(effect);
                }
            last_prf_table_switch=prf_table_switch;


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
            
            // TWTA Trip overide enable/disable
            static int last_twta_trip_ovrd;
            int twta_trip_ovrd;
            param=GetParam( params, FRAME_INST_STATUS_15, 
                            tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, "Error extracting FRAME_INST_STATUS_15\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
            
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
            if (!param) {
                fprintf(stderr, 
                        "Error extracting STATUS_TABLE_CHANGE_FLAGS_10\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
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
            if (!param) {
                fprintf(stderr, 
                        "Error extracting STATUS_TABLE_CHANGE_FLAGS_05\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
            ses_multiseq_dle_response = (int) *(param->data);
            if (!first_time&& 
                last_ses_multiseq_dle_response != 
                ses_multiseq_dle_response ) {
                if (ses_multiseq_dle_response) 
                    AddEffect(effect_time,
                              EFF_SES_MULTISEQ_DLE_RESPONSE_CHANGE);
            }
            last_ses_multiseq_dle_response=ses_multiseq_dle_response;
            
            
            // SAS Multi Sequence Data Loss Response change
            static int last_sas_multiseq_dle_response;
            int sas_multiseq_dle_response;
            param=GetParam( params, STATUS_TABLE_CHANGE_FLAGS_06, 
                            tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, 
                        "Error extracting STATUS_TABLE_CHANGE_FLAGS_06\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
            sas_multiseq_dle_response = (int) *(param->data);
            if (!first_time&& 
                last_sas_multiseq_dle_response != 
                sas_multiseq_dle_response ) {
                if (sas_multiseq_dle_response) 
                    AddEffect(effect_time,
                              EFF_SAS_MULTISEQ_DLE_RESPONSE_CHANGE);
            }
            last_sas_multiseq_dle_response=sas_multiseq_dle_response;


            // Ses Supplemental Heater Control change
            static int last_ses_suppl_htr_cntrl;
            int ses_suppl_htr_cntrl;
            param=GetParam( params, STATUS_TABLE_CHANGE_FLAGS_08, 
                            tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, 
                        "Error extracting STATUS_TABLE_CHANGE_FLAGS_08\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
            ses_suppl_htr_cntrl = (int) *(param->data);
            if (!first_time&& 
                last_ses_suppl_htr_cntrl != 
                ses_suppl_htr_cntrl ) {
                if (ses_suppl_htr_cntrl) 
                    AddEffect(effect_time,
                              EFF_SES_SUPPL_HEATER_CNTRL_CHANGE);
            }
            last_ses_suppl_htr_cntrl=ses_suppl_htr_cntrl;


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
                


            // Get gate A Width
            static int last_rangegatewidth_a;
            int rangegatewidth_a;
            param=GetParam( params, RANGE_GATE_A_WIDTH,
                            tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, "Error extracting RANGE_GATE_A_WIDTH\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
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
            if (!param) {
                fprintf(stderr, "Error extracting RANGE_GATE_B_WIDTH\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
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
            if (!param) {
                fprintf(stderr, "Error extracting TRANSMIT_PULSE_WIDTH\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
            transmit_pulse_width = (float) *(param->data);
            if (!first_time &&
                last_transmit_pulse_width != transmit_pulse_width) 
                AddEffect( effect_time, EFF_TRANSMIT_PULSE_WIDTH_CHANGE );
            last_transmit_pulse_width=transmit_pulse_width;



            // TRS Cmd Success
            param=GetParam( params, DISCRETE_STATUS_2_04,
                            tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, "Error extracting DISCRETE_STATUS_2_04\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
            static int trs = (int) *(param->data);
            if (!first_time &&
                trs==0) 
                AddEffect( effect_time, EFF_TRS_CHANGE );
            

            // Get receiver gain
            static int last_receiver_gain;
            int receiver_gain;
            param=GetParam( params, RECEIVER_GAIN,
                            tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, "Error extracting RECEIVER_GAIN\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
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
            if (!param) {
                fprintf(stderr, "Error extracting FRAME_INST_STATUS_13\n");
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

            // Receive_Protect Normal/Disable
            static int last_receive_protect;
            int receive_protect;
            param=GetParam( params, FRAME_INST_STATUS_14,
                            tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, "Error extracting FRAME_INST_STATUS_14\n");
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

            // CDS Soft Reset
            static int last_cds_soft_reset;
            int cds_soft_reset;
            param=GetParam( params, FRAME_INST_STATUS_19,
                            tlmFile, nextIndex);
            if (!param) {
                fprintf(stderr, "Error extracting FRAME_INST_STATUS_19\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
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
            if (!param) {
                fprintf(stderr, "Error extracting DISCRETE_STATUS_3_01\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
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
            if (!param) {
                fprintf(stderr, "Error extracting SES_CONFIG_FLAGS_03\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
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
            if (!param)  {
                fprintf(stderr, 
                        "Error extracting STATUS_TABLE_CHANGE_FLAGS_09\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
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
            if (!param) {
                fprintf(stderr, "Error extracting PRF_CYCLE_TIME\n");
                return(_status=EFFDETECTOR_ERROR_EXTRACT_DATA);
            }
            prf_cycle_time = (int) *(param->data);
            if (!first_time &&
                prf_cycle_time != last_prf_cycle_time) 
              AddEffect( effect_time, EFF_PRF_CLOCK_CHANGE);
            last_prf_cycle_time=prf_cycle_time;
            

              // Set last_cmd_counter, last_inv_cmd_count and
              // last_cmd_history so that the table upload/switch
              // parts can have access to the last 4 commands and
              // command counters.

            for (j=0;j<4;j++) last_cmd_history[j] = cmd_history[j];
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

    (void) tbllist->GetHead();
    while (tbl=tbllist->RemoveCurrent() ){

        effectid = tbl->FinalEffectId();
        effect=CreateEffectEntry( effect_time, effectid );
        EAUpldTbl_Table_StatusE status2 = FindNearestQpx( tbl, qpxmatch );
        effect->table_status=status2;
        effect->table_type=tbl->GetTableType();
        effect->tableid=tbl->GetTableId();
        tbl->SetTableStatus(status2);
        tbl->SetStatus(EA_UPLDTBL_ERROR);
        if (qpxmatch != NULL ) {
            (void) tbl->SetFilename( qpxmatch->GetFilename());
            (void) tbl->SetDirectory( qpxmatch->GetDirectory());
            char time_string[MAX_FILENAME_LEN+1];
            snprintf( time_string, MAX_FILENAME_LEN, "%s/%s",
                      tbl->GetDirectory(), 
                      tbl->GetFilename() );
            effect->qpx_filename = strdup( time_string );
            if (status2 == EA_UPLDTBL_TABLE_MATCH ||
                status2 == EA_UPLDTBL_TABLE_PARTIAL_MATCH)
                tbl->SetStatus(EA_UPLDTBL_OK);
        } else {
            tbl->SetUqpxDir(_uqpx_directory);
            fprintf(stderr,"Can't find QPx file!\n");
        }
        effect->tbl=tbl;
        AddEffect(effect);
        
    }

    //    delete tbl;
    //    delete tbllist;
    //    delete effect;
    
  return(_status = EFFDETECTOR_OK);
}

//========================
// L1AEffDetector::FindNearestQpx
// Find the entry in _qpx_list which is best 
// match for input table.
//========================

EAUpldTbl_Table_StatusE
L1AEffDetector::FindNearestQpx( UpldTbl *intbl, UpldTbl *&best_match) {
    int nentries=0, best_nentries=-1;
    EAUpldTbl_Table_StatusE status2, keep_status;
    status2 = keep_status=EA_UPLDTBL_TABLE_UNKNOWN;

    for (UpldTbl *tbl2=_qpx_list->GetTail(); tbl2;
         tbl2=_qpx_list->GetPrev()) {

        if (tbl2->GetTableType() == intbl->GetTableType()) {
            if (tbl2 <= intbl) {
                status2=
                    intbl->CompareTable(tbl2, &nentries);
                if (status2 == EA_UPLDTBL_TABLE_MATCH) {
                    best_match = tbl2;
                    break;
                }
                if (status2 == EA_UPLDTBL_TABLE_PARTIAL_MATCH && 
                    nentries > best_nentries ){
                    best_match = tbl2;
                    best_nentries=nentries;
                    keep_status=status2;
                }
            }
        }
    }
    if (status2 != EA_UPLDTBL_TABLE_MATCH && 
        keep_status == EA_UPLDTBL_TABLE_PARTIAL_MATCH )
        status2=keep_status;

    return (status2);

} //FindNearestQpx

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







