//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.0   04 Feb 1998 14:14:48   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:28:09  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef BINNING_H
#define BINNING_H

static const char rcs_binning_h[]="@(#) $Header$";

#define BINNING_CONSTANTS_SIZE  1504
#define CHANNEL_COUNT           4
#define ANTENNA_COUNT           6
#define CELL_COUNT              25

#define A0_OFFSET       0
#define A1_OFFSET       300
#define A2_OFFSET       600
#define P1_OFFSET       900
#define P2_OFFSET       1050
#define BW2_OFFSET      1200
#define PERIOD_OFFSET   1500
#define CHECKSUM_OFFSET 1502

//===================
// BinningParameters 
//===================

class BinningParameters
{
public:
    unsigned char   channel[ANTENNA_COUNT][CELL_COUNT];
    unsigned char   startBin[ANTENNA_COUNT][CELL_COUNT];
    unsigned char   endBin[ANTENNA_COUNT][CELL_COUNT];
    float           startFreq[ANTENNA_COUNT][CELL_COUNT];
    float           endFreq[ANTENNA_COUNT][CELL_COUNT];
    unsigned char   cellSize[ANTENNA_COUNT][CELL_COUNT];
    float           cellBw[ANTENNA_COUNT][CELL_COUNT];
};

//==================
// BinningConstants 
//==================

class BinningConstants
{
public:
    BinningConstants();
    ~BinningConstants();

    int     IsFrozen();
    int     Convert(BinningParameters* bp, int orbit_step,
                char fix_regression);
    float   BinToFreq(int antenna_index, int channel_index, int bin);
    int     FreqToBin(float frequency, int antenna, int channel);
    int     OrbitTimeToStep(unsigned int orbit_time);
    int     ReadBinary(const char* filename);
    int     WriteBinary(const char* filename);
    int     ReadNml(const char* filename);
    int     Array(char* bc_array);

    unsigned short  a0[ANTENNA_COUNT][CELL_COUNT];
    short           a1[ANTENNA_COUNT][CELL_COUNT];
    short           a2[ANTENNA_COUNT][CELL_COUNT];
    char            p1[ANTENNA_COUNT][CELL_COUNT];
    char            p2[ANTENNA_COUNT][CELL_COUNT];
    unsigned short  bw2[ANTENNA_COUNT][CELL_COUNT];
    unsigned short  period;
    unsigned short  checksum;

private:
    int     _FindVariable(char buffer[], int size, char* variable, int* index);
};

int CheckForCmds(char* array, int array_size, int* byte1, int* bit1,
    int* byte2, int* bit2, unsigned short* cmd_code, char** cmd_name);

#endif
