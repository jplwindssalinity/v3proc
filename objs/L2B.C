//==============================================================//
// Copyright (C) 1997-2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_l2b_c[] =
    "@(#) $Id$";

#include <memory.h>
#include "L2B.h"
#include "TlmHdfFile.h"
#include "NoTimeTlmFile.h"
#include "ParTab.h"
#include "L1AExtract.h"
#include "Wind.h"

#define HDF_ACROSS_BIN_NO    76
#define HDF_NUM_AMBIGUITIES  4

//===========//
// L2BHeader //
//===========//

L2BHeader::L2BHeader()
:   crossTrackResolution(0.0), alongTrackResolution(0.0), zeroIndex(0)
{
    return;
}

L2BHeader::~L2BHeader()
{
    return;
}

//-----------------//
// L2BHeader::Read //
//-----------------//

int
L2BHeader::Read(
    FILE*  fp)
{
    if (fread(&crossTrackResolution, sizeof(float), 1, fp) != 1 ||
        fread(&alongTrackResolution, sizeof(float), 1, fp) != 1 ||
        fread(&zeroIndex, sizeof(int), 1, fp) != 1)
    {
        return(0);
    }
    return(1);
}

//------------------//
// L2BHeader::Write //
//------------------//

int
L2BHeader::Write(
    FILE*  fp)
{
    if (fwrite(&crossTrackResolution, sizeof(float), 1, fp) != 1 ||
        fwrite(&alongTrackResolution, sizeof(float), 1, fp) != 1 ||
        fwrite(&zeroIndex, sizeof(int), 1, fp) != 1)
    {
        return(0);
    }
    return(1);
}

//-----------------------//
// L2BHeader::WriteAscii //
//-----------------------//

int
L2BHeader::WriteAscii(
    FILE*  fp)
{
        fprintf(fp, "############################################\n");
        fprintf(fp, "##                L2B DataFile            ##\n");
        fprintf(fp, "############################################\n");
        fprintf(fp,"\n\nCrossTrackRes %g AlongTrackRes %g ZeroIndex %d\n\n",
        crossTrackResolution,alongTrackResolution,zeroIndex);
    return(1);
}

//==========//
// L2BFrame //
//==========//

L2BFrame::L2BFrame()
{
    return;
}

L2BFrame::~L2BFrame()
{
    return;
}

//=====//
// L2B //
//=====//

L2B::L2B()
:   _status(OK)
{
    return;
}

L2B::~L2B()
{
    return;
}

//----------------//
// L2B::WriteVctr //
//----------------//

int
L2B::WriteVctr(
    const char*  filename,
    const int    rank)
{
    return(frame.swath.WriteVctr(filename, rank));
}


//-----------------//
// L2B::WriteAscii //
//-----------------//

int
L2B::WriteAscii()
{
    if (! header.WriteAscii(_outputFp))
        return(0);
    return(frame.swath.WriteAscii(_outputFp));
}

//--------------//
// L2B::ReadHDF //
//--------------//

int
L2B::ReadHDF(
    int  unnormalize_mle)
{
    //------//
    // read //
    //------//

    if (! ReadHDF(_inputFilename, unnormalize_mle))
        return(0);

    return(1);
}

//--------------//
// L2B::ReadHDF //
//--------------//

int
L2B::ReadHDF(
    const char*  filename,
    int          unnormalize_mle)
{
    //--------------------------------//
    // convert filename to TlmHdfFile //
    //--------------------------------//

    HdfFile::StatusE returnStatus = HdfFile::OK;
    NoTimeTlmFile l2b_hdf_file(filename, SOURCE_L2B, returnStatus);
    if (returnStatus != HdfFile::OK)
        return(0);

    //------//
    // read //
    //------//

    if (! ReadHDF(&l2b_hdf_file, unnormalize_mle))
        return(0);

    return(1);
}

//--------------//
// L2B::ReadHDF //
//--------------//

int
L2B::ReadHDF(
    TlmHdfFile*  tlmHdfFile,
    int          unnormalize_mle)
{
    WindSwath* swath = &(frame.swath);
    swath->DeleteEntireSwath();    // just in case

    // along bin number comes from WVC_ROW
    const char* rowSdsName = ParTabAccess::GetSdsNames(SOURCE_L2B, WVC_ROW);
    if (rowSdsName == 0)
        return(0);

    int32 dataType = 0;
    int32 dataStartIndex = 0;
    int32 dataLength = 0;
    int32 numDimensions = 0;
    int32 rowSdsId = tlmHdfFile->SelectDataset(rowSdsName, dataType,
              dataStartIndex, dataLength, numDimensions);
    if (rowSdsId == HDF_FAIL)
        return(0);

    // open all needed datasets
    if ( ! _OpenHdfDataSets(tlmHdfFile))
        return(0);

    //--------------//
    // set up swath //
    //--------------//

    int along_track_bins = dataLength;
    int cross_track_bins = HDF_ACROSS_BIN_NO;

// hack in high resolution
#ifdef HIRES12
    cross_track_bins = 152;
#endif

    if (! swath->Allocate(cross_track_bins, along_track_bins))
        return(0);

    //---------------//
    // create arrays //
    //---------------//

    unsigned char*  numambigArray = new unsigned char[cross_track_bins];
    float*          lonArray = new float[cross_track_bins];
    float*          latArray = new float[cross_track_bins];
    float*          speedArray = (float *)new float[cross_track_bins *
                        HDF_NUM_AMBIGUITIES];
    float*          dirArray = (float *)new float[cross_track_bins *
                        HDF_NUM_AMBIGUITIES];
    float*          mleArray = (float *)new float[cross_track_bins *
                        HDF_NUM_AMBIGUITIES];
    char*           selectArray = new char[cross_track_bins];
    float*          modelSpeedArray = (float *) new float[cross_track_bins];
    float*          modelDirArray = (float *) new float[cross_track_bins];
    char*           tmpArray = new char[cross_track_bins];
    int*            numArray = (int *) new int[cross_track_bins];
    int32           sdsIds[1];

    for (int32 ati = 0; ati < along_track_bins; ati++)
    {
        for (int c = 0; c < cross_track_bins; c++)
            numArray[c] = 0;

        sdsIds[0] = _lonSdsId;
        if (ExtractData2D_76_uint2_float(tlmHdfFile, sdsIds, ati, 1, 1,
            lonArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _latSdsId;
        if (ExtractData2D_76_int2_float(tlmHdfFile, sdsIds, ati, 1, 1,
            latArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _speedSdsId;
        if (ExtractData3D_76_4_int2_float(tlmHdfFile, sdsIds, ati, 1, 1,
            speedArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _dirSdsId;
        if (ExtractData3D_76_4_uint2_float(tlmHdfFile, sdsIds, ati, 1, 1,
            dirArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _mleSdsId;
        if (ExtractData3D_76_4_int2_float(tlmHdfFile, sdsIds, ati, 1, 1,
            mleArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _selectSdsId;
        if (ExtractData2D_76(tlmHdfFile, sdsIds, ati, 1, 1, selectArray) == 0)
            return(0);

        sdsIds[0] = _numambigSdsId;
        if (ExtractData2D_76(tlmHdfFile, sdsIds, ati, 1, 1,
            numambigArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _modelSpeedSdsId;
        if (ExtractData2D_76_int2_float(tlmHdfFile, sdsIds, ati, 1, 1,
            modelSpeedArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _modelDirSdsId;
        if (ExtractData2D_76_int2_float(tlmHdfFile, sdsIds, ati, 1, 1,
            modelDirArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _numInForeSdsId;
        if (ExtractData2D_76(tlmHdfFile, sdsIds, ati, 1, 1, tmpArray) == 0)
            return(0);
        for (int c = 0; c < cross_track_bins; c++)
            numArray[c] += tmpArray[c];

        sdsIds[0] = _numInAftSdsId;
        if (ExtractData2D_76(tlmHdfFile, sdsIds, ati, 1, 1, tmpArray) == 0)
            return(0);
        for (int c = 0; c < cross_track_bins; c++)
            numArray[c] += tmpArray[c];

        sdsIds[0] = _numOutForeSdsId;
        if (ExtractData2D_76(tlmHdfFile, sdsIds, ati, 1, 1, tmpArray) == 0)
            return(0);
        for (int c = 0; c < cross_track_bins; c++)
            numArray[c] += tmpArray[c];

        sdsIds[0] = _numOutAftSdsId;
        if (ExtractData2D_76(tlmHdfFile, sdsIds, ati, 1, 1, tmpArray) == 0)
            return(0);
        for (int c = 0; c < cross_track_bins; c++)
            numArray[c] += tmpArray[c];

        for (int cti = 0; cti < cross_track_bins; cti++)
        {
            WVC* wvc = new WVC();
            wvc->lonLat.longitude = lonArray[cti] * dtr;
            wvc->lonLat.latitude = latArray[cti] * dtr;
            wvc->nudgeWV = new WindVectorPlus();
            float nudge_edir = (450.0 - modelDirArray[cti]) * dtr;
            while (nudge_edir > two_pi)
                nudge_edir -= two_pi;

            while (nudge_edir < 0)
                nudge_edir += two_pi;

            wvc->nudgeWV->SetSpdDir(modelSpeedArray[cti] * NWP_SPEED_CORRECTION,
                nudge_edir);

            for (int k = 0; k < numambigArray[cti]; k++)
            {
                WindVectorPlus* wvp = new WindVectorPlus();
                float edir = (450.0 -
                    dirArray[cti * HDF_NUM_AMBIGUITIES + k]) * dtr;
                while (edir > two_pi)
                    edir -= two_pi;

                while (edir < 0)
                    edir += two_pi;

                wvp->SetSpdDir(speedArray[cti * HDF_NUM_AMBIGUITIES + k], edir);
                if (unnormalize_mle)
                {
                    wvp->obj = mleArray[cti * HDF_NUM_AMBIGUITIES + k] *
                        numArray[cti];
                }
                else
                {
                    wvp->obj = mleArray[cti * HDF_NUM_AMBIGUITIES + k];
                }
                wvc->ambiguities.Append(wvp);
            }
            if (selectArray[cti] > 0 && numambigArray[cti] > 0)
            {
                wvc->selected =
                    wvc->ambiguities.GetByIndex(selectArray[cti] - 1);
                swath->Add(cti, ati, wvc);
            }
            else
            {
                delete wvc;
            }
        }
    }

    delete [] modelDirArray;
    delete [] modelSpeedArray;
    delete [] numambigArray;
    delete [] lonArray;
    delete [] latArray;
    delete [] speedArray;
    delete [] dirArray;
    delete [] mleArray;
    delete [] selectArray;
    delete [] numArray;
    delete [] tmpArray;

    // close all needed datasets
    _CloseHdfDataSets();

    swath->nudgeVectorsRead = 1;
    header.crossTrackResolution = 25.0;
    header.alongTrackResolution = 25.0;
    header.zeroIndex = 38;

    return(1);
}

//---------------------------------//
// L2B::ReadNudgeVectorsFromHdfL2B //
//---------------------------------//

int
L2B::ReadNudgeVectorsFromHdfL2B(
    const char*  filename)
{
    //--------------------------------//
    // convert filename to TlmHdfFile //
    //--------------------------------//

    HdfFile::StatusE returnStatus = HdfFile::OK;
    NoTimeTlmFile l2b_hdf_file(filename, SOURCE_L2B, returnStatus);
    if (returnStatus != HdfFile::OK)
        return(0);

    //------//
    // read //
    //------//

    if (! ReadNudgeVectorsFromHdfL2B(&l2b_hdf_file))
        return(0);

    return(1);
}

//---------------------------------//
// L2B::ReadNudgeVectorsFromHdfL2B //
//---------------------------------//

int
L2B::ReadNudgeVectorsFromHdfL2B(
    TlmHdfFile*  tlmHdfFile)
{
    WindSwath* swath = &(frame.swath);

    int crossTrackBins = HDF_ACROSS_BIN_NO;
#ifdef HIRES12
    crossTrackBins = 152;
#endif

    if (crossTrackBins != swath->GetCrossTrackBins())
    {
        fprintf(stderr,
            "ReadNudgeVectorsFromHdfL2B: crosstrackbins mismatch\n");
        return(0);
    }

    // along bin number comes from WVC_ROW
    const char* rowSdsName = ParTabAccess::GetSdsNames(SOURCE_L2B, WVC_ROW);
    if (rowSdsName == 0)
        return(0);

    int32 dataType = 0;
    int32 dataStartIndex = 0;
    int32 dataLength = 0;
    int32 numDimensions = 0;
    int32 rowSdsId = tlmHdfFile->SelectDataset(rowSdsName, dataType,
              dataStartIndex, dataLength, numDimensions);
    if (rowSdsId == HDF_FAIL)
        return(0);

    int alongTrackBins = dataLength;

    // For now do not handle case in which WVC rows are missing
    if (alongTrackBins != swath->GetAlongTrackBins())
    {
        fprintf(stderr, "Unable to process missing WVC rows in HDF file\n");
        return(0);
    }

    /**** Open Nudge Vector Data Sets ***/
    if ((_modelSpeedSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        MODEL_SPEED)) == 0)
    {
        return(0);
    }
    if ((_modelDirSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        MODEL_DIR)) == 0)
    {
        return(0);
    }

    float* modelSpeedArray=(float*) new float[crossTrackBins];
    float* modelDirArray=(float*) new float[crossTrackBins];

    int32 sdsIds[1];
    for (int32 ati = 0; ati < alongTrackBins; ati++)
    {
        sdsIds[0] = _modelSpeedSdsId;
        if (ExtractData2D_76_int2_float(tlmHdfFile, sdsIds, ati, 1, 1,
            modelSpeedArray) == 0)
        {
            return(0);
        }

        sdsIds[0] = _modelDirSdsId;
        if (ExtractData2D_76_int2_float(tlmHdfFile, sdsIds, ati, 1, 1,
            modelDirArray) == 0)
        {
             return(0);
        }
        for (int cti = 0; cti < crossTrackBins; cti++)
        {
            WVC* wvc = swath->GetWVC(cti, ati);
            if (! wvc)
                continue;
            wvc->nudgeWV = new WindVectorPlus();
            float nudge_edir = (450.0 - modelDirArray[cti]) * dtr;
            while (nudge_edir > two_pi)
                nudge_edir -= two_pi;

            while (nudge_edir < 0)
                nudge_edir += two_pi;
            wvc->nudgeWV->SetSpdDir(modelSpeedArray[cti] * NWP_SPEED_CORRECTION,
                nudge_edir);
        }
    }
    delete [] modelDirArray;
    delete [] modelSpeedArray;

    // close datasets
    (void)SDendaccess(_modelSpeedSdsId); _modelSpeedSdsId = HDF_FAIL;
    (void)SDendaccess(_modelDirSdsId); _modelDirSdsId = HDF_FAIL;

    swath->nudgeVectorsRead = 1;

    return(1);
}

//-----------------------------------//
// L2B::GetArraysForUpdatingDirthHdf //
//-----------------------------------//

int
L2B::GetArraysForUpdatingDirthHdf(
    float**  spd,
    float**  dir,
    int**    num_ambig)
{
    WindSwath* swath = &(frame.swath);
    int along_track_bins = swath->GetAlongTrackBins();
    int cross_track_bins = swath->GetCrossTrackBins();

    for (int cti = 0; cti < cross_track_bins; cti++)
    {
        for (int ati = 0; ati < along_track_bins; ati++)
        {
            WVC* wvc = swath->GetWVC(cti, ati);
            if (! wvc)
            {
                num_ambig[ati][cti] = 0;
            }
            else
            {
                int k = 0;
                num_ambig[ati][cti] = wvc->ambiguities.NodeCount() + 1;
                if (num_ambig[ati][cti] > HDF_NUM_AMBIGUITIES)
                num_ambig[ati][cti] = HDF_NUM_AMBIGUITIES;
                for (WindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
                    wvp = wvc->ambiguities.GetNext())
                {
                    spd[ati][cti*HDF_NUM_AMBIGUITIES+k] = wvp->spd;
                    dir[ati][cti*HDF_NUM_AMBIGUITIES+k] = wvp->dir;
                    k++;
                }
            }
        }
    }
    return(1);
}

//-----------------------//
// L2B::_OpenHdfDataSets //
//-----------------------//

int
L2B::_OpenHdfDataSets(
    TlmHdfFile*  tlmHdfFile)
{
    if ((_numambigSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        NUM_AMBIGS)) == 0)
    {
        return(0);
    }
    if ((_lonSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B, WVC_LON)) == 0)
    {
        return(0);
    }
    if ((_latSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B, WVC_LAT)) == 0)
    {
        return(0);
    }
    if ((_speedSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        WIND_SPEED)) == 0)
    {
        return(0);
    }
    if ((_dirSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        WIND_DIR)) == 0)
    {
        return(0);
    }
    if ((_modelSpeedSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        MODEL_SPEED)) == 0)
    {
        return(0);
    }
    if ((_modelDirSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        MODEL_DIR)) == 0)
    {
        return(0);
    }
    if ((_mleSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        MAX_LIKELIHOOD_EST)) == 0)
    {
        return(0);
    }
    if ((_selectSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        WVC_SELECTION)) == 0)
    {
        return(0);
    }
    if ((_numInForeSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        NUM_IN_FORE)) == 0)
    {
        return(0);
    }
    if ((_numInAftSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        NUM_IN_AFT)) == 0)
    {
        return(0);
    }
    if ((_numOutForeSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        NUM_OUT_FORE)) == 0)
    {
        return(0);
    }
    if ((_numOutAftSdsId = _OpenOneHdfDataSet(tlmHdfFile, SOURCE_L2B,
        NUM_OUT_AFT)) == 0)
    {
        return(0);
    }

    return(1);
}

//-------------------------//
// L2B::_OpenOneHdfDataSet //
//-------------------------//

int
L2B::_OpenOneHdfDataSet(
    TlmHdfFile*  tlmHdfFile,
    SourceIdE    source,
    ParamIdE     param)
{
    const char* sdsName = ParTabAccess::GetSdsNames(source, param);
    if (sdsName == 0)
        return(0);

    int32 dataType = 0;
    int32 dataStartIndex = 0;
    int32 dataLength = 0;
    int32 numDimensions = 0;
    int32 sdsId = tlmHdfFile->SelectDataset(sdsName, dataType,
              dataStartIndex, dataLength, numDimensions);
    if (sdsId == HDF_FAIL)
        return(0);
    else
        return(sdsId);
}

//------------------------//
// L2B::_CloseHdfDataSets //
//------------------------//

void
L2B::_CloseHdfDataSets(void)
{
    (void)SDendaccess(_numambigSdsId); _numambigSdsId = HDF_FAIL;
    (void)SDendaccess(_lonSdsId); _lonSdsId = HDF_FAIL;
    (void)SDendaccess(_latSdsId); _latSdsId = HDF_FAIL;
    (void)SDendaccess(_speedSdsId); _speedSdsId = HDF_FAIL;
    (void)SDendaccess(_dirSdsId); _dirSdsId = HDF_FAIL;
    (void)SDendaccess(_mleSdsId); _mleSdsId = HDF_FAIL;
    (void)SDendaccess(_selectSdsId); _selectSdsId = HDF_FAIL;
    (void)SDendaccess(_numInForeSdsId); _numInForeSdsId = HDF_FAIL;
    (void)SDendaccess(_numInAftSdsId); _numInAftSdsId = HDF_FAIL;
    (void)SDendaccess(_numOutForeSdsId); _numOutForeSdsId = HDF_FAIL;
    (void)SDendaccess(_numOutAftSdsId); _numOutAftSdsId = HDF_FAIL;
    (void)SDendaccess(_modelSpeedSdsId); _modelSpeedSdsId = HDF_FAIL;
    (void)SDendaccess(_modelDirSdsId); _modelDirSdsId = HDF_FAIL;
    return;
}
