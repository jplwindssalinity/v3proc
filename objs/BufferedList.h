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


//==============//
// BufferedList //
//==============//

template <class T>
BufferedList<T>::BufferedList()
:	_inputFp(NULL), _maxNodes(0), _numNodes(0)
{
	return;
}

template <class T>
BufferedList<T>::BufferedList(
	FILE*			input_fp,
	unsigned int	max_nodes)
:	_inputFp(input_fp), _maxNodes(max_nodes), _numNodes(0)
{
	return;
}

template <class T>
BufferedList<T>::~BufferedList()
{
	CloseInputFile();

    T* data;
    List<T>::GetHead();
    while ((data=List<T>::RemoveCurrent()) != NULL)
        delete data;

    return;
}

//----------------------------//
// BufferedList::SetInputFile //
//----------------------------//

template <class T>
int
BufferedList<T>::SetInputFile(
	const char*		filename)
{
	CloseInputFile();		// in case it is open already
	_inputFp = fopen(filename, "r");
	if (_inputFp == NULL)
		return(0);

	return(1);
}

//---------------------------//
// BufferedList::SetMaxNodes //
//---------------------------//

template <class T>
int
BufferedList<T>::SetMaxNodes(
	unsigned int	max_nodes)
{
	_maxNodes = max_nodes;
	return(1);
}

//------------------------------//
// BufferedList::CloseInputFile //
//------------------------------//

template <class T>
int
BufferedList<T>::CloseInputFile()
{
	if (_inputFp)
	{
		fclose(_inputFp);
		_inputFp = NULL;
	}
	return(1);
}

//-----------------------------//
// BufferedList::GetOrReadNext //
//-----------------------------//
// The next node becomes the current node.
// Returns the data from the current node on success, 0 on failure.
// If the current node is the last node in memory, then another node
// is read from the external file.  If this exceeds the maximum number
// of nodes allowed in memory, then the head node is flushed (deallocated).

template <class T>
T*
BufferedList<T>::GetOrReadNext()
{
	// make sure there is a current node
	if (! List<T>::_current)
		List<T>::_current = List<T>::_head;

	if (List<T>::_current && List<T>::_current->next)
	{
		// There is a next node in memory, so just return it.
		return(List<T>::GetNext());
	}
	else
	{
		// At the tail (or empty list), so try to read in another node.
		T* new_data = new T;		// make a new data space
		if (new_data->Read(_inputFp))
		{
			// successful read, so Append the new data.
			Append(new_data);
			_numNodes++;
			if (_numNodes > _maxNodes)
			{
				// flush the oldest node (ie., the head).
				List<T>::GotoHead();
				T* old_data = List<T>::RemoveCurrent();
				delete old_data;
				_numNodes--;

				// go back to the tail
				List<T>::GotoTail();
			}
			return(new_data);
		}
		else
		{
			// unsuccessful read, so get rid of the newly allocated data space
			delete new_data;
			return(NULL);
		}
	}
	return(NULL);	// should never get here
}
#endif
