//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.1   26 Feb 1998 09:42:52   sally
// took out extra unused parameter
// 
//    Rev 1.0   04 Feb 1998 14:14:46   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:28:52  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

static const char rcs_id_binning_c[]="@(#) $Header$";

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "Binning.h"
#include "NSCAT.h"

//================
// some constants 
//================

const float bin_width[CHANNEL_COUNT] =
    { BIN_WIDTH_CH1, BIN_WIDTH_CH2, BIN_WIDTH_CH3, BIN_WIDTH_CH4};

const unsigned short fhh[CHANNEL_COUNT] = {15119, 19733, 21964, 24455};
const unsigned short fhl[CHANNEL_COUNT] = {0, 11521, 17732, 20634};
const unsigned short fhc[CHANNEL_COUNT] = {7560, 15627, 19848, 22545};
const unsigned short fhub[CHANNEL_COUNT] = {1260, 12206, 18085, 20953};
const unsigned short fhut[CHANNEL_COUNT] = {13859, 19048, 21611, 24137};
const unsigned short fhsbw[CHANNEL_COUNT] = {4134, 7611, 14771, 16357};
const unsigned short fhpbw[CHANNEL_COUNT] = {4961, 9134, 17724, 19629};
const unsigned short fhbinw[CHANNEL_COUNT] = {1110, 2043, 3965, 4391};

const short cosine_table[] =
{
    32767, 32767, 32766, 32766, 32765, 32763, 32761, 32759, 32757, 32755,
    32752, 32748, 32745, 32741, 32737, 32732, 32728, 32722, 32717, 32711,
    32705, 32699, 32692, 32685, 32678, 32671, 32663, 32655, 32646, 32637,
    32628, 32619, 32609, 32599, 32589, 32578, 32567, 32556, 32545, 32533,
    32521, 32508, 32495, 32482, 32469, 32455, 32441, 32427, 32412, 32397,
    32382, 32367, 32351, 32335, 32318, 32302, 32285, 32267, 32250, 32232,
    32213, 32195, 32176, 32157, 32137, 32118, 32098, 32077, 32057, 32036,
    32014, 31993, 31971, 31949, 31926, 31903, 31880, 31857, 31833, 31809,
    31785, 31760, 31736, 31710, 31685, 31659, 31633, 31607, 31580, 31553,
    31526, 31498, 31470, 31442, 31414, 31385, 31356, 31327, 31297, 31267,
    31237, 31206, 31176, 31145, 31113, 31082, 31050, 31017, 30985, 30952,
    30919, 30885, 30852, 30818, 30783, 30749, 30714, 30679, 30643, 30607,
    30571, 30535, 30498, 30462, 30424, 30387, 30349, 30311, 30273, 30234,
    30195, 30156, 30117, 30077, 30037, 29997, 29956, 29915, 29874, 29832,
    29791, 29749, 29706, 29664, 29621, 29578, 29535, 29491, 29447, 29403,
    29358, 29313, 29268, 29223, 29177, 29131, 29085, 29039, 28992, 28945,
    28898, 28850, 28803, 28755, 28706, 28658, 28609, 28560, 28510, 28460,
    28411, 28360, 28310, 28259, 28208, 28157, 28105, 28053, 28001, 27949,
    27896, 27843, 27790, 27737, 27683, 27629, 27575, 27521, 27466, 27411,
    27356, 27300, 27245, 27189, 27133, 27076, 27019, 26962, 26905, 26848,
    26790, 26732, 26674, 26615, 26556, 26497, 26438, 26378, 26319, 26259,
    26198, 26138, 26077, 26016, 25955, 25893, 25832, 25770, 25708, 25645,
    25582, 25519, 25456, 25393, 25329, 25265, 25201, 25137, 25072, 25007,
    24942, 24877, 24811, 24746, 24680, 24613, 24547, 24480, 24413, 24346,
    24279, 24211, 24143, 24075, 24007, 23938, 23870, 23801, 23731, 23662,
    23592, 23522, 23452, 23382, 23312, 23241, 23170, 23099, 23027, 22956,
    22884, 22812, 22739, 22667, 22594, 22521, 22448, 22375, 22301, 22227,
    22154, 22079, 22005, 21930, 21856, 21781, 21705, 21630, 21554, 21479,
    21403, 21326, 21250, 21173, 21096, 21019, 20942, 20865, 20787, 20709,
    20631, 20553, 20475, 20396, 20317, 20238, 20159, 20080, 20000, 19921,
    19841, 19761, 19680, 19600, 19519, 19438, 19357, 19276, 19195, 19113,
    19032, 18950, 18868, 18785, 18703, 18620, 18537, 18454, 18371, 18288,
    18204, 18121, 18037, 17953, 17869, 17784, 17700, 17615, 17530, 17445,
    17360, 17275, 17189, 17104, 17018, 16932, 16846, 16759, 16673, 16586,
    16499, 16413, 16325, 16238, 16151, 16063, 15976, 15888, 15800, 15712,
    15623, 15535, 15446, 15358, 15269, 15180, 15090, 15001, 14912, 14822,
    14732, 14643, 14553, 14462, 14372, 14282, 14191, 14101, 14010, 13919,
    13828, 13736, 13645, 13554, 13462, 13370, 13279, 13187, 13094, 13002,
    12910, 12817, 12725, 12632, 12539, 12446, 12353, 12260, 12167, 12074,
    11980, 11886, 11793, 11699, 11605, 11511, 11417, 11322, 11228, 11133,
    11039, 10944, 10849, 10754, 10659, 10564, 10469, 10374, 10278, 10183,
    10087, 9992, 9896, 9800, 9704, 9608, 9512, 9416, 9319, 9223,
    9126, 9030, 8933, 8836, 8739, 8642, 8545, 8448, 8351, 8254,
    8157, 8059, 7962, 7864, 7767, 7669, 7571, 7473, 7375, 7277,
    7179, 7081, 6983, 6885, 6786, 6688, 6590, 6491, 6393, 6294,
    6195, 6096, 5998, 5899, 5800, 5701, 5602, 5503, 5404, 5305,
    5205, 5106, 5007, 4907, 4808, 4708, 4609, 4509, 4410, 4310,
    4210, 4111, 4011, 3911, 3811, 3712, 3612, 3512, 3412, 3312,
    3212, 3112, 3012, 2911, 2811, 2711, 2611, 2511, 2410, 2310,
    2210, 2110, 2009, 1909, 1809, 1708, 1608, 1507, 1407, 1307,
    1206, 1106, 1005, 905, 804, 704, 603, 503, 402, 302,
    201, 101, 0, -101, -201, -302, -402, -503, -603, -704,
    -804, -905, -1005, -1106, -1206, -1307, -1407, -1507, -1608, -1708,
    -1809, -1909, -2009, -2110, -2210, -2310, -2410, -2511, -2611, -2711,
    -2811, -2911, -3012, -3112, -3212, -3312, -3412, -3512, -3612, -3712,
    -3811, -3911, -4011, -4111, -4210, -4310, -4410, -4509, -4609, -4708,
    -4808, -4907, -5007, -5106, -5205, -5305, -5404, -5503, -5602, -5701,
    -5800, -5899, -5998, -6096, -6195, -6294, -6393, -6491, -6590, -6688,
    -6786, -6885, -6983, -7081, -7179, -7277, -7375, -7473, -7571, -7669,
    -7767, -7864, -7962, -8059, -8157, -8254, -8351, -8448, -8545, -8642,
    -8739, -8836, -8933, -9030, -9126, -9223, -9319, -9416, -9512, -9608,
    -9704, -9800, -9896, -9992, -10087, -10183, -10278, -10374, -10469, -10564,
    -10659, -10754, -10849, -10944, -11039, -11133, -11228, -11322, -11417, -11511,
    -11605, -11699, -11793, -11886, -11980, -12074, -12167, -12260, -12353, -12446,
    -12539, -12632, -12725, -12817, -12910, -13002, -13094, -13187, -13279, -13370,
    -13462, -13554, -13645, -13736, -13828, -13919, -14010, -14101, -14191, -14282,
    -14372, -14462, -14553, -14643, -14732, -14822, -14912, -15001, -15090, -15180,
    -15269, -15358, -15446, -15535, -15623, -15712, -15800, -15888, -15976, -16063,
    -16151, -16238, -16325, -16413, -16499, -16586, -16673, -16759, -16846, -16932,
    -17018, -17104, -17189, -17275, -17360, -17445, -17530, -17615, -17700, -17784,
    -17869, -17953, -18037, -18121, -18204, -18288, -18371, -18454, -18537, -18620,
    -18703, -18785, -18868, -18950, -19032, -19113, -19195, -19276, -19357, -19438,
    -19519, -19600, -19680, -19761, -19841, -19921, -20000, -20080, -20159, -20238,
    -20317, -20396, -20475, -20553, -20631, -20709, -20787, -20865, -20942, -21019,
    -21096, -21173, -21250, -21326, -21403, -21479, -21554, -21630, -21705, -21781,
    -21856, -21930, -22005, -22079, -22154, -22227, -22301, -22375, -22448, -22521,
    -22594, -22667, -22739, -22812, -22884, -22956, -23027, -23099, -23170, -23241,
    -23311, -23382, -23452, -23522, -23592, -23662, -23731, -23801, -23870, -23938,
    -24007, -24075, -24143, -24211, -24279, -24346, -24413, -24480, -24547, -24613,
    -24680, -24746, -24811, -24877, -24942, -25007, -25072, -25137, -25201, -25265,
    -25329, -25393, -25456, -25519, -25582, -25645, -25708, -25770, -25832, -25893,
    -25955, -26016, -26077, -26138, -26198, -26259, -26319, -26378, -26438, -26497,
    -26556, -26615, -26674, -26732, -26790, -26848, -26905, -26962, -27019, -27076,
    -27133, -27189, -27245, -27300, -27356, -27411, -27466, -27521, -27575, -27629,
    -27683, -27737, -27790, -27843, -27896, -27949, -28001, -28053, -28105, -28157,
    -28208, -28259, -28310, -28360, -28411, -28460, -28510, -28560, -28609, -28658,
    -28706, -28755, -28803, -28850, -28898, -28945, -28992, -29039, -29085, -29131,
    -29177, -29223, -29268, -29313, -29358, -29403, -29447, -29491, -29534, -29578,
    -29621, -29664, -29706, -29749, -29791, -29832, -29874, -29915, -29956, -29997,
    -30037, -30077, -30117, -30156, -30195, -30234, -30273, -30311, -30349, -30387,
    -30424, -30462, -30498, -30535, -30571, -30607, -30643, -30679, -30714, -30749,
    -30783, -30818, -30852, -30885, -30919, -30952, -30985, -31017, -31050, -31082,
    -31113, -31145, -31176, -31206, -31237, -31267, -31297, -31327, -31356, -31385,
    -31414, -31442, -31470, -31498, -31526, -31553, -31580, -31607, -31633, -31659,
    -31685, -31710, -31736, -31760, -31785, -31809, -31833, -31857, -31880, -31903,
    -31926, -31949, -31971, -31993, -32014, -32036, -32057, -32077, -32098, -32118,
    -32137, -32157, -32176, -32195, -32213, -32232, -32250, -32267, -32285, -32302,
    -32318, -32335, -32351, -32367, -32382, -32397, -32412, -32427, -32441, -32455,
    -32469, -32482, -32495, -32508, -32521, -32533, -32545, -32556, -32567, -32578,
    -32589, -32599, -32609, -32619, -32628, -32637, -32646, -32655, -32663, -32671,
    -32678, -32685, -32692, -32699, -32705, -32711, -32717, -32722, -32728, -32732,
    -32737, -32741, -32745, -32748, -32752, -32755, -32757, -32759, -32761, -32763,
    -32765, -32766, -32766, -32767, -32767, -32767, -32766, -32766, -32765, -32763,
    -32761, -32759, -32757, -32755, -32752, -32748, -32745, -32741, -32737, -32732,
    -32728, -32722, -32717, -32711, -32705, -32699, -32692, -32685, -32678, -32671,
    -32663, -32655, -32646, -32637, -32628, -32619, -32609, -32599, -32589, -32578,
    -32567, -32556, -32545, -32533, -32521, -32508, -32495, -32482, -32469, -32455,
    -32441, -32427, -32412, -32397, -32382, -32367, -32351, -32335, -32318, -32302,
    -32285, -32267, -32250, -32232, -32213, -32195, -32176, -32157, -32137, -32118,
    -32098, -32077, -32057, -32036, -32014, -31993, -31971, -31949, -31926, -31903,
    -31880, -31857, -31833, -31809, -31785, -31760, -31736, -31710, -31685, -31659,
    -31633, -31607, -31580, -31553, -31526, -31498, -31470, -31442, -31414, -31385,
    -31356, -31327, -31297, -31267, -31237, -31206, -31176, -31145, -31113, -31082,
    -31050, -31017, -30985, -30952, -30919, -30885, -30852, -30818, -30783, -30749,
    -30714, -30679, -30643, -30607, -30571, -30535, -30498, -30462, -30424, -30387,
    -30349, -30311, -30273, -30234, -30195, -30156, -30117, -30077, -30037, -29997,
    -29956, -29915, -29874, -29832, -29791, -29749, -29706, -29664, -29621, -29578,
    -29534, -29491, -29447, -29403, -29358, -29313, -29268, -29223, -29177, -29131,
    -29085, -29039, -28992, -28945, -28898, -28850, -28803, -28755, -28706, -28658,
    -28609, -28560, -28510, -28460, -28411, -28360, -28310, -28259, -28208, -28157,
    -28105, -28053, -28001, -27949, -27896, -27843, -27790, -27737, -27683, -27629,
    -27575, -27521, -27466, -27411, -27356, -27300, -27245, -27189, -27133, -27076,
    -27019, -26962, -26905, -26848, -26790, -26732, -26674, -26615, -26556, -26497,
    -26438, -26378, -26319, -26259, -26198, -26138, -26077, -26016, -25955, -25893,
    -25832, -25770, -25708, -25645, -25582, -25519, -25456, -25393, -25329, -25265,
    -25201, -25137, -25072, -25007, -24942, -24877, -24811, -24746, -24680, -24613,
    -24547, -24480, -24413, -24346, -24279, -24211, -24143, -24075, -24007, -23938,
    -23870, -23801, -23731, -23662, -23592, -23522, -23452, -23382, -23312, -23241,
    -23170, -23099, -23027, -22956, -22884, -22812, -22739, -22667, -22594, -22521,
    -22448, -22375, -22301, -22227, -22154, -22079, -22005, -21930, -21856, -21781,
    -21705, -21630, -21554, -21479, -21403, -21326, -21250, -21173, -21096, -21019,
    -20942, -20865, -20787, -20709, -20631, -20553, -20475, -20396, -20317, -20238,
    -20159, -20080, -20000, -19921, -19841, -19761, -19680, -19600, -19519, -19438,
    -19357, -19276, -19195, -19113, -19032, -18950, -18868, -18785, -18703, -18620,
    -18537, -18454, -18371, -18288, -18204, -18121, -18037, -17953, -17869, -17784,
    -17700, -17615, -17530, -17445, -17360, -17275, -17189, -17104, -17018, -16932,
    -16846, -16759, -16673, -16586, -16499, -16413, -16325, -16238, -16151, -16063,
    -15976, -15888, -15800, -15712, -15623, -15535, -15446, -15358, -15269, -15180,
    -15090, -15001, -14912, -14822, -14732, -14643, -14553, -14462, -14372, -14282,
    -14191, -14101, -14010, -13919, -13828, -13736, -13645, -13554, -13462, -13370,
    -13279, -13187, -13094, -13002, -12910, -12817, -12725, -12632, -12539, -12446,
    -12353, -12260, -12167, -12074, -11980, -11886, -11793, -11699, -11605, -11511,
    -11417, -11322, -11228, -11133, -11039, -10944, -10849, -10754, -10659, -10564,
    -10469, -10374, -10278, -10183, -10087, -9992, -9896, -9800, -9704, -9608,
    -9512, -9416, -9319, -9223, -9126, -9030, -8933, -8836, -8739, -8642,
    -8545, -8448, -8351, -8254, -8157, -8059, -7962, -7864, -7767, -7669,
    -7571, -7473, -7375, -7277, -7179, -7081, -6983, -6885, -6786, -6688,
    -6590, -6491, -6393, -6294, -6195, -6096, -5998, -5899, -5800, -5701,
    -5602, -5503, -5404, -5305, -5205, -5106, -5007, -4907, -4808, -4708,
    -4609, -4509, -4410, -4310, -4210, -4111, -4011, -3911, -3811, -3712,
    -3612, -3512, -3412, -3312, -3212, -3112, -3012, -2911, -2811, -2711,
    -2611, -2511, -2410, -2310, -2210, -2110, -2009, -1909, -1809, -1708,
    -1608, -1507, -1407, -1307, -1206, -1106, -1005, -905, -804, -704,
    -603, -503, -402, -302, -201, -101, 0, 101, 201, 302,
    402, 503, 603, 704, 804, 905, 1005, 1106, 1206, 1307,
    1407, 1507, 1608, 1708, 1809, 1909, 2009, 2110, 2210, 2310,
    2410, 2511, 2611, 2711, 2811, 2911, 3012, 3112, 3212, 3312,
    3412, 3512, 3612, 3712, 3811, 3911, 4011, 4111, 4210, 4310,
    4410, 4509, 4609, 4708, 4808, 4907, 5007, 5106, 5205, 5305,
    5404, 5503, 5602, 5701, 5800, 5899, 5998, 6096, 6195, 6294,
    6393, 6491, 6590, 6688, 6786, 6885, 6983, 7081, 7179, 7277,
    7375, 7473, 7571, 7669, 7767, 7864, 7962, 8059, 8157, 8254,
    8351, 8448, 8545, 8642, 8739, 8836, 8933, 9030, 9126, 9223,
    9319, 9416, 9512, 9608, 9704, 9800, 9896, 9992, 10087, 10183,
    10278, 10374, 10469, 10564, 10659, 10754, 10849, 10944, 11039, 11133,
    11228, 11322, 11417, 11511, 11605, 11699, 11793, 11886, 11980, 12074,
    12167, 12260, 12353, 12446, 12539, 12632, 12725, 12817, 12910, 13002,
    13094, 13187, 13279, 13370, 13462, 13554, 13645, 13736, 13828, 13919,
    14010, 14101, 14191, 14282, 14372, 14462, 14553, 14643, 14732, 14822,
    14912, 15001, 15090, 15180, 15269, 15358, 15446, 15535, 15623, 15712,
    15800, 15888, 15976, 16063, 16151, 16238, 16325, 16413, 16499, 16586,
    16673, 16759, 16846, 16932, 17018, 17104, 17189, 17275, 17360, 17445,
    17530, 17615, 17700, 17784, 17869, 17953, 18037, 18121, 18204, 18288,
    18371, 18454, 18537, 18620, 18703, 18785, 18868, 18950, 19032, 19113,
    19195, 19276, 19357, 19438, 19519, 19600, 19680, 19761, 19841, 19921,
    20000, 20080, 20159, 20238, 20317, 20396, 20475, 20553, 20631, 20709,
    20787, 20865, 20942, 21019, 21096, 21173, 21250, 21326, 21403, 21479,
    21554, 21630, 21705, 21781, 21856, 21930, 22005, 22079, 22154, 22227,
    22301, 22375, 22448, 22521, 22594, 22667, 22739, 22812, 22884, 22956,
    23027, 23099, 23170, 23241, 23311, 23382, 23452, 23522, 23592, 23662,
    23731, 23801, 23870, 23938, 24007, 24075, 24143, 24211, 24279, 24346,
    24413, 24480, 24547, 24613, 24680, 24746, 24811, 24877, 24942, 25007,
    25072, 25137, 25201, 25265, 25329, 25393, 25456, 25519, 25582, 25645,
    25708, 25770, 25832, 25893, 25955, 26016, 26077, 26138, 26198, 26259,
    26319, 26378, 26438, 26497, 26556, 26615, 26674, 26732, 26790, 26848,
    26905, 26962, 27019, 27076, 27133, 27189, 27245, 27300, 27356, 27411,
    27466, 27521, 27575, 27629, 27683, 27737, 27790, 27843, 27896, 27949,
    28001, 28053, 28105, 28157, 28208, 28259, 28310, 28360, 28411, 28460,
    28510, 28560, 28609, 28658, 28706, 28755, 28803, 28850, 28898, 28945,
    28992, 29039, 29085, 29131, 29177, 29223, 29268, 29313, 29358, 29403,
    29447, 29491, 29534, 29578, 29621, 29664, 29706, 29749, 29791, 29832,
    29874, 29915, 29956, 29997, 30037, 30077, 30117, 30156, 30195, 30234,
    30273, 30311, 30349, 30387, 30424, 30462, 30498, 30535, 30571, 30607,
    30643, 30679, 30714, 30749, 30783, 30818, 30852, 30885, 30919, 30952,
    30985, 31017, 31050, 31082, 31113, 31145, 31176, 31206, 31237, 31267,
    31297, 31327, 31356, 31385, 31414, 31442, 31470, 31498, 31526, 31553,
    31580, 31607, 31633, 31659, 31685, 31710, 31736, 31760, 31785, 31809,
    31833, 31857, 31880, 31903, 31926, 31949, 31971, 31993, 32014, 32036,
    32057, 32077, 32098, 32118, 32137, 32157, 32176, 32195, 32213, 32232,
    32250, 32267, 32285, 32302, 32318, 32335, 32351, 32367, 32382, 32397,
    32412, 32427, 32441, 32455, 32469, 32482, 32495, 32508, 32521, 32533,
    32545, 32556, 32567, 32578, 32589, 32599, 32609, 32619, 32628, 32637,
    32646, 32655, 32663, 32671, 32678, 32685, 32692, 32699, 32705, 32711,
    32717, 32722, 32728, 32732, 32737, 32741, 32745, 32748, 32752, 32755,
    32757, 32759, 32761, 32763, 32765, 32766, 32766, 32767
};

const float sl[ANTENNA_COUNT][CHANNEL_COUNT] =
{
    {0.2014909E+05, 0.2044900E+06, 0.3038668E+06, 0.3502939E+06},
    {0.2014910E+05, 0.2044900E+06, 0.3038668E+06, 0.3502939E+06},
    {0.2014909E+05, 0.2044900E+06, 0.3038668E+06, 0.3502939E+06},
    {0.2014909E+05, 0.2044900E+06, 0.3038668E+06, 0.3502939E+06},
    {0.2014910E+05, 0.2044900E+06, 0.3038668E+06, 0.3502939E+06},
    {0.2014909E+05, 0.2044900E+06, 0.3038668E+06, 0.3502939E+06}
};

const char rever[ANTENNA_COUNT] = {0, 1, 1, 1, 0, 0};

const float antsh[ANTENNA_COUNT] = {-16306.79, -160796.7, 16306.79,
    16306.79, 160796.7, -16306.79};


//==========================
// BinningConstants methods 
//==========================

BinningConstants::BinningConstants()
:   checksum(0)
{
    return;
}

BinningConstants::~BinningConstants()
{
    return;
}

//----------
// IsFrozen 
//----------
// returns true if the binning constants are frozen (non propagating)

int
BinningConstants::IsFrozen()
{
    for (int i = 0; i < ANTENNA_COUNT; i++)
    {
        for (int j = 0; j < CELL_COUNT; j++)
        {
            if (a1[i][j] != 0 || a2[i][j] != 0)
                return(0);
        }
    }
    return(1);
}


//---------
// Convert 
//---------
// convert binning constants to binning parameters for a given orbit step
// returns 1 if ok, 0 if regressive

#define SHORT_ROUNDER   0x8000
#define MAX_SHORT       0x7fff

int
BinningConstants::Convert(
    BinningParameters*  bp,
    int                 orbit_step,
    char                fix_regression)
{
/*
    short i;
    short fcc, fcl, fcu, upedge, loedge;
    int step,
    int channel_delta, new_delta;
    int binw, int_start, int_end, max_delta;
*/

    int step_x_2 = (orbit_step << 1) & 0x7FF;

    for (int ant_index = 0; ant_index < ANTENNA_COUNT; ant_index++)
    {
        short cell, channel_index;
        for (cell = 0, channel_index = 0; cell < CELL_COUNT; cell++)
        {
            int cos_1_index = orbit_step + (p1[ant_index][cell] << 3);
            if (cos_1_index < 0)
                cos_1_index += 0x800;
            cos_1_index &= 0x7FF;

            int cos_2_index = step_x_2 + (p2[ant_index][cell] << 3);
            if (cos_2_index < 0)
                cos_2_index += 0x800;
            cos_2_index &= 0x7FF;

            int cos_1_value = cosine_table[cos_1_index];
            int cos_2_value = cosine_table[cos_2_index];

            int int_result = cos_1_value * (int) a1[ant_index][cell];
            short fcc;
            if (int_result >= 0) {
                fcc = (int_result + SHORT_ROUNDER) >> 16;
            }
            else {
                fcc = (int_result + SHORT_ROUNDER - 1) >> 16;
            }

            int_result = cos_2_value * (int)a2[ant_index][cell];
            if (int_result >= 0)
                fcc += (int_result + SHORT_ROUNDER) >> 16;
            else
                fcc += (int_result + SHORT_ROUNDER - 1) >> 16;

            short fcl = fcc - bw2[ant_index][cell];
            short fcu = fcc + bw2[ant_index][cell];

            fcl += (fcl >= 0) ? 2 : 1;
            fcl = a0[ant_index][cell] + (fcl >> 2);
            fcu += (fcu >= 0) ? 2 : 1;
            fcu = a0[ant_index][cell] + (fcu >> 2);

            // now compute center from upper and lower
            fcc = (short) (((int) fcl + (int) fcu + (int) 1) >> 1);

            int channel_delta = MAX_SHORT;
            int new_delta;
            int i;
            for (i = channel_index; i < CHANNEL_COUNT; i++)
            {
                if ((fcl >= fhub[i]) && (fcu <= fhut[i]))
                {
                    new_delta = fcc - fhc[i];
                    if (new_delta < 0)
                        new_delta = 0 - new_delta;

                    new_delta = ((new_delta * fhpbw[i]) +
                        SHORT_ROUNDER) >> 16;
                    if (new_delta < channel_delta)
                    {
                        channel_index = i;
                        channel_delta = new_delta;
                    }
                    else if (channel_delta < MAX_SHORT)
                        break;
                }
                else if (channel_delta < MAX_SHORT)
                    break;
            }

            if (channel_delta == MAX_SHORT)
            {
                // cell not on flat top of any channel

                for (i = channel_index; i < CHANNEL_COUNT; i++)
                {
                    if ((fcl >= fhl[i]) && (fcu <= fhh[i]))
                    {
                        new_delta = fcc - fhc[i];
                        if (new_delta < 0)
                            new_delta = 0 - new_delta;

                        new_delta = ((new_delta * fhsbw[i]) +
                        SHORT_ROUNDER) >> 16;
                        if (new_delta < channel_delta)
                        {
                            channel_index = i;
                            channel_delta = new_delta;
                        }
                        else if (channel_delta < MAX_SHORT)
                            break;
                    }
                    else if (channel_delta < MAX_SHORT)
                        break;
                }

                int max_delta = 0;
                if (channel_delta == MAX_SHORT)
                {
                    for (i = channel_index; i < CHANNEL_COUNT; i++)
                    {
                        short upedge = fhh[i];
                        short loedge = fhl[i];
                        if (fcl > loedge)
                            loedge = fcl;
                        if (fcu < upedge)
                            upedge = fcu;
                        new_delta = upedge - loedge;
                        if (new_delta > 0)
                        {
                            if (new_delta > max_delta)
                            {
                                channel_index = i;
                                max_delta = new_delta;
                                fcu = upedge;
                                fcl = loedge;
                            }
                            else
                                break;
                        }
                        else if (max_delta > 0)
                            break;
                    }
                }
            }

            int binw = fhbinw[channel_index];
            bp->channel[ant_index][cell] = channel_index + 1;

            int int_start = (((fcl - fhl[channel_index]) * binw) +
                SHORT_ROUNDER) >> 16;
            if (int_start < 0)
                int_start = 0;
            if (int_start > 255)
                int_start = 255;
            bp->startBin[ant_index][cell] = int_start & 0xFF;
            if (fix_regression == 1 && cell > 0)
            {
                if (bp->startBin[ant_index][cell] <=
                    bp->endBin[ant_index][cell - 1] &&
                    bp->channel[ant_index][cell] ==
                    bp->channel[ant_index][cell - 1])
                {
                    bp->startBin[ant_index][cell] =
                        bp->endBin[ant_index][cell - 1] + 1;
                }
            }

            int int_end = (((fcu - fhl[channel_index]) * binw) +
                SHORT_ROUNDER) >> 16;
            if (int_end < 0)
                int_end = 0;
            if (int_end > 255)
                int_end = 255;
            bp->endBin[ant_index][cell] = int_end;
            bp->startFreq[ant_index][cell] = BinToFreq(ant_index, channel_index,
                bp->startBin[ant_index][cell]);
            bp->endFreq[ant_index][cell] = BinToFreq(ant_index, channel_index,
                bp->endBin[ant_index][cell]);
            bp->cellSize[ant_index][cell] =
                bp->endBin[ant_index][cell] -
                bp->startBin[ant_index][cell] + 1;
            bp->cellBw[ant_index][cell] =
                bp->cellSize[ant_index][cell] * bin_width[channel_index];
        }
    }
    return 1;
}

//-----------
// BinToFreq 
//-----------
// convert a bin to a frequency

float
BinningConstants::BinToFreq(
    int     antenna_index,
    int     channel_index,
    int     bin)
{
    float fout = bin * bin_width[channel_index] +
        sl[antenna_index][channel_index];
    if (rever[antenna_index] == 1)
        fout = -fout;
    fout -= antsh[antenna_index];
    return(fout);
}

//-----------------
// OrbitTimeToStep 
//-----------------

int
BinningConstants::OrbitTimeToStep(
    unsigned int    orbit_time)
{
    return ((int) (orbit_time / period) & 0x7ff);
}

//------------
// ReadBinary 
//------------
// 1 on succes, 0 on failure

int
BinningConstants::ReadBinary(
    const char*     filename)
{
    int ifd = open(filename, O_RDONLY);
    if (ifd == -1)
        return(0);
    if (read(ifd, (char *)this, BINNING_CONSTANTS_SIZE) !=
        BINNING_CONSTANTS_SIZE)
    {
        return(0);
    }
    close(ifd);
    return(1);
}

//---------
// ReadNml 
//---------

#define NML_BUFFER_SIZE     10240

int
BinningConstants::ReadNml(
    const char*     filename)
{
    //-------------------
    // open the nml file 
    //-------------------

    int ifd = open(filename, O_RDONLY);
    if (ifd == -1)
        return(0);

    //------------------------------------------------
    // allocate a giant buffer to hold the whole file 
    //------------------------------------------------

    int size = lseek(ifd, 0, SEEK_END);
    lseek(ifd, 0, SEEK_SET);
    if (size > NML_BUFFER_SIZE)
    {
        close(ifd);
        return(0);
    }
    char buffer[NML_BUFFER_SIZE];

    //------------------------
    // read in the whole file 
    //------------------------

    if (read(ifd, buffer, size) != size)
    {
        close(ifd);
        return(0);
    }
    close(ifd);

    //---------------------------------
    // search for variables, then read 
    //---------------------------------

    int a0_read = 0;
    int a1_read = 0;
    int a2_read = 0;
    int p1_read = 0;
    int p2_read = 0;
    int bw2_read = 0;
    int period_read = 0;

    char variable[1024];
    int index = 0;
    while (_FindVariable(buffer, size, variable, &index))
    {
        int antenna;
        int cell;
        unsigned short ushort_num;
        short short_num;
        int retval;
        int add_index;
        if (strstr(variable, "a0") || strstr(variable, "A0"))
        {
            // read the a0 terms
            for (antenna = 0; antenna < ANTENNA_COUNT; antenna++)
            {
                for (cell = 0; cell < CELL_COUNT; cell++)
                {
                    if (sscanf(buffer + index, " %hd,%n", &ushort_num,
                        &add_index) != 1)
                        return(0);
                    a0[antenna][cell] = ushort_num;
                    index += add_index;
                }
            }
            a0_read = 1;
        }
        else if (strstr(variable, "a1") || strstr(variable, "A1"))
        {
            // read the a1 terms
            for (antenna = 0; antenna < ANTENNA_COUNT; antenna++)
            {
                for (cell = 0; cell < CELL_COUNT; cell++)
                {
                    if (sscanf(buffer + index, " %hd,%n", &short_num,
                        &add_index) != 1)
                        return(0);
                    a1[antenna][cell] = short_num;
                    index += add_index;
                }
            }
            a1_read = 1;
        }
        else if (strstr(variable, "a2") || strstr(variable, "A2"))
        {
            // read the a2 terms
            for (antenna = 0; antenna < ANTENNA_COUNT; antenna++)
            {
                for (cell = 0; cell < CELL_COUNT; cell++)
                {
                    if (sscanf(buffer + index, " %hd,%n", &short_num,
                        &add_index) != 1)
                        return(0);
                    a2[antenna][cell] = short_num;
                    index += add_index;
                }
            }
            a2_read = 1;
        }
        else if (strstr(variable, "p1") || strstr(variable, "P1"))
        {
            // read the p1 terms
            for (antenna = 0; antenna < ANTENNA_COUNT; antenna++)
            {
                for (cell = 0; cell < CELL_COUNT; cell++)
                {
                    retval = sscanf(buffer + index, " %hd,%n", &short_num,
                        &add_index);
                    p1[antenna][cell] = (unsigned char)short_num;
                    index += add_index;
                    if (retval != 1 || p1[antenna][cell] != short_num)
                        return(0);
                }
            }
            p1_read = 1;
        }
        else if (strstr(variable, "p2") || strstr(variable, "P2"))
        {
            // read the p2 terms
            for (antenna = 0; antenna < ANTENNA_COUNT; antenna++)
            {
                for (cell = 0; cell < CELL_COUNT; cell++)
                {
                    retval = sscanf(buffer + index, " %hd,%n", &short_num,
                        &add_index);
                    p2[antenna][cell] = (unsigned char)short_num;
                    index += add_index;
                    if (retval != 1 || p2[antenna][cell] != short_num)
                        return(0);
                }
            }
            p2_read = 1;
        }
        else if (strstr(variable, "band") || strstr(variable, "BAND") ||
            strstr(variable, "bw") || strstr(variable, "BW"))
        {
            // read the bw2 terms
            for (antenna = 0; antenna < ANTENNA_COUNT; antenna++)
            {
                for (cell = 0; cell < CELL_COUNT; cell++)
                {
                    if (sscanf(buffer + index, " %hd,%n", &ushort_num,
                        &add_index) != 1)
                        return(0);
                    bw2[antenna][cell] = ushort_num;
                    index += add_index;
                }
            }
            bw2_read = 1;
        }
        else if (strstr(variable, "orbit") || strstr(variable, "step") ||
            strstr(variable, "period"))
        {
            // read the period
            if (sscanf(buffer + index, " %hd", &ushort_num) != 1)
                return(0);
            period = ushort_num;
            period_read = 1;
        }
    }
    checksum = 0;

    if (! a0_read || ! a1_read || ! a2_read || ! p1_read || ! p2_read ||
        ! bw2_read || ! period_read)
    {
        return(0);
    }

    return(1);
}

//-------
// Array 
//-------
// transfers the binning constants (in command order) into a single array
// this is where the byte swapping is done

int
BinningConstants::Array(
    char*   bc_array)
{
    for (int antenna = 0; antenna < ANTENNA_COUNT; antenna++)
    {
        for (int cell = 0; cell < CELL_COUNT; cell++)
        {
            memcpy(bc_array + A0_OFFSET + (antenna*CELL_COUNT + cell) * 2,
                &(a0[antenna][cell]), 2);
            memcpy(bc_array + A1_OFFSET + (antenna*CELL_COUNT + cell) * 2,
                &(a1[antenna][cell]), 2);
            memcpy(bc_array + A2_OFFSET + (antenna*CELL_COUNT + cell) * 2,
                &(a2[antenna][cell]), 2);
            *(bc_array + P1_OFFSET + antenna*CELL_COUNT + cell) =
                p1[antenna][cell];
            *(bc_array + P2_OFFSET + antenna*CELL_COUNT + cell) =
                p2[antenna][cell];
            memcpy(bc_array + BW2_OFFSET + (antenna*CELL_COUNT + cell) * 2,
                &(bw2[antenna][cell]), 2);
        }
    }
    memcpy(bc_array + PERIOD_OFFSET, &period, 2);

    // byte swap
    for (int i = P1_OFFSET; i < BW2_OFFSET; i += 2)
    {
        char tmp = *(bc_array + i);
        *(bc_array + i) = *(bc_array + i + 1);
        *(bc_array + i + 1) = tmp;
    }
    return(1);
}

//---------------
// _FindVariable 
//---------------
// used by ReadNml to find the next variable name
// an '=' is searched for and the string before the '=' is
// copied into variable
// the index is set to be passed the '='

int
BinningConstants::_FindVariable(
    char    buffer[],
    int     size,
    char*   variable,
    int*    index)
{
    while (*index < size)
    {
        if (buffer[*index] == '=')
        {
            // back up before any whitespace
            int var_index = (*index) - 1;
            (*index)++;     // skip past the '='
            while (var_index >= 0 && isspace(buffer[var_index]))
                var_index--;

            // back up before variable name
            while (var_index >= 0 && ! isspace(buffer[var_index]))
                var_index--;

            // read the variable name
            if (sscanf(buffer + var_index, "%s", variable) != 1)
                return(0);

            // wipe out any '=' sign
            char* ptr = strchr(variable, '=');
            if (ptr)
                *ptr = '\0';

            return(1);
        }
        (*index)++;
    }
    return(0);
}

//--------------
// CheckForCmds 
//--------------
// returns 1 if any embedded commands are found (sets the bytes, bits,
// command code, and command name)
// assumes that the first two bytes are the command

int
CheckForCmds(
    char*           array,
    int             array_size,
    int*            byte1,
    int*            bit1,
    int*            byte2,
    int*            bit2,
    unsigned short* cmd_code,
    char**          cmd_name)
{
    static const struct {
        unsigned short  cmd_code;
        char*           cmd_name;
    } cmd_info[] =
    {
        {0xdfa0, "Standby Mode" },
        {0x5e21, "Receive Only Mode" },
        {0x5d22, "Continuous Calibrate Mode" },
        {0xdca3, "Debug Mode" },
        {0x5b24, "Wind Observation Mode" },
        {0xdaa5, "K9 Set (TWTA)" },
        {0xd9a6, "K9 Reset (TWTA)" },
        {0x5827, "TWTA Body Overcurrent Trip Override" },
        {0x5728, "K10 Set (TWTA)" },
        {0xd6a9, "K10 Reset (TWTA)" },
        {0xd5aa, "K11 Set (HVPS)" },
        {0x542b, "K11 Reset (HVPS)" },
        {0xd3ac, "K12 Set (HVPS)" },
        {0x522d, "K12 Reset (HVPS)" },
        {0x512e, "Binning Constants Load" },
        {0xd0af, "Antenna Sequence Update" },
        {0x4f30, "WTS Position 1" },
        {0x403f, "WTS Position 2" },
        {0x6e11, "RFS Trip Monitor" },
        {0x0000, "None" }
    };

    // initialize with the command
    unsigned short word = *(array) << 8 | *(array + 1);
    for (int byte = 2; byte < array_size; byte++)
    {
        for (int bit = 7; bit >= 0; bit--)
        {
            // shift in the next bit
            word <<= 1;
            word |= (*(array+byte) >> bit) & 0x01;

            // check against command codes
            for (int cmd = 0; cmd_info[cmd].cmd_code; cmd++)
            {
                if (word == cmd_info[cmd].cmd_code)
                {
                    *byte2 = byte;
                    *bit2 = bit;
                    if (bit == 0)
                    {
                        *byte1 = byte - 1;
                        *bit1 = 7;
                    }
                    else
                    {
                        *byte1 = byte - 2;
                        *bit1 = bit - 1;
                    }
                    *cmd_code = cmd_info[cmd].cmd_code;
                    *cmd_name = cmd_info[cmd].cmd_name;
                    return(1);
                }
            }
        }
    }
    return(0);
}
