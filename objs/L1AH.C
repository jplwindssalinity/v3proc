//=========================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_l1ah_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <math.h>
#include "L1AH.h"
#include "ETime.h"
#include "Sds.h"
#include "Qscat.h"
#include "mfhdf.h"

//=========================//
// The table of Attributes //
//=========================//

Attribute* long_name = new Attribute("LongName", "char", "1",
    "SeaWinds Level 1A Engineering Unit Converted Telemetry");
Attribute* short_name = new Attribute("ShortName", "char", "1",
    "SWSL1A");
Attribute* producer_agency = new Attribute("producer_agency", "char", "1",
    "NASA");
Attribute* producer_institution = new Attribute("producer_institution",
    "char", "1", "JPL");
Attribute* instrument_short_name = new Attribute("InstrumentShortName",
    "char", "1", "SeaWinds");
Attribute* platform_long_name = new Attribute("PlatformLongName", "char",
    "1", "Advanced Earth Observing Satellite II");
Attribute* platform_short_name = new Attribute("PlatformShortName", "char",
    "1", "ADEOS-II");
Attribute* platform_type = new Attribute("PlatformType", "char", "1",
    "spacecraft");
Attribute* project_id = new Attribute("project_id", "char", "1", "SeaWinds");
Attribute* data_format_type = new Attribute("data_format_type", "char", "1",
    "NCSA HDF");
Attribute* qa_percent_out_of_bounds_data =
    new Attribute("QAPercentOutOfBoundsData", "int", "1", "0");
Attribute* qa_percent_missing_data = new Attribute("QAPercentMissingData",
    "int", "1", "0");
Attribute* build_id = new Attribute("build_id", "char", "1", "1.0/Sim");
Attribute* hdf_version_id = new Attribute("HDF_version_id", "char", "1",
    "4.1r3");
Attribute* production_date_time = new Attribute("production_date_time",
    "char", "1", "<missing>");
Attribute* sis_id = new Attribute("sis_id", "char", "1",
    "686-644-5/2000-04-01");
Attribute* operation_mode = new Attribute("OperationMode", "char", "1",
    "Wind Observation");
Attribute* start_orbit_number = new Attribute("StartOrbitNumber", "int", "1",
    "<missing>");
Attribute* stop_orbit_number = new Attribute("StopOrbitNumber", "int", "1",
    "<missing>");
Attribute* equator_crossing_longitude =
    new Attribute("EquatorCrossingLongitude", "float", "1", "<missing>");
Attribute* equator_crossing_date = new Attribute("EquatorCrossingDate",
    "char", "1", "<missing>");
Attribute* equator_crossing_time = new Attribute("EquatorCrossingTime",
    "char", "1", "<missing>");
Attribute* rev_number = new Attribute("rev_number", "int", "1", "<missing>");
Attribute* rev_orbit_period = new Attribute("rev_orbit_period", "float", "1",
    "<missing>");
Attribute* orbit_inclination = new Attribute("orbit_inclination", "float", "1",
    "<missing>");
Attribute* orbit_semi_major_axis = new Attribute("orbit_semi_major_axis",
    "float", "1", "<missing>");

Attribute* g_attribute_table[] =
{
    long_name,
    short_name,
    producer_agency,
    producer_institution,
    platform_type,
    instrument_short_name,
    platform_long_name,
    platform_short_name,
    project_id,
    data_format_type,
    qa_percent_out_of_bounds_data,
    qa_percent_missing_data,
    build_id,
    hdf_version_id,
    production_date_time,
    sis_id,
    operation_mode,
    start_orbit_number,
    stop_orbit_number,
    equator_crossing_longitude,
    equator_crossing_date,
    equator_crossing_time,
    rev_number,
    rev_orbit_period,
    orbit_inclination,
    orbit_semi_major_axis,
    NULL
};

//====================//
// The table of SDS's //
//====================//

int32 dim_sizes_frame[] = { SD_UNLIMITED };
const char* dim_names_frame[] = { "Telemetry_Frame" };

SdsFloat64* frame_time_secs = new SdsFloat64("frame_time_secs", 1,
    dim_sizes_frame, "sec", 1.0, 0.0, dim_names_frame, 5.0E9, 0.0);
SdsFloat64* instrument_time = new SdsFloat64("instrument_time", 1,
    dim_sizes_frame, "counts", 1.0, 0.0, dim_names_frame, pow(2.0, 36.0), 0.0);
SdsUInt32* orbit_time = new SdsUInt32("orbit_time", 1, dim_sizes_frame,
    "counts", 1.0, 0.0, dim_names_frame, 4294967295, 0);
SdsFloat32* x_pos = new SdsFloat32("x_pos", 1, dim_sizes_frame, "m", 1.0, 0.0,
    dim_names_frame, 9999999.0, -9999999.0);

Sds* g_sds_table[] =
{
    frame_time_secs,
    instrument_time,
    orbit_time,
    x_pos,
    NULL
};

//======//
// L1AH //
//======//

L1AH::L1AH()
:   _eqxTime(0.0), _rangeBeginningTime(0.0), _rangeEndingTime(0.0),
    _eqxLongitude(0.0), _hdfInputFileId(0), _hdfOutputFileId(0),
    _sdsInputFileId(0), _sdsOutputFileId(0), _currentRecordIdx(0)
{
    _referenceEtime.FromCodeA("2000-01-01");
    return;
}

//------------------//
// L1AH::NextRecord //
//------------------//

int
L1AH::NextRecord()
{
    _currentRecordIdx++;
    return (_currentRecordIdx);
}

//-------------------------//
// L1AH::OpenHdfForWriting //
//-------------------------//

int
L1AH::OpenHdfForWriting()
{
    _hdfOutputFileId = Hopen(_outputFilename, DFACC_CREATE, 0);
    if (_hdfOutputFileId == FAIL)
    {
        return (0);
    }
    return (1);
}

//-------------------------//
// L1AH::OpenHdfForReading //
//-------------------------//

int
L1AH::OpenHdfForReading()
{
    _hdfInputFileId = Hopen(_inputFilename, DFACC_READ, 0);
    if (_hdfInputFileId == FAIL)
    {
        return (0);
    }
    return (1);
}

//--------------------//
// L1AH::CreateVdatas //
//--------------------//

int
L1AH::CreateVdatas()
{
    // initialize the Vdata interface
    if (Vstart(_hdfOutputFileId) != SUCCEED)
    {
        fprintf(stderr, "CreateVdatas: error with Vstart\n");
        return(0);
    }

    // create a new vdata (the -1 means create)
    int32 vdata_id = VSattach(_hdfOutputFileId, -1, "w");

    // set the name and class
    if (VSsetname(vdata_id, FRAME_TIME_NAME) != SUCCEED)
    {
        fprintf(stderr, "CreateVdatas: error with VSsetname\n");
        return(0);
    }
    if (VSsetclass(vdata_id, FRAME_TIME_NAME) != SUCCEED)
    {
        fprintf(stderr, "CreateVdatas: error with VSsetclass\n");
        return(0);
    }

    // define
    if (VSfdefine(vdata_id, FRAME_TIME_NAME, 21, 21) != SUCCEED)
    {
        fprintf(stderr, "CreateVdatas: error with VSfdefine\n");
        return(0);
    }

    // set fields
    if (VSsetfields(vdata_id, FRAME_TIME_NAME) != SUCCEED)
    {
        fprintf(stderr, "CreateVdatas: error with VSsetfields\n");
        return(0);
    }

    // detach
    if (VSdetach(vdata_id) != SUCCEED)
    {
        fprintf(stderr, "CreateVdatas: error with VSdetach\n");
        return(0);
    }

    return (1);
}

//-------------------//
// L1AH::WriteVdatas //
//-------------------//
// coverts the frame time into the appropriate format for writing
// to a vdata

int
L1AH::WriteVdatas()
{
    double add_seconds = (double)_referenceEtime.GetSec();

    // add the reference time to the delta time to get the "real" time
    ETime real_time;
    real_time.SetTime(frame.time + add_seconds);

    // convert to a string
    char string[CODE_B_TIME_LENGTH];
    real_time.ToCodeB(string);

    // get the reference
    int32 vdata_ref = VSfind(_hdfOutputFileId, FRAME_TIME_NAME);
    if (vdata_ref == 0)
    {
        fprintf(stderr, "WriteVdatas: error with VSfind\n");
        return(0);
    }

    // attach
    int32 vdata_id = VSattach(_hdfOutputFileId, vdata_ref, "w");
    if (vdata_id == FAIL)
    {
        fprintf(stderr, "WriteVdatas: error with VSattach\n");
        return(0);
    }

    // seek
    if (_currentRecordIdx > 0)
    {
        // dumb-ass HDF seek function can't seek to the end
        // you are "supposed" to seek to one before the end...
        if (VSseek(vdata_id, _currentRecordIdx - 1) == FAIL)
        {
            fprintf(stderr, "WriteVdatas: error with VSseek\n");
            return(0);
        }
        // ...and then read the last one
        unsigned char dummy[CODE_B_TIME_LENGTH];
        if (VSread(vdata_id, dummy, 1, FULL_INTERLACE) != 1)
        {
            fprintf(stderr, "WriteVdatas: error with VSread\n");
            return(0);
        }
    }

    // write
    if (VSwrite(vdata_id, (unsigned char *)string, 1, FULL_INTERLACE) != 1)
    {
        fprintf(stderr, "WriteVdatas: error with VSwrite\n");
        return(0);
    }

    // detach
    if (VSdetach(vdata_id) != SUCCEED)
    {
        fprintf(stderr, "WriteVdatas: error with VSdetach\n");
        return(0);
    }

    return(1);
}

//----------------------//
// L1AH::EndVdataOutput //
//----------------------//

int
L1AH::EndVdataOutput()
{
    if (Vend(_hdfOutputFileId) != SUCCEED)
    {
        fprintf(stderr, "EndVdataOutput: error wind Vend\n");
        return(0);
    }
    return(1);
}

//-------------------------//
// L1AH::OpenSDSForWriting //
//-------------------------//

int
L1AH::OpenSDSForWriting()
{
    _sdsOutputFileId = SDstart(_outputFilename, DFACC_WRITE);
    if (_sdsOutputFileId == FAIL)
    {
        return (0);
    }
    return (1);
}

//------------------//
// L1AH::CreateSDSs //
//------------------//

int
L1AH::CreateSDSs()
{
    for (int idx = 0; g_sds_table[idx] != NULL; idx++)
    {
        Sds* sds = g_sds_table[idx];
        if (! sds->Create(_sdsOutputFileId))
        {
            fprintf(stderr, "L1AH::CreateSDSs: error creating SDS %d\n", idx);
            return(0);
        }
    }
    return (1);
}

//-----------------//
// L1AH::WriteSDSs //
//-----------------//
int
L1AH::WriteSDSs()
{
    //-----------------------//
    // set all of the values //
    //-----------------------//

    frame_time_secs->SetFromDouble(&(frame.time));
    instrument_time->SetFromUnsignedInt(&(frame.instrumentTicks));
    orbit_time->SetWithUnsignedInt(&(frame.orbitTicks));
    x_pos->SetFromFloat(&(frame.gcX));

    //-------------------------------------------//
    // determine some information from the frame //
    //-------------------------------------------//

    EqxCheck();
    if (_currentRecordIdx == 0)
    {
        _rangeBeginningTime = frame.time;
    }
    else
    {
        _rangeEndingTime = frame.time;
    }

    //----------------//
    // and write them //
    //----------------//

    for (int idx = 0; g_sds_table[idx] != NULL; idx++)
    {
        Sds* sds = g_sds_table[idx];
        if (! sds->Write(_sdsOutputFileId, _currentRecordIdx))
        {
            fprintf(stderr, "L1AH::WriteSDSs: error writing SDS %s\n",
                sds->GetName());
            return(0);
        }
    }

    return(1);
}

//--------------------//
// L1AH::EndSDSOutput //
//--------------------//

int
L1AH::EndSDSOutput()
{
    for (int idx = 0; g_sds_table[idx] != NULL; idx++)
    {
        Sds* sds = g_sds_table[idx];
        if (! sds->EndAccess())
        {
            fprintf(stderr, "L1AH::EndSDSOutput: error ending SDS access\n");
            return(0);
        }
    }
    if (SDend(_sdsOutputFileId) != SUCCEED)
    {
        fprintf(stderr, "L1AH::EndSDSOutput: error wind SDend\n");
        return(0);
    }
    return(1);
}

//---------------------//
// L1AH::WriteHDFFrame //
//---------------------//

int
L1AH::WriteHDFFrame()
{
    if (! WriteVdatas())
    {
        fprintf(stderr, "L1AH::WriteHDFFrame: error with WriteVdatas\n");
        return(0);
    }
    if (! WriteSDSs())
    {
        fprintf(stderr, "L1AH::WriteHDFFrame: error with WriteSDSs\n");
        return(0);
    }
    return (1);
}

//----------------------//
// L1AH::WriteHDFHeader //
//----------------------//

int
L1AH::WriteHDFHeader(
    double  period,
    double  inclination,
    double  sma)
{
    //--------------------------------------------//
    // set up all attributes that need setting up //
    //--------------------------------------------//

    char buffer[1024];

    // production date time
    ETime etime;
    etime.CurrentTime();
    etime.ToCodeB(buffer);
    production_date_time->ReplaceContents(buffer);

    // start and stop orbit numbers
    int start_orbit = (int)(_rangeBeginningTime / period) + 1;
    sprintf(buffer, "%d", start_orbit);
    start_orbit_number->ReplaceContents(buffer);

    int stop_orbit = (int)(_rangeEndingTime / period) + 1;
    sprintf(buffer, "%d", stop_orbit);
    stop_orbit_number->ReplaceContents(buffer);

    // eqx longitude
    sprintf(buffer, "%.4f", _eqxLongitude);
    equator_crossing_longitude->ReplaceContents(buffer);

    // eqx date
    ETime eqx_etime;
    eqx_etime.SetTime(_eqxTime + _referenceEtime.GetSec());
    eqx_etime.ToCodeB(buffer);
    buffer[8] = '\0';
    equator_crossing_date->ReplaceContents(buffer);

    // eqx time
    equator_crossing_time->ReplaceContents(buffer + 9);

    // rev number
    sprintf(buffer, "%d", stop_orbit);
    rev_number->ReplaceContents(buffer);

    // rev orbit period
    sprintf(buffer, "%.3f", period);
    rev_orbit_period->ReplaceContents(buffer);

    // orbit inclination
    sprintf(buffer, "%.5f", inclination);
    orbit_inclination->ReplaceContents(buffer);

    // orbit semi major axis
    sprintf(buffer, "%d", (int)sma);
    orbit_semi_major_axis->ReplaceContents(buffer);

    //----------------------//
    // write out attributes //
    //----------------------//

    for (int idx = 0; g_attribute_table[idx] != NULL; idx++)
    {
        if (! (g_attribute_table[idx])->Write(_sdsOutputFileId))
        {
            return(0);
        }
    }

    return (1);
}

//-------------------------//
// L1AH::CloseHdfInputFile //
//-------------------------//

int
L1AH::CloseHdfInputFile()
{
    if (Hclose(_hdfInputFileId) != SUCCEED)
        return (0);
    return(1);
}

//--------------------------//
// L1AH::CloseHdfOutputFile //
//--------------------------//

int
L1AH::CloseHdfOutputFile()
{
    if (Hclose(_hdfOutputFileId) != SUCCEED)
        return (0);
    return(1);
}

//----------------//
// L1AH::EqxCheck //
//----------------//

void
L1AH::EqxCheck()
{
    static unsigned int last_orbit_ticks = 0;
    if (frame.orbitTicks < last_orbit_ticks)
    {
        // orbit timer must have gotten reset
        // eqx time is the time at 0
        _eqxTime = frame.time -
            (double)frame.orbitTicks / ORBIT_TICKS_PER_SECOND;
        _eqxLongitude = frame.gcLongitude * rtd;
    }
    last_orbit_ticks = frame.orbitTicks;
    return;
}

/*
L1AH::L1AH()
{
    return;
}

L1AH::~L1AH()
{
    return;
}
*/
