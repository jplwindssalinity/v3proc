//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    hdf_l1a_echo_proc
//
// SYNOPSIS
//    hdf_l1a_echo_proc <sim_config_file> <output_echo_file>
//
// DESCRIPTION
//    Reads an official HDF L1A file and estimates frequency offsets
//    (from 0 kHz baseband) of the spectral response of the echo by
//    fitting a gaussian to the echo energies.
//
// OPERANDS
//    The following operands are supported:
//      <sim_config_file>   The simulation configuration file.
//      <output_echo_file>  The output echo file name.
//
// EXAMPLES
//    An example of a command line is:
//      % hdf_l1a_echo_proc qscat.cfg qscat.echo
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
//    James N. Huddleston (hudd@casket.jpl.nasa.gov)
//----------------------------------------------------------------------

//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcs_id[] =
    "@(#) $Id$";

//----------//
// INCLUDES //
//----------//

#include <assert.h>
#include "L1AExtract.h"

#include <stdio.h>
#include "Args.h"
#include "ArgDefs.h"
#include "ArgsPlus.h"
#include "ParTab.h"
#include "Filter.h"
#include "Spacecraft.h"
#include "ConfigSim.h"
#include "ConfigList.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<WindVectorPlus>;
template class List<Meas>;
template class List<AngleInterval>;
template class List<OffsetList>;
template class List<long>;
template class List<MeasSpot>;
template class List<EarthPosition>;
template class TrackerBase<unsigned short>;
template class TrackerBase<unsigned char>;

//-----------//
// CONSTANTS //
//-----------//

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

int ExtractAntSpinRateDnPer99(TlmHdfFile* l1File, int32* sdsIDs,
        int32 start, int32 stride, int32 length, VOIDP buffer,
        PolynomialTable* poly_table);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<sim_config_file>", "<output_echo_file>", 0};

ArgInfo tlm_files_arg = TLM_FILES_ARG;
ArgInfo l1a_files_arg = L1A_FILES_ARG;
ArgInfo start_time_arg = START_TIME_ARG;
ArgInfo end_time_arg = END_TIME_ARG;
ArgInfo filter_arg = FILTER_ARG;
ArgInfo output_file_arg = OUTPUT_FILE_ARG;
ArgInfo poly_table_arg = POLY_TABLE_ARG;
ArgInfo no_gr_header_arg = NO_GR_HEADER_ARG;

ArgInfo ins_config_arg = { "INSTRUMENT_CONFIG_FILE", "-ins",
    "ins_config_file" };

ArgInfo* arg_info_array[] =
{
    &tlm_files_arg,
    &start_time_arg,
    &end_time_arg,
    &filter_arg,
    &output_file_arg,
    &poly_table_arg,
    &no_gr_header_arg,
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
    char* start_time_string = args_plus.Get(start_time_arg);
    char* end_time_string = args_plus.Get(end_time_arg);
    char* filter_string = args_plus.Get(filter_arg);
//    char* output_file_string = args_plus.Get(output_file_arg);
    char* poly_table_string = args_plus.Get(poly_table_arg);
    char* ins_config_string = args_plus.Get(ins_config_arg);

    //-------------------//
    // convert arguments //
    //-------------------//

    SourceIdE tlm_type = SOURCE_L1A;
    Itime start_time = args_plus.GetStartTime(start_time_string);
    Itime end_time = args_plus.GetEndTime(end_time_string);
    char* tlm_files = args_plus.GetTlmFilesOrExit(tlm_files_string, tlm_type,
        NULL, l1a_files_string, NULL, NULL, NULL);

    //---------------//
    // use arguments //
    //---------------//

    TlmFileList* tlm_file_list = args_plus.TlmFileListOrExit(tlm_type,
        tlm_files, start_time, end_time);
//    FILE* output_fp = args_plus.OutputFileOrStdout(output_file_string);
    FilterSet* filter_set = args_plus.FilterSetOrNull(tlm_type, filter_string);
    PolynomialTable* polyTable =
        args_plus.PolynomialTableOrNull(poly_table_string);

    char* no_gr_header_string = args_plus.Get(no_gr_header_arg);
    int no_gr_header = 0;
    if (no_gr_header_string && strcmp(no_gr_header_string, "0") != 0)
    {
        no_gr_header = 1;
    }

    //---------------------//
    // read in config file //
    //---------------------//

/*
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
*/

    //---------------------------------//
    // create and configure spacecraft //
    //---------------------------------//

    Spacecraft spacecraft;
    spacecraft.attitude.SetOrder(2, 1, 3);

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
        ANT_SPIN_RATE, UNIT_DN);

    //---------------------------//
    // process data file by file //
    //---------------------------//

//    int dataRecCount = 0;
    for (TlmHdfFile* tlmFile = tlm_file_list->GetHead(); tlmFile;
        tlmFile = tlm_file_list->GetNext())
    {
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

        //------------------------//
        // process frame by frame //
        //------------------------//

        for (int i = 0; i < tlmFile->GetDataLength(); i++)
        {
            //--------------------//
            // set the spacecraft //
            //--------------------//

            float xpos, ypos, zpos;
            xpos_p->extractFunc(tlmFile, xpos_p->sdsIDs, i, 1, 1, &xpos,
                polyTable);
            ypos_p->extractFunc(tlmFile, ypos_p->sdsIDs, i, 1, 1, &ypos,
                polyTable);
            zpos_p->extractFunc(tlmFile, zpos_p->sdsIDs, i, 1, 1, &zpos,
                polyTable);

            float xvel, yvel, zvel;
            xvel_p->extractFunc(tlmFile, xvel_p->sdsIDs, i, 1, 1, &xvel,
                polyTable);
            yvel_p->extractFunc(tlmFile, yvel_p->sdsIDs, i, 1, 1, &yvel,
                polyTable);
            zvel_p->extractFunc(tlmFile, zvel_p->sdsIDs, i, 1, 1, &zvel,
                polyTable);

            float roll, pitch, yaw;
            roll_p->extractFunc(tlmFile, roll_p->sdsIDs, i, 1, 1, &roll,
                polyTable);
            pitch_p->extractFunc(tlmFile, pitch_p->sdsIDs, i, 1, 1, &pitch,
                polyTable);
            yaw_p->extractFunc(tlmFile, yaw_p->sdsIDs, i, 1, 1, &yaw,
                polyTable);

            spacecraft.orbitState.rsat.Set(xpos, ypos, zpos);
            spacecraft.orbitState.vsat.Set(xvel, yvel, zvel);
            spacecraft.attitude.SetRPY(roll, pitch, yaw);

            //------------------------//
            // estimate the spin rate //
            //------------------------//

            unsigned short spin_rate;
            ExtractAntSpinRateDnPer99(tlmFile, ant_pos_p->sdsIDs, i, 1, 1,
                &spin_rate, polyTable);
printf("%d\n", (int)spin_rate);

/*
            unsigned int theta_max =
                *(frame->antennaPosition + frame->spotsPerFrame - 1);
        unsigned int theta_min =
            *(frame->antennaPosition + 0);
        while (theta_max < theta_min)
        {
            theta_max += ENCODER_N;
        }
        unsigned int delta_theta = theta_max - theta_min;
        double delta_theta_rad = two_pi * (double)delta_theta /
            (double)ENCODER_N;
        double delta_t = (double)(frame->spotsPerFrame - 1) * qscat.ses.pri;
        double omega = delta_theta_rad / delta_t;
        qscat.sas.antenna.spinRate = omega;
*/
        }

        //----------------//
        // close datasets //
        //----------------//

        tlmFile->CloseParamDatasets(xpos_p);
    }

    //----------------//
    // delete objects //
    //----------------//

    delete tlm_file_list;
    if (filter_set)
        delete filter_set;

    return (0);
}

//----------------------------------------------------------------------
// Function: ExtractAntSpinRateDnPer99
// Extracts: UINT2
//----------------------------------------------------------------------

int
ExtractAntSpinRateDnPer99(
    TlmHdfFile*       l1File,
    int32*            sdsIDs,
    int32             start,
    int32             stride,
    int32             length,
    VOIDP             buffer,
    PolynomialTable*  poly_table)    // unused
{
    // extract one at a time
    if (length != 1)
    {
        return(-1);
    }

    assert(l1File != 0);

    // find out the raw antenna position
    unsigned short antPos[100];;
    int32 antposSdsIds[1];
    antposSdsIds[0] = sdsIDs[0];
    int rc = ExtractData2D_100(l1File, antposSdsIds, start, 0, 1, antPos);
    if (rc <= 0)
    {
        return(rc);
    }

    // spin rate is the delta antenna position
    unsigned int theta_max = antPos[99];
    unsigned int theta_min = antPos[0];
    if (theta_max < theta_min)
    {
        theta_max += ENCODER_N;
    }
    unsigned int delta_theta = theta_max - theta_min;

    unsigned short* ptr = (unsigned short *)buffer;
    *ptr = (unsigned short)delta_theta;

    return(1);
}
