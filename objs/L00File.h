//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef L00FILE_H
#define L00FILE_H

static const char rcs_id_l00file_h[] =
	"@(#) $Id$";

#include "GenericFile.h"


//======================================================================
// CLASSES
//		L00File, L0FileList
//======================================================================


//======================================================================
// CLASS
//		L00File
//
// DESCRIPTION
//		The L00File object is used to read and write from a single
//		Level 0.0 data file.
//======================================================================

class L00File : public GenericFile
{
public:

	//--------------//
	// construction //
	//--------------//

	L00File();
	~L00File();

	//--------------//
	// input/output //
	//--------------//

protected:

	//-----------//
	// variables //
	//-----------//
};

#endif
