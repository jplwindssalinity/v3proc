//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_l2a_c[] =
	"@(#) $Id$";

#include <memory.h>
#include "L2A.h"


//=====//
// L2A //
//=====//

L2A::L2A()
:	_status(OK), _headerRead(0),_headerWritten(0)
{
	return;
}

L2A::~L2A()
{
	return;
}

//------------------//
// L2A::WriteHeader //
//------------------//

int
L2A::WriteHeader()
{
	if (_outputFp == NULL)
		return(0);

	if (! header.Write(_outputFp))
		return(0);

	_headerWritten = 1;
	return(1);
}

//------------------//
// L2A::WriteHeader //
//------------------//

int
L2A::WriteHeaderAscii()
{
	if (_outputFp == NULL)
		return(0);

	if (! header.WriteAscii(_outputFp))
		return(0);

	_headerWritten = 1;
	return(1);
}

//-----------------//
// L2A::ReadHeader //
//-----------------//

int
L2A::ReadHeader()
{
	if (_inputFp == NULL)
		return(0);

	if (! header.Read(_inputFp))
		return(0);

	_headerRead = 1;
	return(1);
}

//------------------//
// L2A::ReadDataRec //
//------------------//

int
L2A::ReadDataRec()
{
	if (_inputFp == NULL) return(0);

	if (! _headerRead)
	{
		if (! header.Read(_inputFp))
			return(0);
		_headerRead = 1;
	}
	if (! frame.Read(_inputFp))
		return(0);

	return(1);
}

//-------------------//
// L2A::ReadGroupRec //
//-------------------//

int 
L2A::ReadGroupRec(
     int rset,
     L2AFrame* frameGroup25,
     L2AFrame* frameGroup50)
  
{

  if (_inputFp == NULL) return(0);

  // read header

  if (! _headerRead)
    {
      if (! header.Read(_inputFp))
	  return(0);
      _headerRead = 1;
    }

  // previous reads or first call?
  

  if (! rset) 
    {
      if (! frameGroup25[0].Read(_inputFp))
	return(0);
      rset=1;
    }

  // read frames 1 to ctb-1 for this ati row and next ati row

  idx = 0;
  L2AFrame readFrame;
      
  while (frameGroup25[idx].ati < frameGroup25[0].ati + 2)
    {
      idx++;
      if (! readFrame.Read(_inputFp))
	return(0);
      //      frame.CopyFrame(&frameGroup25[idx], &readFrame);

    }
  
  // CombineFrames returns the number of 50km frames (and the combined frames)

  //  int nFrames=frame.CombineFrames(frameGroup25,frameGroup50);
  int nFrames=0;
  if (nFrames == 0)
    {
      nFrames=1;
      return(0);
    }

  // copy over next frame (from next ati row) and return

  //  frame.CopyFrame(&frameGroup25[0], &frameGroup25[idx]);

  return(nFrames);

}

//-------------------//
// L2A::WriteDataRec //
//-------------------//

int
L2A::WriteDataRec()
{
	if (_outputFp == NULL) return(0);

	if (! _headerWritten)
	{
		if (! header.Write(_outputFp))
			return(0);
		_headerWritten = 1;
	}

	if (! frame.Write(_outputFp))
		return(0);

	return(1);
}

//------------------------//
// L2A::WriteDataRecAscii //
//------------------------//

int
L2A::WriteDataRecAscii()
{
	if (_outputFp == NULL) return(0);

	if (! _headerWritten)
	{
		if (! header.WriteAscii(_outputFp))
			return(0);
		_headerWritten = 1;
	}

	if (! frame.WriteAscii(_outputFp))
		return(0);

	return(1);
}

//--------------------//
// L2A::ReadGSDataRec //
//--------------------//

int
L2A::ReadGSDataRec()
{
	if (_inputFp == NULL) return(0);

	if (! frame.ReadGS(_inputFp))
		return(0);

	return(1);
}
