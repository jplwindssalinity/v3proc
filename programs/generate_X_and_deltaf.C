//==============================================================//
// Copyright (C) 1997-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    generate_X_and_deltaf 
//
// SYNOPSIS
//    
//    generate_X_and_deltaf  <config_file> <X_file> <deltaf_file> 
//
//
// DESCRIPTION
//    Calculates X and deltaf for comparison with official processor
//
// OPTIONS
//    None.
//
// OPERANDS
//    
//  config_file :  simplified simulation configuration file
//  X_file      : output binary file for X values
//  deltaf      : output binary file for deltaf values
//
// EXAMPLES
//    An example of a command line is:
//      % l1a_to_l1b sws1b.cfg
//
// ENVIRONMENT
//    Not environment dependent.
//
// EXIT STATUS
//    The following exit values are returned:
//       0  Program executed successfully
//      >0  Program had an error
//
// NOTES
//    None.
//
// AUTHOR
//    Bryan W. Stiles (bstiles@acid.jpl.nasa.gov)
//----------------------------------------------------------------------

//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcs_id[] =
    "@(#) $Id$";

//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include <stdlib.h>
#include "BufferedList.h"
#include "BufferedList.C"
#include "List.h"
#include "List.C"
#include "Misc.h"
#include "Ephemeris.h"
#include "ConfigList.h"
#include "L1A.h"
#include "ConfigSim.h"
#include "L1B.h"
#include "L1AToL1B.h"
#include "Tracking.h"
#include "Tracking.C"
#include "QscatConfig.h"
#include <assert.h>
#include "L1AExtract.h"
#include <fcntl.h>
#include "Args.h"
#include "ArgDefs.h"
#include "ArgsPlus.h"
#include "ParTab.h"
#include "Filter.h"
#include <assert.h>
#include "L1AExtract.h"

//-----------//
// TEMPLATES //
//-----------//

// Class declarations needed for templates
// eliminates need to include the entire header file
class AngleInterval;

template class List<AngleInterval>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<MeasSpot>;
template class List<WindVectorPlus>;
template class List<long>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;

//-----------//
// CONSTANTS //
//-----------//

//--------//
// MACROS //
//--------//


//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

ArgInfo tlm_files_arg = TLM_FILES_ARG;
ArgInfo l1a_files_arg = L1A_FILES_ARG;
ArgInfo poly_table_arg = POLY_TABLE_ARG;
ArgInfo ins_config_arg = {"INSTRUMENT_CONFIG_FILE", "-ins",
    "ins_config_file" };

ArgInfo* arg_info_array[] =
{
    &tlm_files_arg,
    &l1a_files_arg,
    &poly_table_arg,
    &ins_config_arg,
    0
};


//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int    argc,
    char*  argv[])
{
    //------------------------//
    // parse the command line //
    //------------------------//
    const char* command = no_path(argv[0]);
    char* config_filename = getenv(ENV_CONFIG_FILENAME);
    static ArgsPlus args_plus = ArgsPlus(argc, argv, config_filename,
        arg_info_array);

    if (argc == 1)
    {
        args_plus.Usage();
        exit(1);
    }

    char* tlm_files_string = args_plus.Get(tlm_files_arg);
    char* l1a_files_string = args_plus.Get(l1a_files_arg);
    char* poly_table_string = args_plus.Get(poly_table_arg);
    char* ins_config_string = args_plus.Get(ins_config_arg);

    //-------------------//
    // convert arguments //
    //-------------------//

    SourceIdE tlm_type = SOURCE_L1A;
    char* tlm_files = args_plus.GetTlmFilesOrExit(tlm_files_string, tlm_type,
        NULL, l1a_files_string, NULL, NULL, NULL);

    //---------------//
    // use arguments //
    //---------------//

    TlmFileList* tlm_file_list = args_plus.TlmFileListOrExit(tlm_type,
        tlm_files, INVALID_TIME, INVALID_TIME);
    PolynomialTable* polyTable =
        args_plus.PolynomialTableOrNull(poly_table_string);


 

    //---------------------//
    // read in config file //
    //---------------------//

    if (! ins_config_string)
    {
        fprintf(stderr, "%s: missing instrument configuration file\n",
            command);
        exit(1);
    }
    ConfigList config_list;
    if (! config_list.Read(ins_config_string))
    {
        fprintf(stderr, "%s: error reading configuration file %s\n",
            command, ins_config_string);
        exit(1);
    }

    //---------------------------------//
    // create and configure spacecraft //
    //---------------------------------//

    Spacecraft spacecraft;
    spacecraft.attitude.SetOrder(2, 1, 3);
    Attitude* attitude = &(spacecraft.attitude);

    //----------------------------//
    // create and configure QSCAT //
    //----------------------------//

    Qscat qscat;
    if (! ConfigQscat(&qscat, &config_list))
    {
        fprintf(stderr, "%s: error configuring QSCAT\n", command);
        exit(1);
    }

    //--------------------------------//
    // create and configure XTABLE    //
    //--------------------------------//
    BYUXTable xtable;
    if(!ConfigBYUXTable(&xtable,&config_list)){
        fprintf(stderr, "%s: error configuring QSCAT\n", command);
        exit(1);
    }

    //--------------------------//
    // get necessary parameters //
    //--------------------------//


    Parameter* xpos_p = ParTabAccess::GetParameter(SOURCE_L1A, X_POS,
        UNIT_KILOMETERS);
    Parameter* ypos_p = ParTabAccess::GetParameter(SOURCE_L1A, Y_POS,
        UNIT_KILOMETERS);
    Parameter* zpos_p = ParTabAccess::GetParameter(SOURCE_L1A, Z_POS,
        UNIT_KILOMETERS);

    Parameter* xvel_p = ParTabAccess::GetParameter(SOURCE_L1A, X_VEL,
        UNIT_KMPS);
    Parameter* yvel_p = ParTabAccess::GetParameter(SOURCE_L1A, Y_VEL,
        UNIT_KMPS);
    Parameter* zvel_p = ParTabAccess::GetParameter(SOURCE_L1A, Z_VEL,
        UNIT_KMPS);

    Parameter* roll_p = ParTabAccess::GetParameter(SOURCE_L1A, ROLL,
        UNIT_RADIANS);
    Parameter* pitch_p = ParTabAccess::GetParameter(SOURCE_L1A, PITCH,
        UNIT_RADIANS);
    Parameter* yaw_p = ParTabAccess::GetParameter(SOURCE_L1A, YAW,
        UNIT_RADIANS);

    Parameter* ant_pos_p = ParTabAccess::GetParameter(SOURCE_L1A,
        ANTENNA_POS, UNIT_DN);

    Parameter* orbit_time_p = ParTabAccess::GetParameter(SOURCE_L1A,
        ORBIT_TIME, UNIT_COUNTS);

    Parameter* orbit_step_p = ParTabAccess::GetParameter(SOURCE_L1A,
        DOPPLER_ORBIT_STEP, UNIT_COUNTS);

    Parameter* pri_of_orbit_step_change_p =
        ParTabAccess::GetParameter(SOURCE_L1A, PRF_ORBIT_STEP_CHANGE,
        UNIT_COUNTS);

    Parameter* prf_cycle_time_p = ParTabAccess::GetParameter(SOURCE_L1A,
        PRF_CYCLE_TIME, UNIT_DN);

    //---------------------------//
    // process data file by file //
    //---------------------------//
    for (TlmHdfFile* tlmFile = tlm_file_list->GetHead(); tlmFile;
        tlmFile = tlm_file_list->GetNext())
    {
        //---------------------//
        // create output files //
        //---------------------//
        const char* input_filename = tlmFile->GetFileName();
        const char* tail = no_path(input_filename);
        char X_filename[2048];
        sprintf(X_filename, "%s.X", tail);

        FILE* xfp = fopen(X_filename, "w");
        if (xfp == NULL)
        {
            fprintf(stderr, "%s: error opening output file %s\n", command,
                X_filename);
            exit(1);
        }        

        char df_filename[2048];
        sprintf(df_filename, "%s.df",  tail);
        FILE* dffp = fopen(df_filename, "w");
        if (dffp == NULL)
        {
            fprintf(stderr, "%s: error opening output file %s\n", command,
                df_filename);
            exit(1);
        } 
 
        //-------------------------//
        // open necessary datasets //
        //-------------------------//

        HdfFile::StatusE status;
        status = tlmFile->OpenParamDatasets(xpos_p);
        status = tlmFile->OpenParamDatasets(ypos_p);
        status = tlmFile->OpenParamDatasets(zpos_p);
        status = tlmFile->OpenParamDatasets(xvel_p);
        status = tlmFile->OpenParamDatasets(yvel_p);
        status = tlmFile->OpenParamDatasets(zvel_p);
        status = tlmFile->OpenParamDatasets(roll_p);
        status = tlmFile->OpenParamDatasets(pitch_p);
        status = tlmFile->OpenParamDatasets(yaw_p);
        status = tlmFile->OpenParamDatasets(ant_pos_p);
        status = tlmFile->OpenParamDatasets(orbit_time_p);
        status = tlmFile->OpenParamDatasets(orbit_step_p);
        status = tlmFile->OpenParamDatasets(pri_of_orbit_step_change_p);
        status = tlmFile->OpenParamDatasets(prf_cycle_time_p);

        //------------------------//
        // get record length info //
        //------------------------//

        int num_records=tlmFile->GetDataLength();
        int num_pulses=100;
        int num_slices=8;

        //-------------------------//
        // create output arrays    //
        //-------------------------//

	float*** X=(float***)make_array(sizeof(float),3,num_records,num_pulses,num_slices);

        float** df=(float**)make_array(sizeof(float),2,num_records,num_pulses);

        //-------------------------//
        // Process frame by frame  //
        //-------------------------//

        for (int record_idx = 0; record_idx < num_records;
            record_idx++)
        {    
            if(record_idx%1000==0 && record_idx!=0)
              printf("%s:%d records processed\n",command,record_idx);
            //--------------------//
            // set the spacecraft //
            //--------------------//

            float xpos, ypos, zpos;
            xpos_p->extractFunc(tlmFile, xpos_p->sdsIDs, record_idx, 1, 1,
                &xpos, polyTable);
            ypos_p->extractFunc(tlmFile, ypos_p->sdsIDs, record_idx, 1, 1,
                &ypos, polyTable);
            zpos_p->extractFunc(tlmFile, zpos_p->sdsIDs, record_idx, 1, 1,
                &zpos, polyTable);

            /*** Take care of odd center of the earth case which pops up in
                 L1AP ****/
	    if(xpos==0 && ypos==0 && zpos==0){
	      for(int c=0;c<num_pulses;c++){
                for(int s=0;s<num_slices;s++){
		  X[record_idx][c][s]=100;
		}
                df[record_idx][c]=-500000;
	      }
	      continue;
	    }
            float xvel, yvel, zvel;
            xvel_p->extractFunc(tlmFile, xvel_p->sdsIDs, record_idx, 1, 1,
                &xvel, polyTable);
            yvel_p->extractFunc(tlmFile, yvel_p->sdsIDs, record_idx, 1, 1,
                &yvel, polyTable);
            zvel_p->extractFunc(tlmFile, zvel_p->sdsIDs, record_idx, 1, 1,
                &zvel, polyTable);

            float roll, pitch, yaw;
            roll_p->extractFunc(tlmFile, roll_p->sdsIDs, record_idx, 1, 1,
                &roll, polyTable);
            pitch_p->extractFunc(tlmFile, pitch_p->sdsIDs, record_idx, 1, 1,
                &pitch, polyTable);
            yaw_p->extractFunc(tlmFile, yaw_p->sdsIDs, record_idx, 1, 1, &yaw,
                polyTable);

            spacecraft.orbitState.rsat.Set(xpos, ypos, zpos);
            spacecraft.orbitState.vsat.Set(xvel, yvel, zvel);
            attitude->SetRPY(roll, pitch, yaw);        
   
            //------------------------//
            // command the instrument //
            //------------------------//

            unsigned char pri_dn;
            prf_cycle_time_p->extractFunc(tlmFile, prf_cycle_time_p->sdsIDs,
                record_idx, 1, 1, &pri_dn, polyTable);

            qscat.ses.CmdPriDn(pri_dn);


             //-----------------------//
            // get other information //
            //-----------------------//

            unsigned int orbit_time;
            orbit_time_p->extractFunc(tlmFile, orbit_time_p->sdsIDs,
                record_idx, 1, 1, &orbit_time, polyTable);

            unsigned char base_orbit_step;
            orbit_step_p->extractFunc(tlmFile, orbit_step_p->sdsIDs,
                record_idx, 1, 1, &base_orbit_step, polyTable);

            unsigned char pri_of_orbit_step_change;
            pri_of_orbit_step_change_p->extractFunc(tlmFile,
                pri_of_orbit_step_change_p->sdsIDs, record_idx, 1, 1,
                &pri_of_orbit_step_change, polyTable);

            //--------------------//
            // step through spots //
            //--------------------//

            for (int spot_idx = 0; spot_idx < num_pulses; spot_idx++)
            {
                // determine beam index and beam
                int beam_idx = spot_idx % NUMBER_OF_QSCAT_BEAMS;    
                qscat.cds.currentBeamIdx = beam_idx;

                // determine orbit step
                qscat.cds.orbitTime = orbit_time;
                unsigned char orbit_step = base_orbit_step;
                if (pri_of_orbit_step_change != 255 &&
                    spot_idx < pri_of_orbit_step_change)
                {
                    if (orbit_step == 0)
                        orbit_step = ORBIT_STEPS - 1;
                    else
                        orbit_step--;
                }
                qscat.cds.orbitStep = orbit_step;

                //-------------------------------------//
                // Get Antenna Position                //
                //-------------------------------------//
		unsigned short ant_pos[100];
		ant_pos_p->extractFunc(tlmFile, ant_pos_p->sdsIDs, 
				       record_idx, 1,
				       1, ant_pos, polyTable);  
             
                // set the encoder for tracking
                unsigned short held_encoder = ant_pos[spot_idx];
                qscat.cds.heldEncoder = held_encoder;

                
                //-------------------------------//
                // do range and Doppler tracking //
                //-------------------------------//

                qscat.SetEncoderAzimuth(held_encoder, 1);
                qscat.SetOtherAzimuths(&spacecraft);

                SetDelayAndFrequency(&spacecraft, &qscat);

                //---------------------------------//
                // Compute deltaf                  //
                //---------------------------------//
                float deltaf=xtable.GetDeltaFreq(&spacecraft,&qscat,NULL);
                df[record_idx][spot_idx]=deltaf;

                float orbit_position = qscat.cds.OrbitFraction();
                float azim = qscat.sas.antenna.groundImpactAzimuthAngle;

		
                //----------------------------------//
                // Compute X                        //
                //----------------------------------//
		for(int slice_idx=0;slice_idx<num_slices;slice_idx++){
		  int rel_slice_idx; 
		  abs_to_rel_idx(slice_idx,num_slices,&rel_slice_idx);
		  X[record_idx][spot_idx][slice_idx]=
		    xtable.GetX(beam_idx,azim,orbit_position,rel_slice_idx,
				deltaf);
		}
                		
	    }
        }
	//------------------------------------//
        // write output files                 //
        //------------------------------------//

        //      DELTAF FILE
        if(fwrite((void*)&num_records, sizeof(int),1,dffp)!=1){
	  fprintf(stderr,"failed write to %s\n",df_filename);
	}   
        if(fwrite((void*)&num_pulses, sizeof(int),1,dffp)!=1){
	  fprintf(stderr,"failed write to %s\n",df_filename);
	} 
        for(int record_idx=0;record_idx<num_records;record_idx++){
	  if(fwrite((void*)&df[record_idx][0], sizeof(float),num_pulses,dffp)!=(unsigned)num_pulses){
	    fprintf(stderr,"failed write to %s\n",df_filename);
	  } 
	}
        fclose(dffp);

        //  X FILE
	if(fwrite((void*)&num_records, sizeof(int),1,xfp)!=1){
	  fprintf(stderr,"failed write to %s\n",X_filename);
	}   
        if(fwrite((void*)&num_pulses, sizeof(int),1,xfp)!=1){
	  fprintf(stderr,"failed write to %s\n",X_filename);
	} 
        if(fwrite((void*)&num_slices, sizeof(int),1,xfp)!=1){
	  fprintf(stderr,"failed write to %s\n",X_filename);
	} 

        for(int record_idx=0;record_idx<num_records;record_idx++){
	  for(int spot_idx=0;spot_idx<num_pulses;spot_idx++){
	    if(fwrite((void*)&X[record_idx][spot_idx][0], sizeof(float),num_slices,xfp)!=(unsigned)num_slices){
	      fprintf(stderr,"failed write to %s\n",X_filename);
	    } 
	  }
	}

        fclose(xfp);
                 

        //----------------//
        // close datasets //
        //----------------//

        tlmFile->CloseParamDatasets(xpos_p);
        tlmFile->CloseParamDatasets(ypos_p);
        tlmFile->CloseParamDatasets(zpos_p);
        tlmFile->CloseParamDatasets(xvel_p);
        tlmFile->CloseParamDatasets(yvel_p);
        tlmFile->CloseParamDatasets(zvel_p);
        tlmFile->CloseParamDatasets(roll_p);
        tlmFile->CloseParamDatasets(pitch_p);
        tlmFile->CloseParamDatasets(yaw_p);
        tlmFile->CloseParamDatasets(ant_pos_p);
        tlmFile->CloseParamDatasets(orbit_time_p);
        tlmFile->CloseParamDatasets(orbit_step_p);
        tlmFile->CloseParamDatasets(pri_of_orbit_step_change_p);
        tlmFile->CloseParamDatasets(prf_cycle_time_p);
	free_array((void*)df,2,num_records,num_pulses);
	free_array((void*)X,2,num_records,num_pulses);
 
    }

  delete tlm_file_list;
  return(0);
}

