//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.35   09 Apr 1999 13:57:54   sally
// read number of words from files if the table length is variable
// 
//    Rev 1.34   25 Mar 1999 14:06:16   daffer
// Incorporated Uploadable table support
// 
//    Rev 1.33   29 Jan 1999 15:04:04   sally
// added LASP proc commands
// 
//    Rev 1.32   08 Jan 1999 16:37:12   sally
// add LASP proc commands
// 
//    Rev 1.31   09 Nov 1998 11:18:48   sally
// in GetFileFormat(), _numStaticParams <= 3 is FILE_REQQ_STATIC
// .
// 
//    Rev 1.30   20 Oct 1998 15:46:58   sally
// change some relay commands to be realtime only
// 
//    Rev 1.29   20 Oct 1998 10:52:34   sally
// add static QPF commands (table macro commands)
// 
//    Rev 1.28   08 Oct 1998 16:18:34   sally
// add REQQ like macro commands
// 
//    Rev 1.27   01 Sep 1998 11:15:06   daffer
// Added ReadShort and made other changes to accomodate new PBI commanding 
// 
//    Rev 1.26   01 Sep 1998 10:03:26   sally
// separate CmdHex from CmdId
// 
//    Rev 1.25   31 Aug 1998 14:41:26   sally
// added REQQ-like PBI commands
// 
//    Rev 1.24   28 Aug 1998 11:19:34   sally
// add more formats for REQI datafiles
// 
//    Rev 1.23   21 Aug 1998 15:01:54   daffer
// Changed EA_CMD_NONE to EA_CMD_UNKNOWN in _ReadCmdId
// 
//    Rev 1.22   19 Aug 1998 14:27:48   sally
// add 3 commands - SCANXA1, SCANXA2 and SCORBSYNC
// 
//    Rev 1.21   19 Aug 1998 13:06:58   daffer
// Added elements to effect_mnemonics table, added _ReadCmdId and _WriteCmdId
// added CmdParams member.
// 
//    Rev 1.20   13 Jul 1998 14:21:32   sally
// add the datafile format for SCGATEWID
// 
//    Rev 1.19   09 Jul 1998 16:22:42   sally
// catch the output of process_reqi in an output file
// 
//    Rev 1.18   01 Jul 1998 16:51:30   sally
// added table upload repetition
// 
//    Rev 1.17   29 Jun 1998 16:51:38   sally
// added embedded commands checking
// > ^D
// 
//    Rev 1.16   05 Jun 1998 09:16:54   daffer
// worked on method 'WriteForHumans'
// 
//    Rev 1.15   01 Jun 1998 10:59:00   sally
// add polynomial table to limit checking
// 
//    Rev 1.14   29 May 1998 15:26:10   sally
// adapted to the new REQI datafile format, per Lee's memo
// 
//    Rev 1.13   28 May 1998 11:05:12   sally
// re-order the commands and fixed some typoes
// 
//    Rev 1.12   28 May 1998 09:26:00   sally
// update the formats for REQQ, QPF and RTCF
// 
//    Rev 1.11   26 May 1998 16:36:14   sally
// fixed typo
// 
//    Rev 1.10   22 May 1998 16:35:36   daffer
// Added/modified code to do cmdlp/effect processing.
// Updated commanding and effect mnemonics/enums.
// 
//    Rev 1.9   27 Mar 1998 09:57:24   sally
// added L1A Derived data
// 
//    Rev 1.8   24 Mar 1998 09:15:56   sally
// de-warn for GNU C++
// 
//    Rev 1.7   17 Mar 1998 14:41:04   sally
// changed for REQQ
// 
//    Rev 1.6   16 Mar 1998 13:46:16   sally
// fix some parsing errors
// 
//    Rev 1.5   16 Mar 1998 10:52:16   sally
// ReadReqi() are split into two methods
// 
//    Rev 1.4   12 Mar 1998 17:15:46   sally
// adapt to the new QSCAT REQI format
// 
//    Rev 1.3   09 Mar 1998 16:34:12   sally
// adapt to the new REQI format
// 
//    Rev 1.2   26 Feb 1998 09:54:32   sally
// to pacify GNU C++ compiler
// 
//    Rev 1.1   20 Feb 1998 10:55:58   sally
// L1 to L1A
// 
//    Rev 1.0   04 Feb 1998 14:14:56   daffer
// Initial checking
// Revision 1.3  1998/02/03 00:12:33  sally
// change NRT to L1AP
//
// Revision 1.2  1998/01/30 22:28:56  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================
 
static const char rcs_id_Command_C[] =
    "@(#) $Header$";

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "Command.h"
#include "MacroCommand.h"
#include "Tpg.h"

const EACommandArgsEntry Command::cmdArgsTable[] =
{
  //------------------------------------
  // Pulse Discrete Commands
  //------------------------------------
  { EA_CMD_SCK1RST, 0X000E, "SCK1RST", "K1 Reset, Electronics On",
                                      1, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK2RST, 0X0010, "SCK2RST", "K2 Reset, Electronics On",
                                      1, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK3RST, 0X0012, "SCK3RST", "K3 Reset, Electronics On",
                                      1, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK1SET, 0X000D, "SCK1SET", "K1 Set, Replacement Heaters On",
                                      1, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK2SET, 0X000F, "SCK2SET", "K2 Set, Replacement Heaters On",
                                      1, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK3SET, 0X0011, "SCK3SET", "K3 Set, Replacement Heaters On",
                                      1, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK4RST, 0X0014, "SCK4RST", "K4 Reset, Electronics On",
                                      1, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK5RST, 0X0016, "SCK5RST", "K5 Reset, Electronics On",
                                      1, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK6RST, 0X0018, "SCK6RST", "K6 Reset, Electronics On",
                                      1, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK4SET, 0X0013, "SCK4SET", "K4 Set, Replacement Heaters On",
                                      1, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK5SET, 0X0015, "SCK5SET", "K5 Set, Replacement Heaters On",
                                      1, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK6SET, 0X0017, "SCK6SET", "K6 Set, Replacement Heaters On",
                                      1, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK7RST, 0X0004, "SCK7RST", "K7 Reset, CDS-B Select",
                                      1, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK7SET, 0X0001, "SCK7SET", "K7 Set, CDS-A Select",
                                      1, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK8RST, 0X000A, "SCK8RST", "K8 Reset, CDS-B Select",
                                      1, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK8SET, 0X0007, "SCK8SET", "K8 Set, CDS-A Select",
                                      1, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK13RST,0X0005,"SCK13RST", "K13 Reset, SES Replacement Headers On",
                                      1, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK13SET, 0X0002, "SCK13SET", "K13 Set, ses Electronics On",
                                      1, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK14RST, 0X000B, "SCK14RST", "K14 Reset, SES Replacement Heater On",
                                      1, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK14SET, 0X0008, "SCK14SET", "K14 Set, SES Electronics On",
                                      1, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK17RST, 0X0006, "SCK17RST", "K17 Reset, SAS Replacement Heater On",
                                      1, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK17SET, 0X0003, "SCK17SET", "K17 Set, SAS Electronics On",
                                      1, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK18RST, 0X000C, "SCK18RST", "K18 Reset, SAS Replacement Heater On",
                                      1, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK18SET, 0X0009, "SCK18SET", "K18 Set, SAS Electronics On",
                                      1, 0, 0, EA_DATAFILE_NONE, -1, "" },

  //------------------------------------
  // Serial Magnitude Commands
  //------------------------------------
  { EA_CMD_SCMODSTB, 0XF070, "SCMODSTB", "Standby Mode",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCMODCAL, 0XF007, "SCMODCAL", "Calibration Mode",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCMODRCV, 0XF0E0, "SCMODRCV", "Receive Only Mode",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCMODWOM, 0XF00E, "SCMODWOM", "Wind Observation Mode",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCCDSRST, 0XF093, "SCCDSRST", "CDS System Reset",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCSESRST, 0X0F13, "SCSESRST", "SES Reset",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },

  { EA_CMD_SCK9RST, 0XAA01, "SCK9RST",  "K9 Reset, TWTA Power/On/Off (Pri)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK9SET, 0XAAC1, "SCK9SET" , "K9 Set, TWTA Power/On/Off (Pri)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK10RST, 0XAA02, "SCK10RST", "K10 Reset, TWTA Pwr On/Off (Sec)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK10SET, 0XAAC2, "SCK10SET", "K10 Set, TWTA Pwr On/Off (Sec)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK11RST, 0XAA04, "SCK11RST", "K11 Reset, TWTA 1/2 Select (Pri)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK11SET, 0XAAC4, "SCK11SET", "K11 Set, TWTA 1/2 Select (Pri)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK12RST, 0XAA08, "SCK12RST", "K12 Reset, TWTA 1/2 Select (Sec)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK12SET, 0XAAC8, "SCK12SET", "K12 Set, TWTA 1/2 Select (Sec)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK15RST, 0XAA10, "SCK15RST", "K15 Reset, SES A/B Select (Pri)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK15SET, 0XAAD0, "SCK15SET", "K15 Set, SES A/B Select (Pri)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK16RST, 0XAA20, "SCK16RST", "K16 Reset, SES A/B Select (Sec)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK16SET, 0XAAE0, "SCK16SET", "K16 Set, SES A/B Select (Sec)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK19RST, 0XAA07, "SCK19RST", "K19 Reset, SAS A/B Select (Pri)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK19SET, 0XAAC7, "SCK19SET", "K19 Set, SAS A/B Select (Pri)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK20RST, 0XAA0E, "SCK20RST", "K20 Reset, SAS A/B Select (Sec)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK20SET, 0XAACE, "SCK20SET", "K20 Set, SAS A/B Select (Sec)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK21RST, 0XAA19, "SCK21RST", "K21 Reset, SES Supp Htr On/Off (Pri)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK21SET, 0XAAD9, "SCK21SET", "K21 Set, SES Supp Htr On/Off (Pri)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK22RST, 0XAA31, "SCK22RST", "K22 Reset, SES Supp Htr On/Off (Sec)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK22SET, 0XAAF1, "SCK22SET", "K22 Set, SES Supp Htr On/Off (Sec)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK23RST, 0XAA1C, "SCK23RST", "K23 Reset (Spare)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK23SET, 0XAADC, "SCK23SET", "K23 Set (Spare)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK24RST, 0XAA38, "SCK24RST", "K24 Reset (Spare)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK24SET, 0XAAF7, "SCK24SET", "K24 Set (Spare)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK25RST, 0XAA0B, "SCK25RST", "K25 Reset (Spare)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK25SET, 0XAACB, "SCK25SET", "K25 Set (Spare)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK26RST, 0XAA13, "SCK26RST", "K26 Reset (Spare)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCK26SET, 0XAAD3, "SCK26SET", "K26 Set (Spare)",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCSSARST, 0XAA23, "SCSSARST", "Select SES-A 18 RPM",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCSSASET, 0XAAE3, "SCSSASET", "Select SAS-A 19.8 RPM",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCSSBRST, 0XAA0D, "SCSSBRST", "Select SAS-B 18 RPM",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCSSBSET, 0XAACD, "SCSSBSET", "Select SAS-B 19.8 RPM",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },

  { EA_CMD_SCPRFCLK, 0X0F07, "SCPRFCLK", "PRF Clock Rate",
                                  0, 0, 1, EA_DATAFILE_UDEC2_ASCII, -1, "" },
  { EA_CMD_SCRGAWID, 0X0F03, "SCRGAWID","Range Gate A Width",
                                  0, 0, 1, EA_DATAFILE_UDEC2_ASCII, -1, "" },
  { EA_CMD_SCRGBWID, 0X0F05, "SCRGBWID","Range Gate B Width",
                                  0, 0, 1, EA_DATAFILE_UDEC2_ASCII, -1, "" },
  { EA_CMD_SCTRPWID, 0X0F06, "SCTRPWID", "Transmit Pulse Width",
                                  0, 0, 1, EA_DATAFILE_UDEC2_ASCII, -1, "" },
  { EA_CMD_SCGATEWID, 0X0F52, "SCGATEWID","RX Gate/Xmit Pulse Widths",
                                  0, 0, 2, EA_DATAFILE_2UDEC2_ASCII, -1, "" },
  { EA_CMD_SCRCVGAN, 0X0F08, "SCRCVGAN", "Receiver Gain",
                                  0, 0, 1, EA_DATAFILE_UDEC2_ASCII },
  { EA_CMD_SCCALPER, 0X0F40, "SCCALPER", "Calibration Sequence Pulse Period",
                                  0, 0, 1, EA_DATAFILE_UDEC2_ASCII, -1, "" },

  { EA_CMD_SCGRDNOR, 0X0F0A, "SCGRDNOR", "TWT Grid Normal",
                                  0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCGRDDS, 0X0F0B, "SCGRDDS" , "TWT Grid Disable",
                                  0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCRCVPON, 0X0F0C, "SCRCVPON", "Receive Protect On",
                                  0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCRCVPNR, 0X0F0D, "SCRCVPNR", "Receive Protect Normal",
                                  0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCTOVRON, 0X0F0E, "SCTOVRON", "TWT Trip Override On",
                                  0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCTOVROFF, 0X0F0F, "SCTOVROF", "TWT Trip Override Off",
                                  0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCTWTMDS, 0X0F3D, "SCTWTMDS", "TWTA Trip Monitor Disable",
                                  0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCTWTMEN, 0X0FFD, "SCTWTMEN", "TWTA Trip Monitor Enable",
                                  0, 0, 1, EA_DATAFILE_UDEC2_ASCII, -1, "" },
  { EA_CMD_SCTMDON, 0X0F10, "SCTMDON", "TWT Modulation On",
                                  0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCTMDOFF, 0X0F11, "SCTMDOFF", "TWT Modulation Off",
                                  0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCTLDFPEN, 0X0F01, "SCTLDFPEN", "TWTA Low Drive FP Enable",
                                  0, 0, 1, EA_DATAFILE_UDEC2_ASCII, -1, "" },
  { EA_CMD_SCTLDFPDS, 0X0F02, "SCTLDFPDS", "TWTA Low Drive FP Disable",
                                  0, 0, 0, EA_DATAFILE_NONE, -1, "" },

  { EA_CMD_SCRSRLPR, 0X0F41, "SCRSRLPR", "SES Reset/Reload Sequence Period",
                                  0, 0, 1, EA_DATAFILE_UDEC2_ASCII, -1, "" },
  { EA_CMD_SCITMSYN, 0X0FA1, "SCITMSYN", "Instrument Time Sync Interval",
                                  0, 0, 1, EA_DATAFILE_UDEC2_ASCII, -1, "" },
  { EA_CMD_SCTWTSEL, 0X0F12, "SCTWTSEL", "TWTA T/R 1 or 2 Select",
                                  0, 0, 1, EA_DATAFILE_UDEC2_ASCII, -1, "" },
  { EA_CMD_SCORBTIC, 0X0FA0, "SCORBTIC", "Timer Ticks per Orbit",
                                  0, 0, 2, EA_DATAFILE_UDEC4_TO_2HEX2, -1, "" },

  { EA_CMD_SCSMLEN, 0X0FFA, "SCSMLEN", "SES Multiseq DLE Response Enable",
                                  0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCSMLDS, 0X0F3A, "SCSMLDS", "SES Multiseq DLE Response Disable",
                                  0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCHTREN, 0X0FFB, "SCHTREN", "SES Suppl Htr Mode Change Cntl Enable",
                                  0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCHTRDS, 0X0F3B, "SCHTRDS", "SES Suppl Htr Mode Change Cntl Disable",
                                  0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCMSLEN, 0X0FD2, "SCMSLEN", "SAS Multiseq DLE Response Enable",
                                  0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCMSLDS, 0X0FD3, "SCMSLDS", "SAS Multiseq DLE Response Disable",
                                  0, 0, 0, EA_DATAFILE_NONE, -1, "" },

  { EA_CMD_SCPSTTBL, 0X55FA, "SCPSTTBL", "Standby PRF Table Update",
                                  1, 1, -1, EA_DATAFILE_HEX2_ASCII, -1, "" },
  { EA_CMD_SCPCLTBL, 0X55FB, "SCPCLTBL", "Calibration PRF Table Update",
                                  1, 1, -1, EA_DATAFILE_HEX2_ASCII, -1, "" },
  { EA_CMD_SCPRCTBL, 0X55FC, "SCPRCTBL", "Receive Only PRF Table Update",
                                  1, 1, -1, EA_DATAFILE_HEX2_ASCII, -1, "" },
  { EA_CMD_SCPWOTBL, 0X55FD, "SCPWOTBL", "Wind Observation PRF Table Update",
                                  1, 1, -1, EA_DATAFILE_HEX2_ASCII, -1, "" },
  { EA_CMD_SCSSTTBL, 0X55CA, "SCSSTTBL", "Standby SES Param Tbl Update",
                                  1, 1, 6, EA_DATAFILE_HEX2_ASCII, -1, "" },
  { EA_CMD_SCSCLTBL, 0X55CB, "SCSCLTBL", "Cal SES Param Tbl Update",
                                  1, 1, 6, EA_DATAFILE_HEX2_ASCII, -1, "" },
  { EA_CMD_SCSRCTBL, 0X55CC, "SCSRCTBL", "Recv Only SES Param Tbl Update",
                                  1, 1, 6, EA_DATAFILE_HEX2_ASCII, -1, "" },
  { EA_CMD_SCSWOTBL, 0X55CD, "SCSWOTBL", "Wind Obs SES Param Tbl Update",
                                  1, 1, 6, EA_DATAFILE_HEX2_ASCII, -1, "" },
  { EA_CMD_SCSALLTBL, 0X55CE, "SCSALLTBL", "All Modes SES Param Tbl Update",
                                  1, 1, 12, EA_DATAFILE_HEX2_ASCII, -1, "" },
  { EA_CMD_SCENGTBL, 0X55F8, "SCENGTBL", "Serial Dig Eng TLM Table Update",
                                  1, 1, 20, EA_DATAFILE_HEX2_ASCII, -1, "" },
  { EA_CMD_SCSTATBL, 0X55F9, "SCSTATBL", "Serial Dig Status TLM Table Update",
                                  1, 1, 8, EA_DATAFILE_HEX2_ASCII, -1, "" },
  { EA_CMD_SCBDATBL, 0X55F2, "SCBDATBL", "Doppler Tracking Beam A Tbl Update",
                                  1, 1, 786, EA_DATAFILE_HEX2_ASCII, -1, "" },
  { EA_CMD_SCBDBTBL, 0X55F3, "SCBDBTBL", "Doppler Tracking Beam B Tbl Update",
                                  1, 1, 786, EA_DATAFILE_HEX2_ASCII, -1, "" },
  { EA_CMD_SCBRATBL, 0X55E6, "SCBRATBL", "Range Tracking Beam A Tbl Update",
                                  1, 1, 402, EA_DATAFILE_HEX2_ASCII, -1, "" },
  { EA_CMD_SCBRBTBL, 0X55E7, "SCBRBTBL", "Range Tracking Beam B Tbl Update",
                                  1, 1, 402, EA_DATAFILE_HEX2_ASCII, -1, "" },

  { EA_CMD_SCPSTSW, 0X0FEA, "SCPSTSW" , "Standby PRF Table Switch",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCPCLSW, 0X0FEB, "SCPCLSW" , "Calibration PRF Table Switch",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCPRCSW, 0X0FEC, "SCPRCSW" , "Receive Only PRF Table Switch",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCPWOSW, 0X0FED, "SCPWOSW" , "Wind Obs PRF Table Switch",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCSSTSW, 0X0FBA, "SCSSTSW" , "Standby SES Param Tbl Switch",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCSCLSW, 0X0FBB, "SCSCLSW" , "Calib SES Param Tbl Switch",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCSWOSW, 0X0FBD, "SCSWOSW" , "Wind Obs SES Param Tbl Switch",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCSRCSW, 0X0FBF, "SCSRCSW" , "Recv Only SES Param Tbl Switch",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCSALLSW, 0X0FBE, "SCSALLSW", "All Modes SES Param Tbl Switch",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCENGSW, 0X0FE8, "SCENGSW" , "Serial Dig Eng TLM Table Switch",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCSTASW, 0X0FE9, "SCSTASW", "Serial Dig Status TLM Table Switch",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCBDASW, 0X0FE2, "SCBDASW", "Doppler Tracking Beam A Table Switch",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCBDBSW, 0X0FE3, "SCBDBSW", "Doppler Tracking Beam B Table Switch",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCBRASW, 0X0FD6, "SCBRASW", "Range Tracking Beam A Table Switch",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCBRBSW, 0X0FD7, "SCBRBSW", "Range Tracking Beam B Table Switch",
                                      0, 0, 0, EA_DATAFILE_NONE, -1, "" },

  { EA_CMD_SCSWPATCH, 0XE3E3, "SCSWPATCH", "CDS Flight Software Patch",
                                  1, 1, -1, EA_DATAFILE_HEX2_ASCII, -1, "" },
  { EA_CMD_SCSWPEN, 0X0F50, "SCSWPEN", "Software Patch Enable",
                                  0, 0, 2, EA_DATAFILE_HEX4_TO_2HEX2, -1, "" },
  { EA_CMD_SCSWPDS, 0X0F51, "SCSWPDS" , "Software Patch Disable",
                                  0, 0, 2, EA_DATAFILE_HEX4_TO_2HEX2, -1, "" },

  { EA_CMD_SCBMOFST, 0X0FD0, "SCBMOFST", "Offsets for Beams A and B",
                                  0, 0, 2, EA_DATAFILE_2UDEC2_ASCII, -1, "" },
  { EA_CMD_SCSASOFS, 0X0FD1, "SCSASOFS", "Offsets for SAS A and B",
                                  0, 0, 2, EA_DATAFILE_2UDEC2_ASCII, -1, "" },
  { EA_CMD_SCTBLSTR, 0X0FC1, "SCTBLSTR", "Table Readout Start Table Type",
                                  0, 0, 1, EA_DATAFILE_UDEC2_ASCII, -1, "" },
  { EA_CMD_SCMROSTR, 0X0FC0, "SCMROSTR", "CDS Memory Readout Start Address",
                                  0, 0, 2, EA_DATAFILE_HEX4_TO_2HEX2, -1, "" },
  { EA_CMD_SCMRUPLM, 0X0FC2, "SCMRUPLM", "CDS Memory Readout Upper Limit",
                                  0, 0, 2, EA_DATAFILE_HEX4_TO_2HEX2, -1, "" },
  { EA_CMD_SCMRLOLM, 0X0FC3, "SCMRLOLM", "CDS Memory Readout Lower Limit",
                                  0, 0, 2, EA_DATAFILE_HEX4_TO_2HEX2, -1, "" },

  { EA_CMD_SCANXA1, 0X0F04, "SCANXA1", "Select Asc. Node Xing Primary Alg.",
                                  0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCANXA2, 0X0F09, "SCANXA2", "Select Asc. Node Xing Backup Alg.",
                                  0, 0, 0, EA_DATAFILE_NONE, -1, "" },
  { EA_CMD_SCORBSYNC, 0X0F14, "SCORBSYNC", "Orbit Time Sync",
                                  0, 0, 0, EA_DATAFILE_NONE, -1, "" },

  { EA_CMD_SCNOOP, 0X0FF7, "SCNOOP", "No Op",
                                  0, 0, 0, EA_DATAFILE_NONE, -1, "" },

  //----------------------------
  // PBI commands
  //----------------------------
  // Telemetry Request
  //----------------------------
  { EA_CMD_SCPBHDATA, 0X0CB4, "SCPBHDATA", "Request Housekeeping Data",
                                      0, 0, 0, EA_DATAFILE_NONE, 0, "" },
  //----------------------------
  // PBI Management
  //----------------------------
  { EA_CMD_SCPBSWORD, 0X0FE2, "SCPBSWORD", "Transmit 1553 Status Word",
                                      0, 0, 0, EA_DATAFILE_NONE, 0, "" },
  { EA_CMD_SCPBSTEST, 0X0FE3, "SCPBSTEST", "Initiate Self Test",
                                      0, 0, 0, EA_DATAFILE_NONE, 0, "" },
  { EA_CMD_SCPBXMOFF, 0X0FE4, "SCPBXMOFF", "Shutdown Transmitter",
                                      0, 0, 0, EA_DATAFILE_NONE, 0, "" },
  { EA_CMD_SCPBXMOR, 0X0FE5, "SCPBXMOR", "Override Transmitter Shutdown",
                                      0, 0, 0, EA_DATAFILE_NONE, 0, "" },
  { EA_CMD_SCPBRESET, 0X0FE8, "SCPBRESET", "Reset Remote Terminal",
                                      0, 0, 0, EA_DATAFILE_NONE, 0, "" },
  { EA_CMD_SCPBLCMD, 0X0FF2, "SCPBLCMD", "Transmit Last Command",
                                      0, 0, 0, EA_DATAFILE_NONE, 0, "" },
  { EA_CMD_SCPBBWORD, 0X0FF3, "SCPBBWORD", "Transmit BIT Word",
                                      0, 0, 0, EA_DATAFILE_NONE, 0, "" },
  //----------------------------
  // Intercommunication
  //----------------------------
  { EA_CMD_SCTIME, 0X0BA3, "SCTIME", "Send S/C Time Stamp",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0000 0000 0000" },
  { EA_CMD_SCTIMERB, 0X0FA3, "SCTIMERB", "Readback S/C Time Stamp",
                              0, 0, 0, EA_DATAFILE_NONE, 0, "" },
  { EA_CMD_SCPBRST, 0X0863, "SCPBRST", "Reset PBI",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0001 0FFE 0001" },
  { EA_CMD_SCPBRSTCL, 0X0863, "SCPBRSTCL", "Clear PBI Reset Flag",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0001 0FFE 0000" },
  { EA_CMD_SCWDTEN, 0X0863, "SCWDTEN", "Enable WDT",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0200 0DFF 0000" },
  { EA_CMD_SCWDTDS, 0X0863, "SCWDTDS", "Disable WDT",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0200 0DFF 0200" },
  { EA_CMD_SCWDTDSCL, 0X0863, "SCWDTDSCL", "Disable and Clear WDT",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0A00 05FF 0200" },
  { EA_CMD_SCWDTCL, 0X0863, "SCWDTCL", "Clear WDT Expired Flag",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0800 07FF 0000" },
  { EA_CMD_SCWDTST, 0X0863, "SCWDTST", "Set WDT Expired Flag",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0800 07FF 0800" },
  { EA_CMD_SCWDTRST, 0X0861, "SCWDTRST", "Reset WDT",
                              0, 0, 0, EA_DATAFILE_NONE, 1, "0000" },
  { EA_CMD_SCWPVCL, 0X0863, "SCWPVCL", "Clear Write Protect Violated Flag",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0400 0BFF 0000" },
  { EA_CMD_SCWPVST, 0X0863, "SCWPVST", "Set Write Protect Violated Flag",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0400 0BFF 0400" },
  { EA_CMD_SCWPVEN, 0X0863, "SCWPVEN", "Enable Write Protection",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0100 0EFF 0000" },
  { EA_CMD_SCWPVDS, 0X0863, "SCWPVDS", "Disable Write Protection",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0100 0EFF 0100" },
  { EA_CMD_SCPBIRB, 0X0C64, "SCPBIRB", "Readback PBI Maintenance Command",
                              0, 0, 0, EA_DATAFILE_NONE, 0, "" },
  { EA_CMD_SCRSAMOD, 0X0823, "SCRSAMOD", "Modify RSA2 Descriptor",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0000 0000 0000" },
  { EA_CMD_SCRSARB, 0X0C23, "SCRSARB", "Readback RSA2 Descriptor",
                              0, 0, 0, EA_DATAFILE_NONE, 0, "" },
  { EA_CMD_SCDWRAP, 0X0000, "SCDWRAP", "Data Wrap-Around Command",
                              1, 1, -1, EA_DATAFILE_HEX2_ASCII, 0, "" },
  { EA_CMD_SCDWRAPRB, 0X0000, "SCDWRAPRB", "Readback Data Wrap-Around",
                              1, 1, -1, EA_DATAFILE_HEX2_ASCII, 0, "" },
  { EA_CMD_SCMEMRB, 0X0000, "SCMEMRB", "Readback PBI Memory",
                              1, 1, -1, EA_DATAFILE_HEX2_ASCII, 0, "" },
  { EA_CMD_SCMEMLD, 0X0000, "SCMEMLD", "Load PBI Memory",
                              1, 1, -1, EA_DATAFILE_HEX2_ASCII, 0, "" },

  // macros
  { EA_CMD_SCBMOFST81, 0X0FD0, "SCBMOFST81",
                              "Offsets for Beam A&B = 8192,24576",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "2000 6000 E57A" },
  { EA_CMD_SCCALPER0, 0X0F40, "SCCALPER0",
                              "Calibration Sequence Pulse Period = 0",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0000 **** 64EA" },
  { EA_CMD_SCCALPER1, 0X0F40, "SCCALPER1",
                              "Calibration Sequence Pulse Period = 1",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0001 **** 64EB" },
  { EA_CMD_SCGATEWID16, 0X0F52, "SCGATEWID16",
                              "Rx Gate/Xmit Pulse Widths = 1.6,1.5",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0020 001E 653A" },
  { EA_CMD_SCGATEWID17, 0X0F52, "SCGATEWID17",
                              "Rx Gate/Xmit Pulse Widths = 1.7,1.5",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0022 001E 653C" },
  { EA_CMD_SCGATEWID18, 0X0F52, "SCGATEWID18",
                              "Rx Gate/Xmit Pulse Widths = 1.8,1.5",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0024 001E 653E" },
  { EA_CMD_SCGATEWID19, 0X0F52, "SCGATEWID19",
                              "Rx Gate/Xmit Pulse Widths = 1.9,1.3",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0026 001A 653C" },
  { EA_CMD_SCGATEWID20, 0X0F52, "SCGATEWID20",
                              "Rx Gate/Xmit Pulse Widths = 2.0,1.5",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0028 001E 6542" },
  { EA_CMD_SCGATEWID21, 0X0F52, "SCGATEWID21",
                              "Rx Gate/Xmit Pulse Widths = 2.1,1.5",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "002A 001E 6544" },
  { EA_CMD_SCMRLOLM01, 0X0FC3, "SCMRLOLM01",
                              "CDS Mem Readout Lower Limit = 0x010B0008",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "010B 0008 6680" },
  { EA_CMD_SCMROSTR01, 0X0FC0, "SCMROSTR01",
                              "CDS Mem Readout Start Address = 0x010B0008",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "010B 0008 667D" },
  { EA_CMD_SCMRUPLM01, 0X0FC2, "SCMRUPLM01",
                              "CDS Mem Readout Upper Limit = 0x010B0017",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "010B 0017 668E" },
  { EA_CMD_SCMRLOLM00, 0X0FC3, "SCMRLOLM00",
                              "CDS Mem Readout Lower Limit = 0x0014BA70",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0014 BA70 1FF2" },
  { EA_CMD_SCMROSTR00, 0X0FC0, "SCMROSTR00",
                              "CDS Mem Readout Start Address = 0x0014BA70",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0014 BA70 1FEF" },
  { EA_CMD_SCMRUPLM00, 0X0FC2, "SCMRUPLM00",
                              "CDS Mem Readout Upper Limit = 0x0014BA70",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0014 BA7F 2000" },
  { EA_CMD_SCORBTIC0, 0X0FA0, "SCORBTIC0",
                              "Orbit Timer Tics = 0x0,0x3E8",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0000 03E8 6932" },
  { EA_CMD_SCORBTIC2, 0X0FA0, "SCORBTIC2",
                              "Orbit Timer Tics = 0x2,0xF580",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0002 F580 5ACD" },
  { EA_CMD_SCPRFCLK54, 0X0F07, "SCPRFCLK54",
                              "PRF Clock Rate = 5.4",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0036 **** 64E7" },
  { EA_CMD_SCPRFCLK60, 0X0F07, "SCPRFCLK60",
                              "PRF Clock Rate = 6.0",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "003C **** 64ED" },
  { EA_CMD_SCRCVGAN1, 0X0F08, "SCRCVGAN1",
                              "Receiver Gain = 1",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0001 **** 64B3" },
  { EA_CMD_SCRCVGAN2, 0X0F08, "SCRCVGAN2",
                              "Receiver Gain = 2",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0002 **** 64B4" },
  { EA_CMD_SCRCVGAN3, 0X0F08, "SCRCVGAN3",
                              "Receiver Gain = 3",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0003 **** 64B5" },
  { EA_CMD_SCRCVGAN4, 0X0F08, "SCRCVGAN4",
                              "Receiver Gain = 4",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0004 **** 64B6" },
  { EA_CMD_SCRCVGAN5, 0X0F08, "SCRCVGAN5",
                              "Receiver Gain = 5",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0005 **** 64B7" },
  { EA_CMD_SCRCVGAN6, 0X0F08, "SCRCVGAN6",
                              "Receiver Gain = 6",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0006 **** 64B8" },
  { EA_CMD_SCRCVGAN7, 0X0F08, "SCRCVGAN7",
                              "Receiver Gain = 7",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0007 **** 64B9" },
  { EA_CMD_SCRCVGAN8, 0X0F08, "SCRCVGAN8",
                              "Receiver Gain = 8",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0008 **** 64BA" },
  { EA_CMD_SCRCVGAN9, 0X0F08, "SCRCVGAN9",
                              "Receiver Gain = 9",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0009 **** 64BB" },
  { EA_CMD_SCRCVGAN10, 0X0F08, "SCRCVGAN10",
                              "Receiver Gain = 10",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "000A **** 64BC" },
  { EA_CMD_SCRCVGAN11, 0X0F08, "SCRCVGAN11",
                              "Receiver Gain = 11",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "000B **** 64BD" },
  { EA_CMD_SCRCVGAN12, 0X0F08, "SCRCVGAN12",
                              "Receiver Gain = 12",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "000C **** 64BE" },
  { EA_CMD_SCRCVGAN13, 0X0F08, "SCRCVGAN13",
                              "Receiver Gain = 13",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "000D **** 64BF" },
  { EA_CMD_SCRCVGAN14, 0X0F08, "SCRCVGAN14",
                              "Receiver Gain = 14",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "000E **** 64C0" },
  { EA_CMD_SCRCVGAN15, 0X0F08, "SCRCVGAN15",
                              "Receiver Gain = 15",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "000F **** 64C1" },
  { EA_CMD_SCRCVGAN16, 0X0F08, "SCRCVGAN16",
                              "Receiver Gain = 16",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0010 **** 64C2" },
  { EA_CMD_SCRCVGAN17, 0X0F08, "SCRCVGAN17",
                              "Receiver Gain = 17",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0011 **** 64C3" },
  { EA_CMD_SCRCVGAN18, 0X0F08, "SCRCVGAN18",
                              "Receiver Gain = 18",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0012 **** 64C4" },
  { EA_CMD_SCRCVGAN19, 0X0F08, "SCRCVGAN19",
                              "Receiver Gain = 19",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0013 **** 64C5" },
  { EA_CMD_SCRCVGAN20, 0X0F08, "SCRCVGAN20",
                              "Receiver Gain = 20",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0014 **** 64C6" },
  { EA_CMD_SCRGAWID14, 0X0F03, "SCRGAWID14",
                              "Range Gate A Width = 1.4",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "001C **** 64C9" },
  { EA_CMD_SCRGAWID15, 0X0F03, "SCRGAWID15",
                              "Range Gate A Width = 1.5",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "001E **** 64CB" },
  { EA_CMD_SCRGAWID16, 0X0F03, "SCRGAWID16",
                              "Range Gate A Width = 1.6",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0020 **** 64CD" },
  { EA_CMD_SCRGAWID20, 0X0F03, "SCRGAWID20",
                              "Range Gate A Width = 2.0",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0028 **** 64D5" },
  { EA_CMD_SCRGAWID21, 0X0F03, "SCRGAWID21",
                              "Range Gate A Width = 2.1",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "002A **** 64D7" },
  { EA_CMD_SCRGBWID14, 0X0F05, "SCRGBWID14",
                              "Range Gate B Width = 1.4",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "001C **** 64CB" },
  { EA_CMD_SCRGBWID15, 0X0F05, "SCRGBWID15",
                              "Range Gate B Width = 1.5",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "001E **** 64CD" },
  { EA_CMD_SCRGBWID16, 0X0F05, "SCRGBWID16",
                              "Range Gate B Width = 1.6",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0020 **** 64CF" },
  { EA_CMD_SCRGBWID20, 0X0F05, "SCRGBWID20",
                              "Range Gate B Width = 2.0",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0028 **** 64D7" },
  { EA_CMD_SCRGBWID21, 0X0F05, "SCRGBWID21",
                              "Range Gate B Width = 2.1",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "002A **** 64D9" },
  { EA_CMD_SCRSRLPR0, 0X0F41, "SCRSRLPR0",
                              "Reset/Reload Sequence Period = 0",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0000 **** 64EB" },
  { EA_CMD_SCRSRLPR30, 0X0F41, "SCRSRLPR30",
                              "Reset/Reload Sequence Period = 30",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "001E **** 6509" },
  { EA_CMD_SCRSRLPR45, 0X0F41, "SCRSRLPR45",
                              "Reset/Reload Sequence Period = 4545",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "11C1 **** 76AC" },
  { EA_CMD_SCSASOFS57, 0X0FD1, "SCSASOFS57",
                              "Offsets for SAS A and B = 57,16427",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0039 402B A5DF" },
  { EA_CMD_SCSASOFS81, 0X0FD1, "SCSASOFS81",
                              "Offsets for SAS A and B = 8192,24576",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "2000 6000 E57B" },
  { EA_CMD_SCSASOFS16, 0X0FD1, "SCSASOFS16",
                              "Offsets for SAS A and B = 16408,28",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "4018 001C A5AF" },
  { EA_CMD_SCSWPEN1D, 0X0F50, "SCSWPEN1D",
                              "Software Patch Enable = 0x0,0x1D",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0000 001D 6517" },
  { EA_CMD_SCSWPDS1D, 0X0F51, "SCSWPDS1D",
                              "Software Patch Disable = 0x0,0x1D",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0000 001D 6518" },
  { EA_CMD_SCTBLSTR15, 0X0FC1, "SCTBLSTR15",
                              "Table Readout Start Table Type = 15",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "000F **** 657A" },
  { EA_CMD_SCTLDFPEN5F, 0X0F01, "SCTLDFPEN5F",
                              "TWTA Low Drive FP Enable = 0x5F",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "005F **** 650A" },
  { EA_CMD_SCTRPWID09, 0X0F06, "SCTRPWID09",
                              "Transmit Pulse Width = 0.9",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0012 **** 64C2" },
  { EA_CMD_SCTRPWID10, 0X0F06, "SCTRPWID10",
                              "Transmit Pulse Width = 1.0",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0014 **** 64C4" },
  { EA_CMD_SCTRPWID11, 0X0F06, "SCTRPWID11",
                              "Transmit Pulse Width = 1.1",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0016 **** 64C6" },
  { EA_CMD_SCTRPWID12, 0X0F06, "SCTRPWID12",
                              "Transmit Pulse Width = 1.2",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0018 **** 64C8" },
  { EA_CMD_SCTRPWID13, 0X0F06, "SCTRPWID13",
                              "Transmit Pulse Width = 1.3",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "001A **** 64CA" },
  { EA_CMD_SCTRPWID14, 0X0F06, "SCTRPWID14",
                              "Transmit Pulse Width = 1.4",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "001C **** 64CC" },
  { EA_CMD_SCTRPWID15, 0X0F06, "SCTRPWID15",
                              "Transmit Pulse Width = 1.5",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "001E **** 64CE" },
  { EA_CMD_SCTWT1SEL, 0X0F12, "SCTWT1SEL",
                              "T/R TWTA 1 Select",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0001 **** 64BD" },
  { EA_CMD_SCTWT2SEL, 0X0F12, "SCTWT2SEL",
                              "T/R TWTA 2 Select",
                              0, 0, 0, EA_DATAFILE_NONE, 3, "0002 **** 64BE" },

  //------------------------------------
  // Table Macro Commands
  //------------------------------------
  { EA_CMD_SCBDA_DEF, 0X55F2, "SCBDA_DEF",
                     "Doppler A Tbl - Default",
                     1, 0, 0, EA_DATAFILE_NONE, 100, (char*)BdaDefStaticTable},
  { EA_CMD_SCBDA_DTC, 0X55F2, "SCBDA_DTC",
                     "Doppler A Tbl - DTC Const Tone 5A",
                     1, 0, 0, EA_DATAFILE_NONE, 100, (char*)BdaDtcStaticTable},
  { EA_CMD_SCBDA_PBL, 0X55F2, "SCBDA_PBL",
                     "Doppler A Tbl - PBL Const Tone 020A",
                     1, 0, 0, EA_DATAFILE_NONE, 100, (char*)BdaPblStaticTable},
  { EA_CMD_SCBDA_FLT, 0X55F2, "SCBDA_FLT",
                     "Doppler A Tbl - Flight",
                     1, 0, 0, EA_DATAFILE_NONE, 100, (char*)BdaFltStaticTable},
  { EA_CMD_SCBDB_DEF, 0X55F3, "SCBDB_DEF",
                     "Doppler B Tbl - Default",
                     1, 0, 0, EA_DATAFILE_NONE, 100, (char*)BdbDefStaticTable},
  { EA_CMD_SCBDB_DTC, 0X55F3, "SCBDB_DTC",
                     "Doppler B Tbl - DTC Const Tone 5B",
                     1, 0, 0, EA_DATAFILE_NONE, 100, (char*)BdbDtcStaticTable},
  { EA_CMD_SCBDB_PBL, 0X55F3, "SCBDB_PBL",
                     "Doppler B Tbl - PBL Const Tone 050B",
                     1, 0, 0, EA_DATAFILE_NONE, 100, (char*)BdbPblStaticTable},
  { EA_CMD_SCBDB_FLT, 0X55F3, "SCBDB_FLT",
                     "Doppler B Tbl - Flight",
                     1, 0, 0, EA_DATAFILE_NONE, 100, (char*)BdbFltStaticTable},
  { EA_CMD_SCBRA_DEF, 0X55E6, "SCBRA_DEF",
                     "Range A Tbl - Default",
                     1, 0, 0, EA_DATAFILE_NONE, 100, (char*)BraDefStaticTable},
  { EA_CMD_SCBRA_RGC, 0X55E6, "SCBRA_RGC",
                     "Range A Tbl - RGC AZ Const Tone 144",
                     1, 0, 0, EA_DATAFILE_NONE, 100, (char*)BraRgcStaticTable},
  { EA_CMD_SCBRA_FLT, 0X55E6, "SCBRA_FLT",
                     "Range A Tbl - Flight",
                     1, 0, 0, EA_DATAFILE_NONE, 100, (char*)BraFltStaticTable},
  { EA_CMD_SCBRB_DEF, 0X55E7, "SCBRB_DEF",
                     "Range A Tbl - Default",
                     1, 0, 0, EA_DATAFILE_NONE, 100, (char*)BrbDefStaticTable},
  { EA_CMD_SCBRB_RGC, 0X55E7, "SCBRB_RGC",
                     "Range B Tbl - RGC AZ Const Tone 154",
                     1, 0, 0, EA_DATAFILE_NONE, 100, (char*)BrbRgcStaticTable},
  { EA_CMD_SCBRB_FLT, 0X55E7, "SCBRB_FLT",
                     "Range B Tbl - Flight",
                     1, 0, 0, EA_DATAFILE_NONE, 100, (char*)BrbFltStaticTable},
  { EA_CMD_SCENG_T, 0X55F8, "SCENG_T",
                     "S/D Eng Tlm - FF SD Eng Tbl",
                     1, 0, 0, EA_DATAFILE_NONE, 100, (char*)EngTlmStaticTable},
  { EA_CMD_SCPCL_T, 0X55FB, "SCPCL_T",
                     "Cal PRF - FF Cal Prf Tbl",
                     1, 0, 0, EA_DATAFILE_NONE, 100, (char*)CalPrfStaticTable},
  { EA_CMD_SCPRC_T, 0X55FC, "SCPRC_T",
                     "Rcv PRF - FF Rcv Prf Tbl",
                     1, 0, 0, EA_DATAFILE_NONE, 100, (char*)RcvPrfStaticTable},
  { EA_CMD_SCPST_T, 0X55FA, "SCPST_T",
                     "Stby PRF - FF Stby Prf Tbl",
                     1, 0, 0, EA_DATAFILE_NONE, 100, (char*)StbyPrfStaticTable},
  { EA_CMD_SCPWO_T, 0X55FD, "SCPWO_T",
                     "Wind Obs PRF - FF Wind Obs Prf Tbl",
                 1, 0, 0, EA_DATAFILE_NONE, 100, (char*)WindObsPrfStaticTable},
  { EA_CMD_SCSALL_CT1, 0X55CE, "SCSALL_CT1",
                         "All Modes SES - FF SES All Cal TWTA 1",
                 1, 0, 0, EA_DATAFILE_NONE, 100, (char*)SesCalTwt1StaticTable},
  { EA_CMD_SCSALL_CT2, 0X55CE, "SCSALL_CT2",
                         "All Modes SES - FF SES All Cal TWTA 2",
                 1, 0, 0, EA_DATAFILE_NONE, 100, (char*)SesCalTwt2StaticTable},
  { EA_CMD_SCSALL_DT1, 0X55CE, "SCSALL_DT1",
                         "All Modes SES - FBL SES Tbl TWTA 1",
                     1, 0, 0, EA_DATAFILE_NONE, 100, (char*)FblTwt1StaticTable},
  { EA_CMD_SCSALL_DT2, 0X55CE, "SCSALL_DT2",
                         "All Modes SES - FBL SES Tbl TWTA 2",
                     1, 0, 0, EA_DATAFILE_NONE, 100, (char*)FblTwt2StaticTable},
  { EA_CMD_SCSALL_RT1, 0X55CE, "SCSALL_RT1",
                         "All Modes SES - FF SES All Rcv TWTA 1",
                 1, 0, 0, EA_DATAFILE_NONE, 100, (char*)SesRcvTwt1StaticTable},
  { EA_CMD_SCSALL_RT2, 0X55CE, "SCSALL_RT2",
                         "All Modes SES - FF SES All Rcv TWTA 2",
                 1, 0, 0, EA_DATAFILE_NONE, 100, (char*)SesRcvTwt2StaticTable},
  { EA_CMD_SCSALL_ST1, 0X55CE, "SCSALL_ST1",
                         "All Modes SES - FF SES All Stby TWTA 1",
                 1, 0, 0, EA_DATAFILE_NONE, 100, (char*)SesStbyTwt1StaticTable},
  { EA_CMD_SCSALL_ST2, 0X55CE, "SCSALL_ST2",
                         "All Modes SES - FF SES All Stby TWTA 2",
                 1, 0, 0, EA_DATAFILE_NONE, 100, (char*)SesStbyTwt2StaticTable},
  { EA_CMD_SCSALL_WT1, 0X55CE, "SCSALL_WT1",
                         "All Modes SES - FF SES All Wind Obs TWTA 1",
             1, 0, 0, EA_DATAFILE_NONE, 100, (char*)SesWindObsTwt1StaticTable},
  { EA_CMD_SCSALL_WT2, 0X55CE, "SCSALL_WT2",
                         "All Modes SES - FF SES All Wind Obs TWTA 2",
             1, 0, 0, EA_DATAFILE_NONE, 100, (char*)SesWindObsTwt2StaticTable},
  { EA_CMD_SCSCL_T, 0X55CB, "SCSCL_T",
                         "Cal SES - FF Cal SES Tbl",
                 1, 0, 0, EA_DATAFILE_NONE, 100, (char*)FfCalSesStaticTable},
  { EA_CMD_SCSRC_T, 0X55CC, "SCSRC_T",
                         "Rcv SES - FF Rcv SES Tbl",
                 1, 0, 0, EA_DATAFILE_NONE, 100, (char*)FfRcvSesStaticTable},
  { EA_CMD_SCSST_T, 0X55CA, "SCSST_T",
                         "Stby SES - FF Stby SES Tbl",
                 1, 0, 0, EA_DATAFILE_NONE, 100, (char*)FfStbySesStaticTable},
  { EA_CMD_SCSTA_T, 0X55F9, "SCSTA_T",
                         "S/D Status Tlm - FF SD Status Tbl",
                 1, 0, 0, EA_DATAFILE_NONE, 100, (char*)SdStatusStaticTable},
  { EA_CMD_SCSWO_T, 0X55CD, "SCSWO_T",
                         "Wind Obs SES - FF Wind Obs SES Tbl",
                 1, 0, 0, EA_DATAFILE_NONE, 100, (char*)FfWindObsStaticTable},
  { EA_CMD_SCSTATBLE, 0X55F9, "SCSTATBLE",
                         "S/D Status Tlm with Error Counter Vs. Latches",
              1, 0, 0, EA_DATAFILE_NONE, 100, (char*)StatusTlmCntrStaticTable},
  { EA_CMD_SCENGTBLC, 0X55F8, "SCENGTBLC",
                         "S/D Engineering Tlm with Currents Vs. Temps",
                1, 0, 0, EA_DATAFILE_NONE, 100, (char*)EngTlmCrntStaticTable},
  { EA_CMD_SCSWPATCH_T, 0XE3E3, "SCSWPATCH_T",
                         "Software Patch - Good File Patch",
               1, 0, 0, EA_DATAFILE_NONE, 100, (char*)SwPatchStaticTable},

  //-----------------------------------------------------
  // LASP proc macros
  // Attention: the Command Hex code is made-up for
  //            internal processing, it is not official
  //-----------------------------------------------------
  { EA_CMD_VBAND_RELEASE, 0X7101, "VBAND_RELEASE",
                         "V-band release - uncage scat. antenna",
                         1, 0, 0, EA_DATAFILE_NONE, -1, ""},
  { EA_CMD_PLB_TURN_ON, 0X7102, "PLB_TURN_ON", "Payload Bus turn-on",
                         1, 0, 1, EA_DATAFILE_ASCII, -1, ""},
  { EA_CMD_PLB_TURN_OFF, 0X7103, "PLB_TURN_OFF", "Payload Bus turn-off",
                         1, 0, 1, EA_DATAFILE_ASCII, -1, ""},
  { EA_CMD_PPS_TURN_ON, 0X7104, "PPS_TURN_ON", "Payload Power Supply turn-on",
                         1, 0, 1, EA_DATAFILE_ASCII, -1, ""},
  { EA_CMD_PPS_TURN_OFF, 0X7105, "PPS_TURN_OFF","Payload Power Supply turn-off",
                         1, 0, 1, EA_DATAFILE_ASCII, -1, ""},
  { EA_CMD_CDS_TURN_ON, 0X7106, "CDS_TURN_ON", "CDS turn-on",
                         1, 0, 1, EA_DATAFILE_ASCII, -1, ""},
  { EA_CMD_CDS_TURN_OFF, 0X7107, "CDS_TURN_OFF", "CDS turn-off",
                         1, 0, 0, EA_DATAFILE_NONE, -1, ""},
  { EA_CMD_SAS_TURN_ON, 0X7108, "SAS_TURN_ON", "SAS turn-on",
                         1, 0, 3, EA_DATAFILE_ASCII, -1, ""},
  { EA_CMD_SAS_TURN_OFF, 0X7109, "SAS_TURN_OFF", "SAS turn-off",
                         1, 0, 0, EA_DATAFILE_NONE, -1, ""},
  { EA_CMD_SAS_SPIN_VFY, 0X7110, "SAS_SPIN_VFY", "Verify SAS spin-up",
                         1, 0, 2, EA_DATAFILE_ASCII, -1, ""},
  { EA_CMD_SAS_SPIN_VFY2, 0X7111, "SAS_SPIN_VFY2", "Verify SAS spin-up (2)",
                         1, 0, 2, EA_DATAFILE_ASCII, -1, ""},
  { EA_CMD_SES_TURN_ON, 0X7112, "SES_TURN_ON", "SES turn-on",
                         1, 0, 1, EA_DATAFILE_ASCII, -1, ""},
  { EA_CMD_SES_TURN_OFF, 0X7113, "SES_TURN_OFF", "SES turn-off",
                         1, 0, 0, EA_DATAFILE_NONE, -1, ""},
  { EA_CMD_TWTA_TURN_ON, 0X7114, "TWTA_TURN_ON", "TWTA turn-on",
                         1, 0, 1, EA_DATAFILE_ASCII, -1, ""},
  { EA_CMD_TWTA_TURN_OFF, 0X7115, "TWTA_TURN_OFF", "TWTA turn-off",
                         1, 0, 0, EA_DATAFILE_NONE, -1, ""},
  { EA_CMD_TWTA_VFY, 0X7116, "TWTA_VFY",
                         "Wait 3.5 min. and Verify TWTA telemetry",
                         1, 0, 1, EA_DATAFILE_ASCII, -1, ""},
  { EA_CMD_INST_MODE, 0X7117, "INST_MODE", "Go to specified mode",
                         1, 0, 1, EA_DATAFILE_ASCII, -1, ""},
  { EA_CMD_SES_RESET_ENBL, 0X7118,
                         "SES_RESET_ENBL", "Enable automatic SES reset",
                         1, 0, 0, EA_DATAFILE_NONE, -1, ""},
  { EA_CMD_SES_RESET_DSBL, 0X7119,
                         "SES_RESET_DSBL", "Disable automatic SES reset",
                         1, 0, 0, EA_DATAFILE_NONE, -1, ""},
  { EA_CMD_INST_QUIK_ON, 0X7120,
                         "INST_QUIK_ON", "Turn Instrument on in one pass",
                         1, 0, 1, EA_DATAFILE_ASCII, -1, ""},
  { EA_CMD_SES_RESET, 0X7121, "SES_RESET", "Reset SES",
                         1, 0, 0, EA_DATAFILE_NONE, -1, ""},
  { EA_CMD_INST_TURN_OFF, 0X7122, "INST_TURN_OFF", "Turn Instrument off",
                         1, 0, 0, EA_DATAFILE_NONE, -1, ""},
  { EA_CMD_SCAT_FST_ACQ, 0X7123, "SCAT_FST_ACQ",
                  "Check Instrument for nominal status for first acquisition",
                         1, 0, 0, EA_DATAFILE_NONE, -1, ""},
  { EA_CMD_RAD_PAR_UPDATE, 0X7124, "RAD_PAR_UPDATE",
                         "Switch to new onboard stored gate width",
                         1, 0, 1, EA_DATAFILE_ASCII, -1, ""},
  { EA_CMD_SCAT_MOD_ON, 0X7125, "SCAT_MOD_ON", "Turn on modulation",
                         1, 0, 0, EA_DATAFILE_NONE, -1, ""},
  { EA_CMD_SCAT_MOD_OFF, 0X7126, "SCAT_MOD_OFF", "Turn off modulation",
                         1, 0, 0, EA_DATAFILE_NONE, -1, ""},
  { EA_CMD_CDS_TMON_ENBL, 0X7127, "CDS_TMON_ENBL", "Enable CDS thermal safing",
                         1, 0, 0, EA_DATAFILE_NONE, -1, ""},
  { EA_CMD_SES_TMON_ENBL, 0X7128, "SES_TMON_ENBL", "Enable SES thermal safing",
                         1, 0, 0, EA_DATAFILE_NONE, -1, ""},
  { EA_CMD_SAS_TMON_ENBL, 0X7129, "SAS_TMON_ENBL", "Enable SAS thermal safing",
                         1, 0, 0, EA_DATAFILE_NONE, -1, ""},
  { EA_CMD_INST_TEMP_MON, 0X7130, "INST_TEMP_MON",
                         "Enable/disable autonomous instrument temp monitoring",
                         1, 0, 1, EA_DATAFILE_ASCII, -1, ""}
};

const int Command::numCmdArgsEntries = ElementNumber(cmdArgsTable);

//static
const CmdTypeMnemonics Command::cmd_type_mnemonics[] =
{
    { TYP_UNKNOWN, "Unknown" },
    { TYP_AUTOMATIC, "Automatic" },
    { TYP_REAL_TIME, "Real Time" },
    { TYP_UNPLANNED, "Unplanned" },
    { (CmdTypeE)0, (char*)0 }
};

//static
const EffectMnemonics Command::effect_mnemonics[] =
{
    { EFF_UNKNOWN, "Unknown" },
    { EFF_ALL_OFF, "Off" },
    { EFF_ELECTRONICS_ON, "Elec On" },
    { EFF_RHM, "RHM" },
    { EFF_TWTA_ON,"TWTA Electronincs On" },
    { EFF_TWTA_REPL_HEATER_ON, "TWTA Rep Htr On" },
    { EFF_TWTA_REPL_HEATER_OFF, "TWTA Rep Htr Off" },
    { EFF_CDS_A, "CDS A" },
    { EFF_CDS_B, "CDS B" },
    { EFF_SES_ELECTRONICS_ON,"SES Electronics On" },
    { EFF_SES_REPL_HEATER_ON, "SES Rep Htr On" },
    { EFF_SES_REPL_HEATER_OFF, "SES Rep Htr Off" },
    { EFF_SES_SUPPL_HEATER_ON, "SES Supplemental Htr On" },
    { EFF_SES_SUPPL_HEATER_OFF, "SES Supplemental Htr Off" },
    { EFF_SAS_ELECTRONICS_ON, "SAS Electronics On",},
    { EFF_SAS_REPL_HEATER_ON, "SAS Replacements HEATER On"},
    { EFF_REPLACEMENT_HEATERS_ON,"All Replacment Heaters On"},
    { EFF_REPLACEMENT_HEATERS_OFF,"All Replacment Heaters Off"},
    { EFF_STB, "STB" },
    { EFF_RCV, "RCV" },
    { EFF_CAL, "CAL" },
    { EFF_WOM, "WOM" },
    { EFF_TWTA_1, "TWTA #1" },
    { EFF_TWTA_2, "TWTA #2" },
    { EFF_TRS_CHANGE, "TRS Change" },
    { EFF_TWT_TRIP_OVERRIDE_ENABLE, "TWT Trip Ovr Enable" },
    { EFF_TWT_TRIP_OVERRIDE_DISABLE, "TWT Trip Ovr Disable" },
    { EFF_TWTA_MONITOR_CHANGE, "TWTA Trip Mon Change" },
    { EFF_TWTA_LOWDRIVE_POWER_FP_ON,"TWTA Low Drive Power FP On"},
    { EFF_TWTA_LOWDRIVE_POWER_FP_OFF,"TWTA Low Drive Power FP Off"},
    // These effects are seen in the command history queue
    { EFF_UNKNOWABLE_QPX,    "CH: Table changed before readback" } ,
    { EFF_PRF_STANDBY_TABLE_UPDATE, "CH: Prf Standby Table Update"},    
    { EFF_PRF_WOM_TABLE_UPDATE,     "CH: Prf WOM Table Update"},        
    { EFF_PRF_CAL_TABLE_UPDATE,     "CH: Prf Cal Table Update"},        
    { EFF_PRF_RCV_TABLE_UPDATE,     "CH: SES Recv Table Update"},        
    { EFF_SES_ALL_TABLE_UPDATE,     "CH: SES All Modes Update"},      
    { EFF_SES_WOM_TABLE_UPDATE,     "CH: SES WOM Table Update"},        
    { EFF_SES_CAL_TABLE_UPDATE,     "CH: SES Cal Table Update"},        
    { EFF_SES_RCV_TABLE_UPDATE,     "CH: SES Recv Table Update"},        
    { EFF_SES_STANDBY_TABLE_UPDATE, "CH: SES Standby Table Update"},    
    { EFF_RANGEGATE_A_TABLE_UPDATE, "CH: RangeGate A Table Update"},  
    { EFF_RANGEGATE_B_TABLE_UPDATE, "CH: RangeGate A Table Update"},  
    { EFF_DOPPLER_A_TABLE_UPDATE,   "CH: RangeGate A Table Update"},  
    { EFF_DOPPLER_B_TABLE_UPDATE,   "CH: RangeGate A Table Update"},  
    { EFF_SER_DIG_ENG_TLM_TABLE_UPDATE, "CH: SerDig Eng Tlm Tbl Update"},   
    { EFF_SER_DIG_ST_TLM_TABLE_UPDATE,  "CH: SerDig Status Tlm Tbl Update"},
    // TABLE CHANGE EFFECTS (via command history queue)
    { EFF_PRF_STANDBY_TABLE_CHANGE, "CH: Prf Standby Table Change"},  
    { EFF_PRF_WOM_TABLE_CHANGE,     "CH: Prf WOM Table Change"},      
    { EFF_PRF_CAL_TABLE_CHANGE,     "CH: Prf Cal Table Change"},      
    { EFF_PRF_RCV_TABLE_CHANGE,     "CH: SES Recv Table Change"},     
    { EFF_SES_ALL_TABLE_CHANGE,     "CH: SES All Modes Change"},      
    { EFF_SES_WOM_TABLE_CHANGE,     "CH: SES WOM Table Change"},      
    { EFF_SES_CAL_TABLE_CHANGE,     "CH: SES Cal Table Change"},      
    { EFF_SES_RCV_TABLE_CHANGE,     "CH: SES Recv Table Change"},     
    { EFF_SES_STANDBY_TABLE_CHANGE, "CH: SES Standby Table Change"},  
    { EFF_RANGEGATE_A_TABLE_CHANGE, "CH: RangeGate A Table Change"},  
    { EFF_RANGEGATE_B_TABLE_CHANGE, "CH: RangeGate A Table Change"},  
    { EFF_DOPPLER_A_TABLE_CHANGE,   "CH: Doppler A Table Change"},  
    { EFF_DOPPLER_B_TABLE_CHANGE,   "CH: Doppler A Table Change"},  
    { EFF_SER_DIG_ENG_TLM_TABLE_CHANGE, "CH: SerDig Eng Tlm Tbl Change"},   
    { EFF_SER_DIG_ST_TLM_TABLE_CHANGE,  "CH: SerDig Status Tlm Tbl Change"},
    // Status flag changes (table updates)
    //{ EFF_SES_PARAMS_TABLE_UPDATE, "ST: SES Params Table Update"},
    //{ EFF_PRF_TABLE_UPDATE,       "ST: PRF Params Table Update"},
    //{ EFF_RANGEGATE_TABLE_UPDATE, "ST: RangeGate Params Table Update"},
    //{ EFF_DOPPLER_TABLE_UPDATE,   "ST: Doppler Params Table Update"},
    // Status flag changes (table changes)
    { EFF_SES_PARAMS_TABLE_CHANGE, "ST: SES Params Table Change"},
    { EFF_PRF_TABLE_CHANGE,       "ST: PRF Params Table Change"},
    { EFF_RANGEGATE_TABLE_CHANGE, "ST: RangeGate Params Table Change"},
    { EFF_DOPPLER_TABLE_CHANGE,   "ST: Doppler Params Table Change"},
    { EFF_SER_DIG_ENG_TLM_TABLE_CHANGE, "ST: SerDig Eng Tlm Tbl Change"},
    { EFF_SER_DIG_ST_TLM_TABLE_CHANGE,  "ST: SerDig Status Tlm Tbl Change"},
    // Quasi Effect for reporting final status.
    { EFF_PRF_STANDBY_TABLE_FINAL, "PRF Standby table Final" },
    { EFF_PRF_WOM_TABLE_FINAL,     "PRF WOM table Final" },
    { EFF_PRF_CAL_TABLE_FINAL,     "PRF CAL table Final" },
    { EFF_PRF_RCV_TABLE_FINAL,     "PRF RCV table Final" },
    { EFF_SES_ALL_TABLE_FINAL,     "SES ALL table Final" },
    { EFF_SES_WOM_TABLE_FINAL,     "SES WOM table Final" },
    { EFF_SES_CAL_TABLE_FINAL,     "SES CAL table Final" },
    { EFF_SES_RCV_TABLE_FINAL,     "SES ReCV table Final" },
    { EFF_SES_STANDBY_TABLE_FINAL, "SES Standby table Final" },
    { EFF_RANGEGATE_A_TABLE_FINAL,  "RangeGate A table Final" },
    { EFF_RANGEGATE_B_TABLE_FINAL,   "Rangegate B table Final" },
    { EFF_DOPPLER_A_TABLE_FINAL,     "Doppler A table Final" },
    { EFF_DOPPLER_B_TABLE_FINAL,     "Doppler B table Final" },
    { EFF_SER_DIG_ENG_TLM_TABLE_FINAL, "Serial Digital Eng Tlm table Final" },
    { EFF_SER_DIG_ST_TLM_TABLE_FINAL,  "Serial Digital Status Tlm table Final" },

    //

    { EFF_VALID_COMMAND_CNTR_CHANGE,"Valid Command Counter Change" },
    { EFF_INVALID_COMMAND_CNTR_CHANGE,"Invalid Command Counter Change" },
    { EFF_COMMAND_HISTORY_CHANGE, "Command History Queue Change" },
    { EFF_COMMAND_HISTORY_REPEAT, "Command History Queue Repeat" },
    { EFF_SAS_A, "SAS A" },
    { EFF_SAS_B, "SAS B" },
    { EFF_SES_A, "SES A" },
    { EFF_SES_B, "SES B" },
    { EFF_SAS_A_SPIN_RATE_198, "SAS-A to Spin Rate to 19.8 RPM"},
    { EFF_SAS_A_SPIN_RATE_180, "SAS-A to Spin Rate to 18.0 RPM"},
    { EFF_SAS_B_SPIN_RATE_198, "SAS-B to Spin Rate to 19.8 RPM"},
    { EFF_SAS_B_SPIN_RATE_180, "SAS-B to Spin Rate to 18.0 RPM"},
    { EFF_RANGE_GATE_A_WIDTH_CHANGE,"Range Gate A Width Change"},
    { EFF_RANGE_GATE_B_WIDTH_CHANGE,"Range Gate B Width Change"},
    { EFF_TRANSMIT_PULSE_WIDTH_CHANGE,"Transmit Pulse Width Change"},
    { EFF_RECEIVER_GAIN_CHANGE,"Reciever Gain Change" },
    { EFF_GRID_NORMAL,   "Grid Normal" },
    { EFF_GRID_DISABLED, "Grid Disabled" },
    { EFF_MODULATION_ON,"Modulation On" },
    { EFF_MODULATION_OFF,"Modulation Off" },
    { EFF_PRF_CLOCK_CHANGE,"Prf Clock Change"},
    { EFF_RECEIVE_PROTECT_NORMAL,"Receive Protect Normal"},
    { EFF_RECEIVE_PROTECT_ON,"Receive Protect On"},
    { EFF_SES_RESET, "SES Reset" },
    { EFF_SES_MULTISEQ_DLE_RESPONSE_CHANGE,"SES Multi SEQ Rspns Change"},
    { EFF_SAS_MULTISEQ_DLE_RESPONSE_CHANGE,"SAS Multi SEQ Rspns Change"},
    { EFF_SES_SUPPL_HEATER_CNTRL_CHANGE,"SES Suppl Htr Cntrl Change"},
    { EFF_CAL_PULSE_PERIOD_CHANGE,"Calibration Pulse Period Change"},
    { EFF_SES_RESET_RELOAD_PERIOD,"SES Reset/Reload Period Commanded"},
    { EFF_CDS_SW_PATCH,"CDS Software Patch"},
    { EFF_CDS_SW_PATCH_ENABLE,"Software Patch Enable"},
    { EFF_CDS_SW_PATCH_DISABLE,"Software Patch Disable"},
    { EFF_TRANSMITPULSE_RANGEGATE_WIDTH_CHANGE,"Change in pulse/gate widths"},
    { EFF_ORBIT_TIME_TICKS,"Timer Ticks/Orbit" },
    { EFF_INST_TIME_SYNC_INTERVAL,"Instrument Time Sync Interval Change" },
    { EFF_CDS_MEMORY_RDOUT_START_ADDR,"CDS Memory Readout Start Address" },
    { EFF_CDS_MEMORY_RDOUT_UP_LIM,"CDS Memory Readout Upper Limit" },
    { EFF_CDS_MEMORY_RDOUT_LO_LIM,"CDS Memory Readout Lower Limit" },
    { EFF_TABLE_READOUT_START,"ID of Tlm Table to read out" },
    { EFF_BEAM_OFFSETS,"Change in Offsets for Beams A/B"},
    { EFF_SAS_OFFSETS,"Change in SAS offsets A/B" },
    { EFF_CDS_SOFT_RESET, "CDS Soft Reset" },
    { EFF_NONE, "None" },
    { EFF_MOOT, "Moot" },
    { (EffectE)0, (char*)0 }
};

//static
const CmdStatusMnemonics Command::cmd_status_mnemonics[] =
{
    { STA_UNKNOWN, "Unknown", "???" },
    { STA_OK, "OK", "OK" },
    { STA_UNEXPECTED, "Unexpected", "UnX" },
    { STA_BOGUS, "Bogus", "Bog" },
    { STA_CANCELED, "Canceled", "Can" },
    { STA_MISSED, "Missed", "Mis" },
    { STA_REJECTED, "Rejected", "Rej" },
    { STA_GHOST, "Ghost", "Gho" },
    { STA_ANOMALOUS, "Anomalous", "Ano" },
    { STA_IDIOSYNCRATIC, "Idiosyncratic", "Idi" },
    { (CmdStatusE)0, (char*)0, (char*)0 }
};

//static
const CmdVerifyMnemonics Command::cmd_verify_mnemonics[]=
{
    { VER_NO, "No" },
    { VER_YES, "Yes" },
    { VER_NA, "N/A" },
    { VER_OK, "OK" },
    { (VerifyE)0, (char*)0 }
};

//=================
// Command methods 
//=================

Command::Command(
int         lineNo,              // line number
const char* reqiCommandString)   // REQI command string
:   commandId(EA_CMD_UNKNOWN), cmdHex(0),
    plannedTpg(INVALID_TPG), dataFilename(NULL),
    cmdParams(NULL),originator(NULL), comments(NULL), 
    qpx_filename(NULL),
    effectId(EFF_UNKNOWN),effect_value(0),statusId(STA_OK),
    table_type(EA_TBLTYPE_UNKNOWN), tableid( EA_TBL_UNKNOWN),
    expectedTime(INVALID_TIME), l1aTime(INVALID_TIME),
    hk2Time(INVALID_TIME), l1apTime(INVALID_TIME), l1aVerify(VER_NO),
    hk2Verify(VER_NO), l1apVerify(VER_NO), tableRepetition(0),
    _status(OK), _commandType(COMMAND_TYPE_UNKNOWN), 
    _format(FORMAT_UNKNOWN), tbl(NULL), uqpx_filename(NULL)
    
{
    if ( ! ReadReqiCommandString(lineNo, reqiCommandString))
        _status = ERROR_READING_CMD;
    tbl= new UpldTbl();

} // Command::Command

Command::Command(
EACommandE     cmdID)
:   commandId(cmdID),
    plannedTpg(INVALID_TPG), dataFilename(NULL),
    cmdParams(NULL),originator(NULL), comments(NULL), effectId(EFF_UNKNOWN),
    effect_value(0), statusId(STA_OK),
    expectedTime(INVALID_TIME), l1aTime(INVALID_TIME),
    hk2Time(INVALID_TIME), l1apTime(INVALID_TIME), l1aVerify(VER_NO),
    hk2Verify(VER_NO), l1apVerify(VER_NO), tableRepetition(0),
    _status(OK), _commandType(COMMAND_TYPE_UNKNOWN),
    _format(FORMAT_UNKNOWN), tbl(NULL),uqpx_filename(NULL)

{
    cmdHex = CmdIdToCmdHex(cmdID);
    strncpy(mnemonic, CmdIdToMnemonic(cmdID), STRING_LEN - 1);
    mnemonic[STRING_LEN - 1] = '\0';
}

Command::Command()
:   commandId(EA_CMD_UNKNOWN), cmdHex(0),
    plannedTpg(INVALID_TPG), dataFilename(NULL),
    cmdParams(NULL),originator(NULL), comments(NULL), 
    qpx_filename(NULL), 
    effectId(EFF_UNKNOWN), effect_value(0), 
    statusId(STA_OK),expectedTime(INVALID_TIME), 
    l1aTime(INVALID_TIME),hk2Time(INVALID_TIME), 
    l1apTime(INVALID_TIME), l1aVerify(VER_NO),
    hk2Verify(VER_NO), l1apVerify(VER_NO), tableRepetition(0),
    _status(OK),     table_type(EA_TBLTYPE_UNKNOWN), 
    tableid( EA_TBL_UNKNOWN),
    _commandType(COMMAND_TYPE_UNKNOWN),
    _format(FORMAT_UNKNOWN), tbl(NULL), uqpx_filename(NULL)

{
    tbl= new UpldTbl();
}

Command::~Command()
{
    free(dataFilename);
    free(cmdParams);
    free(originator);
    free(comments);
    free(qpx_filename);
    //    if (table != NULL)
    //        free (table);
    if (tbl)
        delete tbl;

    return;
}

//------
// Read 
//------

Command::StatusE
Command::Read(
    FILE*   ifp)
{
    int retval = _ReadMarker(ifp, CMD_START_MARKER);
    if (retval == -1)
        _status = END_OF_FILE;
    else if (retval == 0)
        _status = ERROR_READING_CMD;
    else if (_ReadInt(ifp, COMMAND_ID_LABEL, (int *)&commandId) &&
        _ReadTpg(ifp, PLANNED_TPG_LABEL, &plannedTpg) &&
        _ReadString(ifp, DATA_FILENAME_LABEL, &dataFilename) &&
        _ReadString(ifp, CMD_PARAMS_LABEL, &cmdParams) &&
        _ReadString(ifp, ORIGINATOR_LABEL, &originator) &&
        _ReadString(ifp, COMMENTS_LABEL, &comments) &&
        _ReadInt(ifp, EFFECT_ID_LABEL, (int *)&effectId) &&
        _ReadInt(ifp, STATUS_ID_LABEL, (int *)&statusId) &&
        _ReadTime(ifp, EXPECTED_TIME_LABEL, &expectedTime) &&
        _ReadTime(ifp, L1A_TIME_LABEL, &l1aTime) &&
        _ReadTime(ifp, HK2_TIME_LABEL, &hk2Time) &&
        _ReadTime(ifp, L1AP_TIME_LABEL, &l1apTime) &&
        _ReadInt(ifp, L1A_VERIFY_LABEL, (int *)&l1aVerify) &&
        _ReadInt(ifp, HK2_VERIFY_LABEL, (int *)&hk2Verify) &&
        _ReadInt(ifp, L1AP_VERIFY_LABEL, (int *)&l1apVerify) &&
        _ReadShort(ifp, EFFECT_VALUE_LABEL, (short int *)&effect_value) &&
        _ReadInt(ifp, TABLE_TYPE_LABEL, (int *) &table_type) &&
        _ReadInt(ifp, TABLE_ID_LABEL, (int *) &tableid) &&
        _ReadString(ifp, QPX_FILENAME_LABEL, &qpx_filename) &&
        _ReadString(ifp, UQPX_LABEL, &uqpx_filename) &&
        _ReadMarker(ifp, CMD_END_MARKER) )
    {
        _status = OK;
    }
    else
        _status = ERROR_READING_CMD;

    return (_status);
}

//-------------------------
// ReadReqiCommandString 
//-------------------------

int
Command::ReadReqiCommandString(
int           lineNo,    // line number
const char*   line)      // REQI command line
{
    if (line == 0) return 0;

    // take out the trailing '\n' and blanks first
    char* lastChar = (char*) line + strlen(line) - 1;
    if (*lastChar == '\n')
        *lastChar-- = '\0';
    while (*lastChar == ' ')
        *lastChar-- = '\0';

    const char* ptr = line;
    // look for a time and a command mnemonic
    char timeString[STRING_LEN];
    int bytes=0;
    if (sscanf(ptr, " %s %s%n", timeString, mnemonic, &bytes) != 2)
        return 0;

    Itime::CodeAFormatE timeFormat =
                            Itime::CheckCodeAFormat(timeString);
                
    switch (timeFormat)
    {
        case Itime::CODE_A_FORMAT_UNKNOWN:
        case Itime::CODE_A_FORMAT_YEAR:
        case Itime::CODE_A_FORMAT_MONTH:
        case Itime::CODE_A_FORMAT_MILLI_SECOND:
            fprintf(stderr,
                    "\nInvalid time string(%s) at line %d\n",
                    timeString, lineNo);
            return 0;
            break;
        case Itime::CODE_A_FORMAT_DAY:
            break;
        case Itime::CODE_A_FORMAT_HOUR:
            _commandType = REALTIME_COMMAND;
            _format = USE_TIME_FORMAT;
            break;
        case Itime::CODE_A_FORMAT_MINUTE:
        case Itime::CODE_A_FORMAT_SECOND:
            _commandType = AUTOMATIC_COMMAND;
            _format = USE_TIME_FORMAT;
            break;
    }
    // set the time
    Itime cmd_time;
    cmd_time.CodeAToItime(timeString);
    plannedTpg.time = cmd_time;
    expectedTime = cmd_time;        // current best guess

    // set the command id
    commandId = MnemonicToCmdId(mnemonic, cmdHex);
    if (commandId == EA_CMD_UNKNOWN)
    {
        fprintf(stderr,
                "\nInvalid command mnemonic (%s) at line %d\n",
                mnemonic, lineNo);
        return 0;
    }
    ptr += bytes;

    // set isQPF
    SetIsQPF(IsQPF(commandId));

    // set the datafile format
    SetDatafileFormat(DatafileFormat(commandId));

    // set the num of static params
    SetNumStaticParams(NumStaticParams(commandId));

    // set the static param string
    SetStaticParamString(StaticParamString(commandId));

    // look for a path and argument of latitutde
    int path;
    float gamma;
    bytes = 0;
    if (sscanf(ptr, " %d%n", &path, &bytes) == 1)
    {
        // catch time format commands + path
        if (_format == USE_TIME_FORMAT)
        {
            fprintf(stderr, "\nLine %d: both time and path are specified\n",
                            lineNo);
            return 0;
        }
        _format = USE_PATH_FORMAT;
        plannedTpg.path = path;
        ptr += bytes;
    }

    bytes = 0;
    if (sscanf(ptr, " %g%n", &gamma, &bytes) == 1)
    {
        _commandType = AUTOMATIC_COMMAND;
        plannedTpg.gamma = gamma;
        ptr += bytes;
    }
    else
    {
        if (_commandType == COMMAND_TYPE_UNKNOWN)
            _commandType = REALTIME_COMMAND;
    }

    if (RealtimeOnly(commandId) && _commandType != REALTIME_COMMAND)
    {
        fprintf(stderr, "\nLine %d: %s: must be a realtime command only\n",
                        lineNo, line);
        return 0;
    }

    // check if this command requires data file
    if ( ! NeedDataFile(commandId))
    {
        // should know the command type and format by now
        if (_commandType != COMMAND_TYPE_UNKNOWN && _format != FORMAT_UNKNOWN)
        {
            // no more junk after this, then it is ok
            if ((unsigned int)(ptr - line) == strlen(line))
                return 1;
            else
            {
                fprintf(stderr, "\nLine %d: extra arguments [%s]\n",
                            lineNo, ptr);
                return 0;
            }
        }
    }

    char filename[MAX_LINE_SIZE];
    if (sscanf(ptr, " %s%n", filename, &bytes) == 1)
    {
        SetDataFilename(filename);
        ptr += bytes;
    }

    // set the number of words in param file, if any
    // this must be done after data filename is set
    SetNumWordsInParamFile(NumWordsInParamFile(commandId));

    // should know the command type and format by now
    if (_commandType != COMMAND_TYPE_UNKNOWN && _format != FORMAT_UNKNOWN)
    {
        // no more junk after this, then it is ok
        if ((unsigned int)(ptr - line) == strlen(line))
            return 1;
        else
        {
            fprintf(stderr, "\nLine %d: extra arguments [%s]\n",
                              lineNo, ptr);
            return 0;
        }
    }

    return 1;

} // Command::ReadReqiCommandString

//-------
// Write 
//-------

Command::StatusE
Command::Write(
    FILE*   ofp)
{
    if (tbl) {
        if (table_type == EA_TBL_UNKNOWN)
            table_type=tbl->GetTableType();
        if (tableid == EA_TBL_UNKNOWN)
            tableid=tbl->GetTableId();
        if (qpx_filename == NULL && 
            tbl->GetDirectory() != NULL &&
            tbl->GetFilename() != NULL ) {
            char file_string[MAX_FILENAME_LEN+1];
            snprintf(file_string,
                     MAX_FILENAME_LEN ,
                     "%s/%s",
                     tbl->GetDirectory(),
                     tbl->GetFilename());
            qpx_filename =strdup( file_string );
        }
    } 

    _WriteMarker(ofp, CMD_START_MARKER);

    _WriteInt(ofp, COMMAND_ID_LABEL, commandId);
    _WriteTpg(ofp, PLANNED_TPG_LABEL, plannedTpg);
    _WriteString(ofp, DATA_FILENAME_LABEL, dataFilename);
    _WriteString(ofp, CMD_PARAMS_LABEL, cmdParams);
    _WriteString(ofp, ORIGINATOR_LABEL, originator);
    _WriteString(ofp, COMMENTS_LABEL, comments);

    _WriteInt(ofp, EFFECT_ID_LABEL, effectId);
    _WriteInt(ofp, STATUS_ID_LABEL, statusId);
    _WriteTime(ofp, EXPECTED_TIME_LABEL, expectedTime);

    _WriteTime(ofp, L1A_TIME_LABEL, l1aTime);
    _WriteTime(ofp, HK2_TIME_LABEL, hk2Time);
    _WriteTime(ofp, L1AP_TIME_LABEL, l1apTime);

    _WriteInt(ofp, L1A_VERIFY_LABEL, l1aVerify);
    _WriteInt(ofp, HK2_VERIFY_LABEL, hk2Verify);
    _WriteInt(ofp, L1AP_VERIFY_LABEL, l1apVerify);

    _WriteInt(ofp, EFFECT_VALUE_LABEL, (int) effect_value);
    _WriteInt(ofp, TABLE_TYPE_LABEL, (int) table_type);
    _WriteInt(ofp, TABLE_ID_LABEL, (int) tableid);
    _WriteString(ofp, QPX_FILENAME_LABEL, qpx_filename);
    _WriteString(ofp, UQPX_LABEL, uqpx_filename);

    _WriteMarker(ofp, CMD_END_MARKER);

    return (_status);
}

//----------------
// WriteForHumans 
//----------------

Command::StatusE
Command::WriteForHumans(
FILE*   ofp)
{
    char string[256];

    //---------------------------------------
    // command mnemonic and the command type 
    //---------------------------------------

    // mnemonic is not written out to cmdlp, need to expand here
    strncpy(mnemonic, CmdIdToMnemonic(commandId), STRING_LEN - 1);
    mnemonic[STRING_LEN - 1] = '\0';

    CmdTypeE cmd_type = CmdType();
    if (cmd_type == TYP_AUTOMATIC || cmd_type == TYP_REAL_TIME)
    {
        plannedTpg.TpgToString(string);
        fprintf(ofp, "%s (%s - %s)\n", mnemonic, GetCmdTypeString(cmd_type),
            string);
    }
    else
    {
        fprintf(ofp, "%s (%s)\n", mnemonic, GetCmdTypeString(cmd_type));
    }

    //-----------------------
    // data filename or data 
    //-----------------------

    if (dataFilename)
    {
        switch (commandId)
        {

        case EA_CMD_SCPSTTBL:
            fprintf(ofp, " Standby PRF Table: %s\n", dataFilename);
            break;
        case EA_CMD_SCPCLTBL:
            fprintf(ofp, " Calibration PRF Table: %s\n", dataFilename);
            break;
        case EA_CMD_SCPRCTBL:
            fprintf(ofp, " Receive Only PRF Table: %s\n", dataFilename);
            break;
        case EA_CMD_SCPWOTBL:
            fprintf(ofp, " Wind Obs PRF Table: %s\n", dataFilename);
            break;
        case EA_CMD_SCSWPATCH:
            fprintf(ofp, " CDS Flight Software Patch: %s\n", dataFilename);
            break;

        case EA_CMD_SCPRFCLK:
            fprintf(ofp, " PRF Clock Rate: %s\n", dataFilename);
            break;
        case EA_CMD_SCRGAWID:
            fprintf(ofp, " Range Gate (A) Width: %s\n", dataFilename);
            break;
        case EA_CMD_SCRGBWID:
            fprintf(ofp, " Range Gate (B) Width: %s\n", dataFilename);
            break;
        case EA_CMD_SCTRPWID:
            fprintf(ofp, " Transmit Pulse Width: %s\n", dataFilename);
            break;
        case EA_CMD_SCRCVGAN:
            fprintf(ofp, " Recvr Gain: %s\n", dataFilename);
            break;
        case EA_CMD_SCCALPER:
            fprintf(ofp, " Cal Seq Pulse Period: %s\n", dataFilename);
            break;
        case EA_CMD_SCTWTMEN:
            fprintf(ofp, " TWT Trip Monitor Enable: %s\n", dataFilename);                        break;
        case EA_CMD_SCRSRLPR:
            fprintf(ofp, " SES Rst/Rld Seq. Period: %s\n", dataFilename);
            break;
        case EA_CMD_SCITMSYN:
            fprintf(ofp, " Intr. Time Sync Interval: %s\n", dataFilename);
            break;
        case EA_CMD_SCTWTSEL:
            fprintf(ofp, " TWT T/R Select: %s\n", dataFilename);
            break;
        case EA_CMD_SCORBTIC:
            fprintf(ofp, " Timer Ticks/Orbit: %s\n", dataFilename);
            break;
        case EA_CMD_SCBDATBL:
            fprintf(ofp, " Doppler Tracking (A) Tbl Updt: %s\n", dataFilename);
            break;
        case EA_CMD_SCBDBTBL:
            fprintf(ofp, " Doppler Tracking (B) Tbl Updt: %s\n", dataFilename);
            break;
        case EA_CMD_SCBRATBL:
            fprintf(ofp, " Range Tracking (A) Tbl Updt: %s\n", dataFilename);
            break;
        case EA_CMD_SCBRBTBL:
            fprintf(ofp, " Range Tracking (B) Tbl Updt: %s\n", dataFilename);
            break;
        case EA_CMD_SCMROSTR:
            fprintf(ofp, " CDS Mem. Rdout Start: %s\n", dataFilename);
            break;
        case EA_CMD_SCMRUPLM:
            fprintf(ofp, " CDS Mem. Rdout Upper Limit: %s\n", dataFilename);
            break;
        case EA_CMD_SCMRLOLM:
            fprintf(ofp, " CDS Mem. Rdout Lower Limit: %s\n", dataFilename);
            break;

        default:
            break;
        }
    }

    //-------------------------
    // comments and originator 
    //-------------------------

    if (originator)
        fprintf(ofp, "  Originator: %s\n", originator);
    if (comments)
        fprintf(ofp, "  Comments: %s\n", comments);

    //-------------------
    // effect and status 
    //-------------------

    fprintf(ofp, "  Effect: %s     Status: %s\n", EffectString(),
        CmdStatusString());

    //---------------
    // expected time 
    //---------------

    if (expectedTime != INVALID_TIME)
    {
        expectedTime.ItimeToCodeA(string);
        fprintf(ofp, "  Expected Time: %s\n", string);
    }

    //--------------------
    // verification times 
    //--------------------

    fprintf(ofp, "  L1A: ");
    switch(l1aVerify)
    {
        case VER_NO: fprintf(ofp, "Unverified"); break;
        case VER_YES: fprintf(ofp, "Verified"); break;
        case VER_NA: fprintf(ofp, "N/A"); break;
        case VER_OK: fprintf(ofp, "OK"); break;
        default: break;
    }
    l1aTime.ItimeToCodeA(string);
    fprintf(ofp, ", %s\n", string);

    fprintf(ofp, "  HK2: ");
    switch(hk2Verify)
    {
        case VER_NO: fprintf(ofp, "Unverified"); break;
        case VER_YES: fprintf(ofp, "Verified"); break;
        case VER_NA: fprintf(ofp, "N/A"); break;
        case VER_OK: fprintf(ofp, "OK"); break;
        default: break;
    }
    hk2Time.ItimeToCodeA(string);
    fprintf(ofp, ", %s\n", string);

    fprintf(ofp, "  L1AP: ");
    switch(l1apVerify)
    {
        case VER_NO: fprintf(ofp, "Unverified"); break;
        case VER_YES: fprintf(ofp, "Verified"); break;
        case VER_NA: fprintf(ofp, "N/A"); break;
        case VER_OK: fprintf(ofp, "OK"); break;
        default: break;
    }
    l1apTime.ItimeToCodeA(string);
    fprintf(ofp, ", %s\n", string);


    return (_status);
}

//-----------------
// SetDataFilename 
//-----------------

int
Command::SetDataFilename(
    const char*     string)
{
    free(dataFilename);     // in case it is non-NULL
    if (string == NULL)
    {
        dataFilename = NULL;
        return(1);
    }
    else
    {
        dataFilename = strdup(string);
        if (dataFilename == NULL)
            return(0);
        else
            return(1);
    }
}

//---------------
// SetOriginator 
//---------------

int
Command::SetOriginator(
    const char*     string)
{
    free(originator);       // in case it is non-NULL
    if (string == NULL)
    {
        originator = NULL;
        return(1);
    }
    else
    {
        originator = strdup(string);
        if (originator == NULL)
            return(0);
        else
            return(1);
    }
}

//-------------
// SetComments 
//-------------

int
Command::SetComments(
    const char*     string)
{
    free(comments);     // in case it is non-NULL
    if (string == NULL)
    {
        comments = NULL;
        return(1);
    }
    else
    {
        comments = strdup(string);
        if (comments == NULL)
            return(0);
        else
            return(1);
    }
}

//=============================================
// ROUTINE: ToReqiString
// PURPOSE: Return a pointer to the string to 
//          be written to the REQI file. 
//=============================================
int
Command::ToReqiString(
char*   string)     // user shall provide space
{
    char tempString[BIG_SIZE];

    //-----------------------------------
    // write the date and cmd mnemonic
    //-----------------------------------
    if (_commandType == AUTOMATIC_COMMAND)
    {
        if (_format == USE_PATH_FORMAT)
        {
            if (plannedTpg.time.ItimeToCodeADate(tempString) == 0)
                return (0);
            (void) sprintf(string, "%s %s %d %6.2f", tempString,
                            mnemonic, plannedTpg.path, plannedTpg.gamma);
        }
        else
        {
            if (plannedTpg.time.ItimeToCodeASecond(tempString) == 0)
                return (0);
            (void) sprintf(string, "%s %s", tempString, mnemonic);
        }
    }
    else
    {
        if (_format == USE_PATH_FORMAT)
        {
            if (plannedTpg.time.ItimeToCodeADate(tempString) == 0)
                return (0);
            (void) sprintf(string, "%s %s %d", tempString,
                            mnemonic, plannedTpg.path);
        }
        else
        {
            if (plannedTpg.time.ItimeToCodeAHour(tempString) == 0)
                return (0);
            (void) sprintf(string, "%s %s", tempString, mnemonic);
        }
    }

    //-----------------------------------
    // if datafile is specified, write it
    //-----------------------------------
    if (dataFilename != NULL)
    {
       (void)strcat(string, " ");
       (void)strcat(string,dataFilename);
    }
    (void)strcat(string, "\n");
   
     if (_GetOptionalReqiString(tempString))
         (void)strcat(string, tempString);
     else
         return 0;

    return 1;

}//Command::ToReqiString

//--------------
// EffectString 
//--------------

const char*
Command::EffectString()
{
    return(GetEffectString(effectId));
}

//-----------------
// GetEffectString 
//-----------------

const char*
Command::GetEffectString(
    EffectE     targetEffectId)
{
    for (int i = 0; effect_mnemonics[i].mnemonic; i++)
    {
        if (effect_mnemonics[i].effectId == targetEffectId)
            return (effect_mnemonics[i].mnemonic);
    }
    return (0);
}

//------------------
// GetCmdTypeString 
//------------------

const char*
Command::GetCmdTypeString(
    CmdTypeE    targetCmdType)
{
    for (int i = 0; cmd_type_mnemonics[i].mnemonic; i++)
    {
        if (cmd_type_mnemonics[i].commandType == targetCmdType)
            return (cmd_type_mnemonics[i].mnemonic);
    }
    return (0);
}

//-----------------
// CmdStatusString 
//-----------------

const char*
Command::CmdStatusString()
{
    for (int i = 0; cmd_status_mnemonics[i].mnemonic; i++)
    {
        if (cmd_status_mnemonics[i].cmdStatusId == statusId)
            return (cmd_status_mnemonics[i].mnemonic);
    }
    return (0);
}

//-----------------
// CmdStatusAbbrev 
//-----------------

const char*
Command::CmdStatusAbbrev()
{
    for (int i = 0; cmd_status_mnemonics[i].abbreviation; i++)
    {
        if (cmd_status_mnemonics[i].cmdStatusId == statusId)
            return (cmd_status_mnemonics[i].abbreviation);
    }
    return (0);
}

//--------------
// EarliestTime 
//--------------
// returns the earliest verified time or the expected time
// (if no verified times exist)

Itime
Command::EarliestTime()
const
{
    Itime earliest_time = INVALID_TIME;

    //---------------------------------
    // find the earliest verified time 
    //---------------------------------

    if (l1aTime != INVALID_TIME)
    {
        if (earliest_time == INVALID_TIME || l1aTime < earliest_time)
            earliest_time = l1aTime;
    }
    if (hk2Time != INVALID_TIME)
    {
        if (earliest_time == INVALID_TIME || hk2Time < earliest_time)
            earliest_time = hk2Time;
    }
    if (l1apTime != INVALID_TIME)
    {
        if (earliest_time == INVALID_TIME || l1apTime < earliest_time)
            earliest_time = l1apTime;
    }
    if (earliest_time != INVALID_TIME)
        return(earliest_time);

    //------------------------------------------------
    // no valid verified times, try the expected time 
    //------------------------------------------------

    if (expectedTime != INVALID_TIME)
        return(expectedTime);

    return(INVALID_TIME);
}

//------------
// LatestTime 
//------------
// returns the latest verified time or the expected time
// (if no verified times exist)

Itime
Command::LatestTime()
const
{
    Itime latest_time = INVALID_TIME;

    //-------------------------------
    // find the latest verified time 
    //-------------------------------

    if (l1aTime != INVALID_TIME)
    {
        if (latest_time == INVALID_TIME || l1aTime > latest_time)
            latest_time = l1aTime;
    }
    if (hk2Time != INVALID_TIME)
    {
        if (latest_time == INVALID_TIME || hk2Time > latest_time)
            latest_time = hk2Time;
    }
    if (l1apTime != INVALID_TIME)
    {
        if (latest_time == INVALID_TIME || l1apTime > latest_time)
            latest_time = l1apTime;
    }
    if (latest_time != INVALID_TIME)
        return(latest_time);

    //------------------------------------------------
    // no valid verified times, try the expected time 
    //------------------------------------------------

    if (expectedTime != INVALID_TIME)
        return(expectedTime);

    return(INVALID_TIME);
}

//----------
// BestTime 
//----------
// returns what is believed to be the most accurate time of the command

Itime
Command::BestTime()
const
{
    Itime iv_time = INVALID_TIME;
    if (l1aTime != iv_time)
        return(l1aTime);
    if (l1apTime != iv_time)
        return(l1apTime);
    if (hk2Time != iv_time)
        return(hk2Time);
    if (expectedTime != iv_time)
        return(expectedTime);
    if (plannedTpg.time != iv_time)
        return(plannedTpg.time);
    return(iv_time);
}

//---------
// CmdType 
//---------
// returns the type of command based on the plannedTpg

CmdTypeE
Command::CmdType()
{
    if (plannedTpg.path == INVALID_PATH)
    {
        if (plannedTpg.time == INVALID_TIME)
            return(TYP_UNPLANNED);
        else
            return(TYP_REAL_TIME);
    }
    else    // path is valid
    {
        if (plannedTpg.time == INVALID_TIME)
            return(TYP_UNKNOWN);
        else
            return(TYP_AUTOMATIC);
    }
    return(TYP_UNKNOWN);
}

//-----------
// Matchable 
//-----------
// returns 1 if the passed effect can be matched with the given command

int
Command::Matchable(
    EACommandE    command_id,
    EffectE     effect_id)
{
    //--------------------------------
    // allowable effects for commands 
    //--------------------------------

     static const EffectE k1_k2_k3_set[] = { EFF_UNKNOWN, 
                                             EFF_ALL_OFF, 
                                             EFF_RHM,
                                             EFF_NONE, EFF_MOOT, 
                                             NUM_EFFECTS };
     static const EffectE k1_k2_k3_reset[] = { EFF_UNKNOWN, 
                                               EFF_ELECTRONICS_ON,
                                               EFF_NONE, 
                                               EFF_MOOT, 
                                               NUM_EFFECTS };
     static const EffectE k4_k5_k6_set[] = { EFF_UNKNOWN, 
                                             EFF_ALL_OFF,
                                             EFF_REPLACEMENT_HEATERS_OFF, 
                                             EFF_NONE, EFF_MOOT, 
                                             NUM_EFFECTS };
     static const EffectE k4_k5_k6_reset[] = { EFF_UNKNOWN, 
                                               EFF_RHM,
                                               EFF_REPLACEMENT_HEATERS_ON, 
                                               EFF_NONE, EFF_MOOT, 
                                               NUM_EFFECTS };

    static const EffectE k7_k8_switch[] = { EFF_UNKNOWN, EFF_CDS_A, EFF_CDS_B,
        EFF_NONE, NUM_EFFECTS };
    static const EffectE k9_k10_switch[] = { EFF_UNKNOWN, EFF_TWTA_ON,
        EFF_TWTA_REPL_HEATER_ON, EFF_NONE, NUM_EFFECTS };
    static const EffectE k11_k12_switch[] = { EFF_UNKNOWN, EFF_TWTA_1,
        EFF_TWTA_2, EFF_NONE, NUM_EFFECTS };
    static const EffectE k13_k14_switch[] = { EFF_UNKNOWN,
        EFF_SES_ELECTRONICS_ON, EFF_SES_REPL_HEATER_ON, EFF_NONE,
        NUM_EFFECTS };
    static const EffectE k15_k16_switch[] = { EFF_UNKNOWN, EFF_SES_A, EFF_SES_B,
        EFF_NONE, NUM_EFFECTS };
    static const EffectE k17_k18_switch[] = { EFF_UNKNOWN,
        EFF_SAS_ELECTRONICS_ON, EFF_SAS_REPL_HEATER_ON, EFF_NONE, 
        NUM_EFFECTS };
    static const EffectE k19_k20_switch[] = { EFF_UNKNOWN, EFF_SAS_A, EFF_SAS_B,
        EFF_NONE, NUM_EFFECTS };
    static const EffectE k21_k22_switch[] = { EFF_UNKNOWN, 
        EFF_SES_SUPPL_HEATER_ON, EFF_SES_SUPPL_HEATER_OFF, NUM_EFFECTS };
    static const EffectE mode_stb[] = { EFF_UNKNOWN, EFF_STB, 
        EFF_NONE, NUM_EFFECTS };
    static const EffectE mode_rcv[] = { EFF_UNKNOWN, EFF_RCV, EFF_NONE,
        NUM_EFFECTS };
    static const EffectE mode_cal[] = { EFF_UNKNOWN, EFF_CAL, EFF_NONE,
        NUM_EFFECTS };
    static const EffectE mode_wom[] = { EFF_UNKNOWN, EFF_WOM, EFF_NONE,
        NUM_EFFECTS };
    //    static const EffectE bin_load[] = { EFF_UNKNOWN,
    //        EFF_NEW_BINNING_CONSTANTS, NUM_EFFECTS };
    //    static const EffectE ant_seq[] = { EFF_UNKNOWN, EFF_NEW_ANTENNA_SEQUENCE,
    //        NUM_EFFECTS };
    static const EffectE trs[] = { EFF_UNKNOWN, EFF_TRS_CHANGE, NUM_EFFECTS };
    static const EffectE mon[] = { EFF_UNKNOWN, EFF_TWTA_MONITOR_CHANGE,
                                   NUM_EFFECTS };
    static const EffectE ovr[] = { EFF_UNKNOWN,
                                   EFF_TWT_TRIP_OVERRIDE_ENABLE, 
                                   EFF_TWT_TRIP_OVERRIDE_DISABLE, 
                                   NUM_EFFECTS };
    //    static const EffectE caret[] = { EFF_UNKNOWN,
    //                                     EFF_HVPS_OFF, NUM_EFFECTS };
    static const EffectE sas_spin_rate[] = { EFF_UNKNOWN, 
                                             EFF_SAS_A_SPIN_RATE_198,
                                             EFF_SAS_A_SPIN_RATE_180,
                                             EFF_SAS_B_SPIN_RATE_198,
                                             EFF_SAS_B_SPIN_RATE_180,
                                             NUM_EFFECTS };

    static const EffectE range_gate_width[] = { EFF_UNKNOWN,
                                                EFF_RANGE_GATE_A_WIDTH_CHANGE,
                                                EFF_RANGE_GATE_B_WIDTH_CHANGE,
                                                NUM_EFFECTS};
#if 0
    static const EffectE prf_clock[] = {EFF_UNKNOWN,
                                  EFF_PRF_CLOCK_CHANGE,
                                  NUM_EFFECTS};
#endif
    

    static const EffectE twta_lowdrivefp[] = {EFF_UNKNOWN,
                                         EFF_TWTA_LOWDRIVE_POWER_FP_ON,
                                         EFF_TWTA_LOWDRIVE_POWER_FP_OFF,
                                         NUM_EFFECTS};

    static const EffectE transmit_pulse[] = {EFF_UNKNOWN,
                                             EFF_TRANSMIT_PULSE_WIDTH_CHANGE,
                                             NUM_EFFECTS};

    static const EffectE recv_gain[] = {EFF_UNKNOWN,
                                        EFF_RECEIVER_GAIN_CHANGE,
                                        NUM_EFFECTS};
   
    static const EffectE grid[] = {EFF_UNKNOWN,
                                   EFF_GRID_NORMAL,
                                   EFF_GRID_DISABLED,
                                   NUM_EFFECTS};

    static const EffectE recv_protect[] = {EFF_UNKNOWN,
                                   EFF_RECEIVE_PROTECT_NORMAL,
                                   EFF_RECEIVE_PROTECT_ON,
                                   NUM_EFFECTS};

    static const EffectE modulation[] = {EFF_UNKNOWN,
                                         EFF_MODULATION_ON,
                                         EFF_MODULATION_OFF,
                                         NUM_EFFECTS};

    
    static const EffectE none[] = { NUM_EFFECTS };
    
    //--------------------------------------
    // unknown commands match with anything 
    //--------------------------------------
    
    if (command_id == EA_CMD_UNKNOWN)
        return(1);
    
    //-------------------------------------
    // select the appropriate effect array 
    //-------------------------------------
    
    // select the array to use
    const EffectE* effect_array;
    switch(command_id) {


        //--------------
        // power relays 
        //--------------
        
     case EA_CMD_SCK1SET: 
     case EA_CMD_SCK2SET: 
     case EA_CMD_SCK3SET:
         effect_array = k1_k2_k3_set;
         break;
     case EA_CMD_SCK1RST: 
     case EA_CMD_SCK2RST: 
     case EA_CMD_SCK3RST:
         effect_array = k1_k2_k3_reset;
         break;
      
         //---------------------------
         // replacement heater relays 
         //---------------------------
      
     case EA_CMD_SCK4SET: 
     case EA_CMD_SCK5SET: 
     case EA_CMD_SCK6SET:
         effect_array = k4_k5_k6_set;
         break;
     case EA_CMD_SCK4RST: 
     case EA_CMD_SCK5RST: 
     case EA_CMD_SCK6RST:
         effect_array = k4_k5_k6_reset;
         break;
      
      //------------
      // cds relays 
      //------------
      
     case EA_CMD_SCK7SET: 
     case EA_CMD_SCK7RST: 
     case EA_CMD_SCK8SET: 
     case EA_CMD_SCK8RST:
         effect_array = k7_k8_switch;
         break;
        
        //-------------
        // twta on/off relays 
        //-------------
        
    case EA_CMD_SCK9SET: 
    case EA_CMD_SCK9RST: 
    case EA_CMD_SCK10SET: 
    case EA_CMD_SCK10RST:
        effect_array = k9_k10_switch;
        break;
        
        //-------------
        // twta selection relays 
        //-------------
        
    case EA_CMD_SCK11SET: 
    case EA_CMD_SCK11RST: 
    case EA_CMD_SCK12SET: 
    case EA_CMD_SCK12RST:
        effect_array = k11_k12_switch;
        break;

        
        //---------------------
        // SES Electronics/Replacement Heater relays 
        //---------------------
        
    case EA_CMD_SCK13SET: 
    case EA_CMD_SCK13RST: 
    case EA_CMD_SCK14SET: 
    case EA_CMD_SCK14RST:
        effect_array = k13_k14_switch;
        break;

        
        //---------------------
        // SES A/B Switch
        //---------------------
        
    case EA_CMD_SCK15SET: 
    case EA_CMD_SCK15RST: 
    case EA_CMD_SCK16SET: 
    case EA_CMD_SCK16RST:
        effect_array = k15_k16_switch;
        break;
        
        
        //---------------------
        // SAS Electronics/Replacement Heater relays 
        //---------------------
        
    case EA_CMD_SCK17SET: 
    case EA_CMD_SCK17RST: 
    case EA_CMD_SCK18SET: 
    case EA_CMD_SCK18RST:
        effect_array = k17_k18_switch;
        break;

        //---------------------
        // SAS A/B 
        //---------------------
        
    case EA_CMD_SCK19SET: 
    case EA_CMD_SCK19RST: 
    case EA_CMD_SCK20SET: 
    case EA_CMD_SCK20RST:
        effect_array = k19_k20_switch;
        break;

        //---------------------
        // SES Supplemental HEATER On/Off
        //---------------------
        
    case EA_CMD_SCK21SET: 
    case EA_CMD_SCK21RST: 
    case EA_CMD_SCK22SET: 
    case EA_CMD_SCK22RST:
        effect_array = k21_k22_switch;
        break;

        //-------
        // modes 
        //-------
        
    case EA_CMD_SCMODSTB:
        effect_array = mode_stb;
        break;
    case EA_CMD_SCMODRCV:
        effect_array = mode_rcv;
        break;
    case EA_CMD_SCMODCAL:
        effect_array = mode_cal;
        break;
    case EA_CMD_SCMODWOM:
        effect_array = mode_wom;
        break;
        
        //-----
        // trs 
        //-----
    case EA_CMD_SCTWTSEL:
        effect_array = trs;
        break;

        //----------
        // twt monitors
        //----------
    case EA_CMD_SCTWTMDS:
    case EA_CMD_SCTWTMEN:
        effect_array = mon;
        break;

        //----------
        // trip overide
        //-----------
    case EA_CMD_SCTOVRON:
    case EA_CMD_SCTOVROFF:
        effect_array = ovr;
        break;

        
        //----------
        // SAS Spin rate
        //-----------
    case EA_CMD_SCSSASET:
    case EA_CMD_SCSSBSET:
    case EA_CMD_SCSSARST:
    case EA_CMD_SCSSBRST:
        effect_array=sas_spin_rate;
        break;


        //---------------
        // Range gate width
        //---------------

    case EA_CMD_SCRGAWID:
    case EA_CMD_SCRGBWID:
        effect_array=range_gate_width;
        break;


        //---------------
        // Transmit pulse width
        //---------------
       
    case EA_CMD_SCTRPWID:
    case EA_CMD_SCGATEWID:
        effect_array=transmit_pulse;
        break;

        //---------------
        // Receiver Gain
        //---------------
        
    case EA_CMD_SCRCVGAN:
        effect_array=recv_gain;
        break;

        //---------------
        // Grid normal/on
        //---------------
        

    case EA_CMD_SCGRDNOR:
    case EA_CMD_SCGRDDS:
        effect_array=grid;
        break;

        //---------------
        // Receive protect/on
        //---------------
        

    case EA_CMD_SCRCVPON:
    case EA_CMD_SCRCVPNR:
        effect_array=recv_protect;
        break;

        //---------------
        // Modulation on/off
        //---------------
        

    case EA_CMD_SCTMDON:
    case EA_CMD_SCTMDOFF:
        effect_array=modulation;
        break;

        //---------------
        // TWTA Low Drive power Fault Protection enable/disable.
        //---------------
        

    case EA_CMD_SCTLDFPEN:
    case EA_CMD_SCTLDFPDS:
        effect_array=twta_lowdrivefp;
        break;
        //---------------
        // anything else 
        //---------------
        
    default:
        effect_array = none;
        break;
    }
    
    //------------------
    // check the effect 
    //------------------
    
    for (int i = 0; effect_array[i] != NUM_EFFECTS; i++) {
        if (effect_array[i] == effect_id)
            return(1);
    }
    return(0);

}//Command::Matchable

//------------------
// UpdateWithEffect 
//------------------

int
Command::UpdateWithEffect(
    Command*    effect_cmd)
{
    // make sure the command is matchable
    if (! Matchable(commandId, effect_cmd->effectId))
        return(0);

    // verify that a single time is given for the effect
    int verify_count = 0;
    if (effect_cmd->l1aTime != INVALID_TIME)
        verify_count++;
    if (effect_cmd->hk2Time != INVALID_TIME)
        verify_count++;
    if (effect_cmd->l1apTime != INVALID_TIME)
        verify_count++;
    if (verify_count != 1)
        return(0);

    // don't stomp over any effect other than unknown
    if (effectId != EFF_UNKNOWN && effectId != effect_cmd->effectId)
        return(0);

    if (effect_cmd->l1aTime != INVALID_TIME)
    {
        if (l1aTime == INVALID_TIME)
        {
            l1aTime = effect_cmd->l1aTime;
            effectId = effect_cmd->effectId;
            l1aVerify = VER_YES;
            return(1);
        }
        else
            return(0);
    }
    if (effect_cmd->hk2Time != INVALID_TIME)
    {
        if (hk2Time == INVALID_TIME)
        {
            hk2Time = effect_cmd->hk2Time;
            effectId = effect_cmd->effectId;
            hk2Verify = VER_YES;
            return(1);
        }
        else
            return(0);
    }
    if (effect_cmd->l1apTime != INVALID_TIME)
    {
        if (l1apTime == INVALID_TIME)
        {
            l1apTime = effect_cmd->l1apTime;
            effectId = effect_cmd->effectId;
            l1apVerify = VER_YES;
            return(1);
        }
        else
            return(0);
    }
    return(0);
}

//-----------
// Effective 
//-----------
// returns 1 if the command is effective (actually has or will do something)
// returns 0 otherwise

int
Command::Effective()
{
    if (commandId == EA_CMD_UNKNOWN || commandId == EA_CMD_NONE)
        return(0);

    if (statusId == STA_OK ||
        statusId == STA_UNEXPECTED ||
        statusId == STA_ANOMALOUS)
    {
        return(1);
    }

    return(0);
}

//--------------
// _WriteMarker 
//--------------

void
Command::_WriteMarker(
    FILE*   ofp,
    char*   marker)
{
    fprintf(ofp, "%s\n", marker);
    return;
}

//-----------
// _WriteInt 
//-----------

void
Command::_WriteInt(
    FILE*   ofp,
    char*   label,
    int     value)
{
    fprintf(ofp, "%s", label);
    if (value == INVALID_INT)
        fprintf(ofp, "\n");
    else
        fprintf(ofp, " %d\n", value);
    return;
}


//-----------
// _WriteTpg 
//-----------

void
Command::_WriteTpg(
    FILE*   ofp,
    char*   label,
    Tpg     tpg)
{
    fprintf(ofp, "%s", label);
    char tpg_string[TPG_LEN];
    if (tpg.TpgToString(tpg_string))
        fprintf(ofp, " %s\n", tpg_string);
    else
        fprintf(ofp, "\n");
    return;
}

//--------------
// _WriteString 
//--------------

void
Command::_WriteString(
    FILE*   ofp,
    char*   label,
    char*   string)
{
    fprintf(ofp, "%s", label);
    if (string)
        fprintf(ofp, " %s\n", string);
    else
        fprintf(ofp, "\n");
    return;
}

//------------
// _WriteTime 
//------------

void
Command::_WriteTime(
    FILE*   ofp,
    char*   label,
    Itime   itime)
{
    fprintf(ofp, "%s", label);
    if (itime == INVALID_TIME)
        fprintf(ofp, "\n");
    else
    {
        char string[CODEA_TIME_LEN];
        itime.ItimeToCodeA(string);
        fprintf(ofp, " %s\n", string);
    }
    return;
}

//-------------
// _ReadMarker 
//-------------

int
Command::_ReadMarker(
    FILE*   ifp,
    char*   marker)
{
    char line[256];
    if (fgets(line, 256, ifp) != line)
    {
        if (feof(ifp))
            return (-1);
        else
            return (0);
    }

    char marker_string[256];
    int retval = sscanf(line, " %s", marker_string);
    switch (retval)
    {
    case 1:
        if (strcmp(marker_string, marker) != 0)
            return (0);
        else
            return (1);
        break;
    default:
        return (0);
    }
}

//----------
// _ReadInt 
//----------

int
Command::_ReadInt(
    FILE*   ifp,
    char*   label,
    int*    value)
{
    char line[256];
    if (fgets(line, 256, ifp) != line)
        return (0);

    char label_string[80];
    int retval = sscanf(line, " %s %d", label_string, value);
    switch (retval)
    {
    case 2:
        if (strcmp(label_string, label) != 0)
            return (0);
        else
            return (1);
        break;
    case 1:
        if (strcmp(label_string, label) != 0)
            return (0);
        else
        {
            *value = INVALID_INT;
            return (1);
        }
        break;
    default:
        return (0);
        break;
    }
}



//----------
// _ReadInt 
//----------

int
Command::_ReadShort(
    FILE*   ifp,
    char*   label,
    short int*  value)
{
    char line[256];
    if (fgets(line, 256, ifp) != line)
        return (0);

    char label_string[80];
    int retval = sscanf(line, " %s %hd", label_string, value);
    switch (retval)
    {
    case 2:
        if (strcmp(label_string, label) != 0)
            return (0);
        else
            return (1);
        break;
    case 1:
        if (strcmp(label_string, label) != 0)
            return (0);
        else
        {
            *value = INVALID_INT;
            return (1);
        }
        break;
    default:
        return (0);
        break;
    }
}


//----------
// _ReadTpg 
//----------

int
Command::_ReadTpg(
    FILE*   ifp,
    char*   label,
    Tpg*    tpg)
{
    char line[256];
    if (fgets(line, 256, ifp) != line)
        return (0);

    char label_string[80], tpg_string[256];
    int retval = sscanf(line, " %s %[^\n]", label_string, tpg_string);
    switch (retval)
    {
    case 2:
        if (strcmp(label_string, label) != 0)
            return (0);
        else
            return (tpg->StringToTpg(tpg_string));
        break;
    case 1:
        if (strcmp(label_string, label) != 0)
            return (0);
        else
        {
            *tpg = INVALID_TPG;
            return (1);
        }
        break;
    default:
        return (0);
        break;
    }
}

//-------------
// _ReadString 
//-------------

int
Command::_ReadString(
    FILE*   ifp,
    char*   label,
    char**  string)
{
    char line[MAX_STRING_SIZE];
    if (fgets(line, MAX_STRING_SIZE, ifp) != line)
        return (0);

    // fget doesn't strip out new-line char, we have to do it
    if (line[strlen(line)-1] == '\n')
        line[strlen(line)-1] = '\0';

    char label_string[80];
    int count;
    int retval = sscanf(line, " %s %n", label_string, &count);

    if (retval != 1)
        return(0);
    if (strcmp(label_string, label) != 0)
        return(0);

    free(*string);
    if (*(line+count) != '\0')
    {
        *string = strdup(line + count);
        if (*string == NULL)
            return(0);
    }
    return(1);
}

//-----------
// _ReadTime 
//-----------

int
Command::_ReadTime(
    FILE*   ifp,
    char*   label,
    Itime*  itime)
{
    char line[256];
    if (fgets(line, 256, ifp) != line)
        return (0);

    char label_string[80], time_string[256];
    int retval = sscanf(line, " %s %s", label_string, time_string);
    switch (retval)
    {
    case 2:
        if (strcmp(label_string, label) != 0)
            return (0);
        else
            return (itime->CodeAToItime(time_string));
        break;
    case 1:
        if (strcmp(label_string, label) != 0)
            return (0);
        else
        {
            *itime = INVALID_TIME;
            return (1);
        }
        break;
    default:
        return (0);
        break;
    }
}

//==============================================
// ROUTINE: MnemonicToCmdId
// PURPOSE: Returns the command id
//          given the mnemonic string. (RER)
//==============================================
// static
EACommandE
Command::MnemonicToCmdId(
const char*       string,    // IN
unsigned short&   p_cmdHex)  // OUT
{
    for (int i = 0; i < numCmdArgsEntries; i++)
    {
        if (strcmp(cmdArgsTable[i].mnemonic, string) == 0)
        {
            p_cmdHex = cmdArgsTable[i].cmdHex;
            return(cmdArgsTable[i].cmdID);
        }
    }
    return(EA_CMD_UNKNOWN); 
}

// static
EACommandE
Command::CmdHexToCmdId(
unsigned short    p_cmdHex)  // IN
{
    for (int i = 0; i < numCmdArgsEntries; i++)
    {
        if (p_cmdHex == cmdArgsTable[i].cmdHex)
            return(cmdArgsTable[i].cmdID);
    }
    return(EA_CMD_UNKNOWN);

} // Command::CmdHexToCmdId

// static
unsigned short
Command::CmdIdToCmdHex(
EACommandE   p_cmdId)  // IN
{
    for (int i = 0; i < numCmdArgsEntries; i++)
    {
        if (p_cmdId == cmdArgsTable[i].cmdID)
            return(cmdArgsTable[i].cmdHex);
    }
    return(0);

} // Command::CmdIdToCmdHex

//-----------
// operators 
//-----------

int
operator<(
    const Command& a,
    const Command& b)
{
    return (a.EarliestTime() < b.EarliestTime());
}

int
operator<=(
    const Command& a,
    const Command& b)
{
    Itime atime = a.EarliestTime();
    Itime btime = b.EarliestTime();
    return (atime < btime || atime == btime);
}

int
operator>(
    const Command& a,
    const Command& b)
{
    return (a.EarliestTime() > b.EarliestTime());
}

int
operator>=(
    const Command& a,
    const Command& b)
{
    Itime atime = a.EarliestTime();
    Itime btime = b.EarliestTime();
    return (atime > btime || atime == btime);
}

int
operator==(
    const Command&  a,
    const Command&  b)
{
    // convert to strings
    char* adf = CONVERT_TO_STRING(a.dataFilename);
    char* bdf = CONVERT_TO_STRING(b.dataFilename);
    char* aqx = CONVERT_TO_STRING(a.qpx_filename);
    char* bqx = CONVERT_TO_STRING(b.qpx_filename);
    char* ao = CONVERT_TO_STRING(a.originator);
    char* bo = CONVERT_TO_STRING(b.originator);
    char* ac = CONVERT_TO_STRING(a.comments);
    char* bc = CONVERT_TO_STRING(b.comments);
    return(
        a.commandId == b.commandId &&
        a.plannedTpg == b.plannedTpg &&
        strcmp(adf, bdf) == 0 &&
        strcmp(aqx,bqx) == 0 &&
        strcmp(ao, bo) == 0 &&
        strcmp(ac, bc) == 0 &&
        a.effectId == b.effectId &&
        a.statusId == b.statusId &&
        a.expectedTime == b.expectedTime &&
        a.l1aTime == b.l1aTime &&
        a.hk2Time == b.hk2Time &&
        a.l1apTime == b.l1apTime &&
        a.l1aVerify == b.l1aVerify &&
        a.hk2Verify == b.hk2Verify &&
        a.l1apVerify == b.l1apVerify &&
        a.effect_value==b.effect_value &&
        a.table_type==b.table_type &&
        a.tableid==b.tableid);
}

const char*
Command::_VerifyIdToMnemonic(
const VerifyE       id)
{
    for (int i = 0; cmd_verify_mnemonics[i].mnemonic; i++)
    {
        if (cmd_verify_mnemonics[i].cmdVerifyId == id)
        {
            return(cmd_verify_mnemonics[i].mnemonic);
        }
    }

    return(cmd_verify_mnemonics[0].mnemonic);

} //Command::_VerifyIdToMnemonic

//-----------------------------------------------
// ROUTINE: MnemonicToEffectId
// PURPOSE: Returns the effect id
//          given the mnemonic string.
//-----------------------------------------------
// static
EffectE
Command::MnemonicToEffectId(
char*   string)
{
    for (int i = 0; effect_mnemonics[i].mnemonic; i++)
    {
        if (strcmp(effect_mnemonics[i].mnemonic, string) == 0)
            return(effect_mnemonics[i].effectId);
    }
    return(effect_mnemonics[0].effectId); 

} //Command::MnemonicToEffectId

//-----------------------------------------------
// ROUTINE: MnemonicToStatusId
// PURPOSE: Returns the command status id
//          given the mnemonic string.
//-----------------------------------------------
// static
CmdStatusE
Command::MnemonicToStatusId(
char*   string)
{
    for (int i = 0; cmd_status_mnemonics[i].mnemonic; i++)
    {
        if (strcmp(cmd_status_mnemonics[i].mnemonic, string) == 0)
            return(cmd_status_mnemonics[i].cmdStatusId);
    }
    return(cmd_status_mnemonics[0].cmdStatusId); 

} //Command::MnemonicToStatusId

//-----------------------------------------------
// ROUTINE: MnemonicToVerifyId
// PURPOSE: Returns the command status id
//          given the mnemonic string.
//-----------------------------------------------
// static
VerifyE
Command::MnemonicToVerifyId(
char*   string)
{
    for (int i = 0; cmd_verify_mnemonics[i].mnemonic; i++)
    {
        if (strcmp(cmd_verify_mnemonics[i].mnemonic, string) == 0)
            return(cmd_verify_mnemonics[i].cmdVerifyId);
    }
    return(cmd_verify_mnemonics[0].cmdVerifyId); 

} //Command::MnemonicToVerifyId

//=============================================
// ROUTINE: Get Optional REQI string (repitition, originator and comments)
// PURPOSE: Return a pointer to the string to 
//          be written to the REQI file. 
//=============================================
int
Command::_GetOptionalReqiString(
char*   string)     // user shall provide space
{
    if (string == 0) return 0;

    string[0] = '\0';
    char tempString[BIG_SIZE];

   //-----------------------------------
   // if is table command, write repetition
   //-----------------------------------
   if (GetFileFormat(this) == FILE_QPF ||
                  GetFileFormat(this) == FILE_QPF_STATIC)
   {
      sprintf(tempString,"REPETITION: %d\n", tableRepetition); 
      strcat(string, tempString); 
   }

   //-----------------------------------
   // if originator is specified, write it
   //-----------------------------------
   if (originator != 0 && originator[0] != 0)
   {
      sprintf(tempString,"ORIGINATOR: %s\n",originator); 
      strcat(string,tempString); 
   }

   //-----------------------------------
   // if comments are specified, write it
   //-----------------------------------
   if (comments != 0 && comments[0] != 0)
   {
      sprintf(tempString,"COMMENTS: %s\n",comments); 
      strcat(string,tempString); 
   }

   return 1;

}//Command::_GetOptionalReqiString

//=============================================
// ROUTINE: Read Optional REQI string (repitition, originator and comments)
//          Return TRUE if ok
//=============================================
int
Command::ReadReqiOptionalString(
int           lineNo,     // line number
const char*   string)     // optional string (with ending '\n')
{
    if (string == 0) return 0;

    char tempString[BIG_SIZE];
    int rep_count=0;

    //-----------------------------------
    // try to read it in as originator
    //-----------------------------------
    if (originator == 0 &&
               sscanf(string," ORIGINATOR:%[^\n]", tempString) == 1)
    {
        SetOriginator(tempString);
        return 1;
    }
    //-----------------------------------
    // try to read it in as comments
    //-----------------------------------
    else if (comments == 0 &&
                  sscanf(string," COMMENTS:%[^\n]", tempString) == 1)
    {
        SetComments(tempString);
        return 1;
    }
    else if (sscanf(string, " REPETITION:%d", &rep_count) == 1)
    {
        if (GetFileFormat(this) != FILE_QPF &&
                GetFileFormat(this) != FILE_QPF_STATIC)
        {
            fprintf(stderr, "\nRepetition not allowed for %s "
                            "commands at line %d\n", mnemonic, lineNo);
            return(_status = ERROR_READING_CMD);
        }
        if (rep_count < 1 && rep_count > 9)
        {
            fprintf(stderr, "\nRepetition must be btwn 1 and 9 (line %d)\n",
                        lineNo);
            return(_status = ERROR_READING_CMD);
        }
        tableRepetition = (unsigned char)rep_count;
        return 1;
    }

    fprintf(stderr, "\nLine %d: %s: unrecognized syntax\n", lineNo, string);
    return 0;

}//Command::ReadReqiOptionalString

int
Command::NeedDataFile(
EACommandE commandID)
{
    for (int i=0; i < numCmdArgsEntries; i++)
    {
        if (commandID == cmdArgsTable[i].cmdID &&
                               cmdArgsTable[i].numWordsInParamFile != 0)
            return 1;
    }
    return 0;

} // Command::NeedDataFile

int
Command::RealtimeOnly(
EACommandE commandID)
{
    for (int i=0; i < numCmdArgsEntries; i++)
    {
        // real time command takes no arg of latitude
        if (commandID == cmdArgsTable[i].cmdID &&
                            cmdArgsTable[i].realtimeOnly)
            return 1;
    }
    return 0;

} // Command::RealtimeOnly

int
Command::IsQPF(
EACommandE commandID)
{
    for (int i=0; i < numCmdArgsEntries; i++)
    {
        if (commandID == cmdArgsTable[i].cmdID)
            return cmdArgsTable[i].isQPF;
    }
    return 0;

} // Command::IsQPF

int
Command::NumWordsInParamFile(
EACommandE commandID)
{
    for (int i=0; i < numCmdArgsEntries; i++)
    {
        if (commandID == cmdArgsTable[i].cmdID)
            return cmdArgsTable[i].numWordsInParamFile;
    }
    return 0;

} // Command::NumWordsInParamFile

Command::FileFormatE
Command::GetFileFormat(
const Command*     cmd)
{
    //--------------------------------------
    // decide the commanding file format
    //--------------------------------------
    if (cmd->_isQPF)
        return(FILE_QPF);
    else if (cmd->_commandType == REALTIME_COMMAND)
        return(FILE_RTCF);
    else if (cmd->_numStaticParams > 3)
        return(FILE_QPF_STATIC);
    else if (cmd->_numStaticParams >= 0 && cmd->_numStaticParams <= 3)
        return(FILE_REQQ_STATIC);
    else
        return(FILE_REQQ);

} // Command::GetFileFormat

EADataFileFormat
Command::DatafileFormat(
EACommandE commandID)
{
    for (int i=0; i < numCmdArgsEntries; i++)
    {
        if (commandID == cmdArgsTable[i].cmdID)
            return cmdArgsTable[i].datafileFormat;
    }
    return EA_DATAFILE_NONE;

} // Command::DatafileFormat

int
Command::NumStaticParams(
EACommandE commandID)
{
    for (int i=0; i < numCmdArgsEntries; i++)
    {
        if (commandID == cmdArgsTable[i].cmdID)
            return cmdArgsTable[i].numStaticParams;
    }
    return (-1);

} // Command::NumStaticParams

const char*
Command::StaticParamString(
EACommandE commandID)
{
    for (int i=0; i < numCmdArgsEntries; i++)
    {
        if (commandID == cmdArgsTable[i].cmdID)
            return cmdArgsTable[i].staticParamString;
    }
    return ("");

} // Command::StaticParamString


//--------------//
// CheckForCmds //
//--------------//
// returns 1 if any embedded commands are found (sets the bytes, bits,
// command code, and command name)
// assumes that the first two bytes are the command

int
Command::CheckForCommands(
char*			array,
int				array_size,
int&			byte1,
int&			bit1,
int&			byte2,
int&			bit2,
EACommandE&    	cmd_code,
const char*&    cmd_name,
FILE*           output_fp)
{
    FILE* outputFP = (output_fp ? output_fp : stderr);
    int embeddedFound = 0;
	// initialize with the command
	unsigned short word = *(array) << 8 | ((*(array + 1)) & 0x000f);
	for (int byte = 2; byte < array_size; byte++)
	{
		for (int bit = 7; bit >= 0; bit--)
		{
			// shift in the next bit
			word <<= 1;
			word |= (*(array+byte) >> bit) & 0x01;

			// check against command codes
            for (int cmdIndex = 0; cmdIndex < numCmdArgsEntries; cmdIndex++)
			{
                // skip pulse discrete commands
                if ( ! (cmdArgsTable[cmdIndex].cmdID & 0xff00))
                    continue;

				if (word == cmdArgsTable[cmdIndex].cmdID)
				{
					byte2 = byte;
					bit2 = bit;
					if (bit == 0)
					{
						byte1 = byte - 1;
						bit1 = 7;
					}
					else
					{
						byte1 = byte - 2;
						bit1 = bit - 1;
					}
					cmd_code = cmdArgsTable[cmdIndex].cmdID;
					cmd_name = cmdArgsTable[cmdIndex].description;
                    fprintf(outputFP, "\n---------- WARNING ----------\n");
                    fprintf(outputFP, "  Embedded Command: %s (0x%04X)\n",
                                           cmd_name, cmd_code);
                    fprintf(outputFP, "  Command Range: byte %d, bit %d "
                                    "to byte %d, bit %d\n",
                                                byte1, bit1, byte2, bit2);
                    fprintf(outputFP, "  (Bytes start at 0 and include "
                                    "the command word.)\n");
                    fprintf(outputFP, "-----------------------------\n");
					embeddedFound = 1;
				}
			}
		}
	}
	return(embeddedFound);

} //Command::CheckForCmds

const char *
Command::CmdIdToDescript (EACommandE cmdId ) 
{
    for (int i=0; i<numCmdArgsEntries; i++){
        if ( cmdArgsTable[i].cmdID == cmdId ) 
            return( cmdArgsTable[i].description );
    }
    return("");
} // Command::CmdIdToDescript


const char *
Command::CmdIdToMnemonic ( EACommandE cmdId) 
{
    for (int i=0; i<numCmdArgsEntries; i++){
        if ( cmdArgsTable[i].cmdID == cmdId ) 
            return( cmdArgsTable[i].mnemonic );
    }
    return("");
} // Command::CmdIdToMnemonic

void
Command::SetNumWordsInParamFile(
int numWords)
{
    if (numWords >= 0)
    {
        _numWordsInParamFile = numWords;
        return;
    }
    if (dataFilename == 0)
    {
    }
    FILE* inFP = fopen(dataFilename, "r");
    if (inFP == NULL)
    {
        fprintf(stderr, "Cannot open data filename (%s)\n", dataFilename);
        return;
    }
    unsigned short data;
    int wordsInDataFile=0;
    while (fscanf(inFP, "%hx", &data) == 1)
        wordsInDataFile++;
    _numWordsInParamFile = wordsInDataFile;
    return;
} // Command::SetNumWordsInParamFile

//-----------------
// SetCmdParams
//-----------------

int
Command::SetCmdParams(
    const char*     string)
{
    free(cmdParams);     // in case it is non-NULL
    if (string == NULL)
    {
        cmdParams = NULL;
        return(1);
    }
    else
    {
        cmdParams = strdup(string);
        if (cmdParams == NULL)
            return(0);
        else
            return(1);
    }
} //Command::SetCmdParams



// int 
// Command::CmdParamsToEU( float *values ) {
//     char param1[5], param2[5];

//     switch (commandId) {
//     case EA_CMD_SCRGAWID:
//         // 1 param
//         sscanf( cmdParams, "%04X", param1);
//         values[0]=0.049903*atof(param1);
//         break;
//     case EA_CMD_SCRGBWID:
//         // 1 param
//         sscanf( cmdParams, "%04X", param1);
//         values[0]=0.049903*atof(param1);
//         break;
//     case EA_CMD_SCTRPWID:
//         // 1 param
//         sscanf( cmdParams, "%04X", param1);
//         values[0]=0.049903*atof(param1);
//         break;
//     case EA_CMD_SCPRFCLK:
//         // 1 param
//         sscanf( cmdParams, "%04X", param1);
//         values[0]=0.099806*atof(param1);
//         break;
//     case EA_CMD_SCRCVGAN:
//         // 1 param
//         sscanf( cmdParams, "%04X", param1);
//         values[0]=-1*atof(param1);
//         break;
//     case EA_CMD_SCRGATEWID:
//         // 2 param
//         sscanf( cmdParams, "%04X %04X", param1, param2);
//         values[0]=0.049903*atof(param1);
//         values[1]=0.049903*atof(param2);
//         break;
//     case EA_CMD_SCTWTSEL:
//         // 1 param
//         sscanf( cmdParams, "%04X", param1);
//         values[0] = atof(param1);
//         break;
//     case EA_CMD_SCCALPER:
//         // 1 param
//         sscanf( cmdParams, "%04X", param1);
//         values[0] = atof(param1);
//         break;
//     case EA_CMD_SCRSRLPR:
//         // 1 param
//         sscanf( cmdParams, "%04X", param1);
//         values[0] = atof(param1);
//         break;
//     case EA_CMD_SCTWTMEN:
//         // 1 param
//         sscanf( cmdParams, "%04X", param1);
//         values[0] = atof(param1);
//         break;
//     case EA_CMD_SCORBTIC:
//         // 2param
//         sscanf( cmdParams, "%04X %04X", param1, param2);
//         values[0] = atof(param1);
//         break;
//     case EA_CMD_SCITMSYN:
//         // 1 param
//         sscanf( cmdParams, "%04X", param1);
//         values[0] = atof(param1);
//         break;
//     case EA_CMD_SCSWPEN:
//         // 2 param
//         sscanf( cmdParams, "%04X %04X", param1, param2);
//         values[0] = atof(param1);
//         values[1] = atof(param2);
//         break;
//     case EA_CMD_SCSWPDS:
//         // 2 param
//         sscanf( cmdParams, "%04X %04X", param1, param2);
//         values[0] = atof(param1);
//         values[1] = atof(param2);
//         break;
//     case EA_CMD_SCBMOFST:
//         // 2 param
//         sscanf( cmdParams, "%04X %04X", param1, param2);
//         values[0] = 360.*(atof(param1)/32768.);
//         values[1] = 360.*(atof(param2)/32768.);
//         break;
//     case EA_CMD_SCSASOFS:
//         // 2 param
//         sscanf( cmdParams, "%04X %04X", param1, param2);
//         values[0] = 360.*(atof(param1)/32768.);
//         values[1] = 360.*(atof(param2)/32768.);
//         break;
//     case EA_CMD_SCTBLSTR:
//         // 1 param
//         sscanf( cmdParams, "%04X", param1);
//         values[0] = atof(param1);
//         break;
//     case EA_CMD_SCMROSTR:
//         // 2 param
//         sscanf( cmdParams, "%04X %04X", param1, param2);
//         values[0] = atof(param1);
//         values[1] = atof(param2);
//         break;
//     case EA_CMD_SCMRUPLM:
//         // 2 param
//         sscanf( cmdParams, "%04X %04X", param1, param2);
//         values[0] = atof(param1);
//         values[1] = atof(param2);
//         break;
//     case EA_CMD_SCMRLOLM:
//         // 2 param
//         sscanf( cmdParams, "%04X %04X", param1, param2);
//         values[0] = atof(param1);
//         values[1] = atof(param2);
//         break;
//     default:
//         break;
        
//     }
//     return (1);
// } // CommandCmdParamsToEU



