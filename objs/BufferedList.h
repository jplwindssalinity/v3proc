//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef BUFFEREDLIST_H
#define BUFFEREDLIST_H

#include <stdio.h>
#include "List.h"

static const char rcs_id_bufferedlist_h[] =
	"@(#) $Id$";

//======================================================================
// CLASSES
//		BufferedList
//======================================================================

//======================================================================
// CLASS
//		BufferedList
//
// DESCRIPTION
//		The BufferedList object is a doubly linked list of Nodes.  The list
//		can be traversed forwards and backwards.
//======================================================================

template <class T>
class BufferedList : public List<T>
{
public:

	//--------------//
	// construction //
	//--------------//

	BufferedList();
	BufferedList(FILE *nodefile, long max_nodes);
	~BufferedList();

	//--------------------//
	// Buffered retrieval //
	//--------------------//

	T*		ReadNext();			// current = next, return T* of current

protected:

	//-----------//
	// variables //
	//-----------//

	FILE *_nodeFile;
	long _maxNodes;
	long _numNodes;
};

#endif
