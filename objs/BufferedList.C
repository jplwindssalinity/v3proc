//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_bufferedlist_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include "BufferedList.h"

//==============//
// BufferedList //
//==============//

template <class T>
BufferedList<T>::BufferedList()
{
	_nodefile = NULL;
	_max_nodes = 0;
	_num_nodes = 0;
	_head = NULL;
	_tail = NULL;
	_current = NULL;
	return;
}

template <class T>
BufferedList<T>::BufferedList(FILE *nodefile, long max_nodes)
{
	_nodefile = nodefile;
	_max_nodes = max_nodes;
	_num_nodes = 0;
	_head = NULL;
	_tail = NULL;
	_current = NULL;
	return;
}

template <class T>
BufferedList<T>::~BufferedList()
{
	if (_head != NULL)
	{
		fprintf(stderr, "BufferedList destroyed without being deallocated!\n");
	}
	return;
}

//------------------------//
// BufferedList::ReadNext //
//------------------------//
// The next node becomes the current node.
// Returns the data from the current node on success, 0 on failure.
// If the current node is the last node in memory, then another node
// is read from the external file.  If this exceeds the maximum number
// of nodes allowed in memory, then the head node is flushed (deallocated).

template <class T>
T*
BufferedList<T>::ReadNext()

{

if (_current)
{
	if (_current->next)
	{	// There is a next node in memory, so just point to it.
		_current = _current->next;
		return (GetCurrent());
	}
	else
	{	// At the tail, so try to read in another node.
		T* new_data = new T;		// make a new data space
		if (new_data->Read(_nodefile))
		{	// successful read, so Append the new data.
			Append(new_data);
			_num_nodes++;
			if (_num_nodes > _max_nodes)
			{	// flush the oldest node (ie., the head).
				_current = _head;
				T* old_data = RemoveCurrent();
				delete old_data;
				_num_nodes--;
				_current = _tail;
				return(new_data);
			}
		}
		else
		{	// unsuccessful read, so get rid of the newly allocated data space
		delete new_data;
		return(NULL);
		}
	}

}
return(NULL);
}
