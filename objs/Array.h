//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef ARRAY_H
#define ARRAY_H

static const char rcs_id_array_h[] =
	"@(#) $Id$";

//======================================================================
// CLASSES
//		Array
//======================================================================

//======================================================================
// CLASS
//		Array
//
// DESCRIPTION
//		The Array object sets up multi-dimensional arrays that are accessed
//		by multi-indirection.  This is the same approach used by Numerical
//		Recipes.  It allows easy indexing with arbitrary offsets.
//		The data pointer is public so that the caller can access the array
//		without any function overhead.
//======================================================================

template <class T>
class Array
{
public:

	//--------------//
	// construction //
	//--------------//

	Array(int i1);
	Array(int i1, int i2);
	~Array();

	//-----------//
	// variables //
	//-----------//

	int dims;
	void* ptr;

};

#endif
