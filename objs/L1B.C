//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_l1b_c[] =
	"@(#) $Id$";

#include <memory.h>
#include "L1B.h"


//=====//
// L1B //
//=====//

L1B::L1B()
:	_status(OK)
{
	return;
}

L1B::~L1B()
{
	return;
}

int
L1B::WriteDataRecAscii(){
  fprintf(_outputFp,"\n###############################################\n");
  fprintf(_outputFp,"#####                                    ######\n");
  fprintf(_outputFp,"#####      L1B Data Record               ######\n");
  fprintf(_outputFp,"#####                                    ######\n");
  fprintf(_outputFp,"###############################################\n");
  fprintf(_outputFp,"\n");
  return(frame.spotList.WriteAscii(_outputFp));
}





