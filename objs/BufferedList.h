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
	BufferedList(FILE* input_fp, unsigned int max_nodes);
	~BufferedList();

	int		SetInputFile(const char* filename);
	int		SetMaxNodes(unsigned int max_nodes);
	int		CloseInputFile();

	//--------------------//
	// Buffered retrieval //
	//--------------------//

	T*		GetOrReadNext();			// current = next, return T* of current

protected:

	//-----------//
	// variables //
	//-----------//

	FILE*			_inputFp;
	unsigned int	_maxNodes;
	unsigned int	_numNodes;
};

#endif
