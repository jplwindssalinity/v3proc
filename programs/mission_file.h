//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef MISSION_FILE_H
#define MISSION_FILE_H

static const char rcs_id_mission_file_h[] =
    "@(#) $Id$";

#include <stdio.h>

//======================================================================
// CLASS
//    MissionFile
//
// DESCRIPTION
//    The MissionFile object handles level 0 type data.
//======================================================================

#define SFDU_PACKET_SIZE   868
#define SFDU_HEADER_SIZE   72

#define BALL_PACKET_SIZE   804
#define BALL_HEADER_SIZE   6

#define TOTAL_BUFFER_SIZE  3000

class MissionFile
{
public:
    enum   MissionFileTypeE { SFDU, BALL };

    MissionFile();
    int    Open(const char* filename);
    int    SetType(MissionFileTypeE type);
    int    GetCompleteFrame();
    char*  GetPacket1() { return(_packet1); };
    char*  GetPacket2() { return(_packet2); };
    char*  GetPacket3() { return(_packet3); };
    char*  GetData1() { return(_data1); };
    char*  GetData2() { return(_data2); };
    char*  GetData3() { return(_data3); };
    int    Eof();
    int    Close();

private:
    FILE*             _ifp;
    char              _buffer[TOTAL_BUFFER_SIZE];
    char*             _currentPtr;
    char*             _packet1;
    char*             _data1;
    char*             _packet2;
    char*             _data2;
    char*             _packet3;
    char*             _data3;

    MissionFileTypeE  _fileType;
    int               _packetSize;
    int               _headerSize;
};

MissionFile::MissionFile()
:   _ifp(NULL), _currentPtr(_buffer), _packet1(NULL), _data1(NULL),
    _packet2(NULL), _data2(NULL), _packet3(NULL), _data3(NULL),
    _fileType(SFDU), _packetSize(0), _headerSize(0)
{
    return;
}

//-------------------//
// MissionFile::Open //
//-------------------//

int
MissionFile::Open(
    const char*  filename)
{
    if (_ifp != NULL)
        fclose(_ifp);
    _ifp = fopen(filename, "r");
    if (_ifp == NULL)
        return(0);
    return(1);
}

//----------------------//
// MissionFile::SetType //
//----------------------//

int
MissionFile::SetType(
    MissionFileTypeE  type)
{
    _fileType = type;
    switch (_fileType)
    {
    case SFDU:
        _packetSize = SFDU_PACKET_SIZE;
        _headerSize = SFDU_HEADER_SIZE;
        break;
    case BALL:
        _packetSize = BALL_PACKET_SIZE;
        _headerSize = BALL_HEADER_SIZE;
        break;
    default:
        _packetSize = 0;
        _headerSize = 0;
        break;
    }

    _packet1 = _buffer;
    _data1 = _packet1 + _headerSize;
    _packet2 = _buffer + _packetSize;
    _data2 = _packet2 + _headerSize;
    _packet3 = _buffer + 2 * _packetSize;
    _data3 = _packet3 + _headerSize;

    return(1);
}

//-------------------------------//
// MissionFile::GetCompleteFrame //
//-------------------------------//

int
MissionFile::GetCompleteFrame()
{
    static int last_count = -1;
    do
    {
        // read a packet
        if (fread(_currentPtr, _packetSize, 1, _ifp) != 1)
            return(0);

        // determine the id
        unsigned char id;
        memcpy(&id, _currentPtr + _headerSize + 2, 1);
        id &= 0xc0;
        id >>= 6;

        int count;
        memcpy(&count, _currentPtr + _headerSize, 4);
        count &= 0x3FFF;

        switch(id)
        {
        case 1:    // first packet (is always good)
            if (_currentPtr == _packet1)
            {
                _currentPtr = _packet2;
            }
            else
            {
                // not in first slot, put it there
                memcpy(_packet1, _currentPtr, _packetSize);
                _currentPtr = _packet2;
            }
            break;
        case 0:    // second packet
            if (count == (last_count + 1) % 16384 && _currentPtr == _packet2)
            {
                _currentPtr = _packet3;
            }
            else
            {
                _currentPtr = _packet1;
            }
            break;
        case 2:    // third packet
            if (count == (last_count + 1) % 16384 && _currentPtr == _packet3)
            {
                _currentPtr = _packet1;
                return(1);
            }
            else
            {
                _currentPtr = _packet1;
            }
            break;
        }
        last_count = count;
    } while (1);
    return(0);
}

//------------------//
// MissionFile::Eof //
//------------------//

int
MissionFile::Eof()
{
    return(feof(_ifp));
}

//--------------------//
// MissionFile::Close //
//--------------------//

int
MissionFile::Close()
{
    fclose(_ifp);
    _ifp = NULL;
    return(1);
}

#endif
