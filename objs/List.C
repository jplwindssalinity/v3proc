//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_list_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include "List.h"

//======//
// Node //
//======//

template <class T>
Node<T>::Node(
	T*	new_data)
:	prev(NULL), next(NULL), data(new_data)
{
	return;
}

template <class T>
Node<T>::~Node()
{
	return;
}

//======//
// List //
//======//

template <class T>
List<T>::List()
:	_head(NULL), _tail(NULL), _current(NULL)
{
	return;
}

template <class T>
List<T>::~List()
{
	return;
}

//--------------//
// List::Append //
//--------------//
// Creates a node with data in it and appends it to the end of the list.
// If the list is not empty, the current node remains unchanged.  If the
// list is empty, the new node becomes the current node.
// Returns 1 on success, 0 on failure

template <class T>
int
List<T>::Append(
	T*	new_data)
{
	Node<T>* new_node = new Node<T>(new_data);
	if (new_node == NULL)
		return(0);

	if (_tail)
	{
		// end of list exists, just append
		new_node->next = NULL;
		new_node->prev = _tail;
		_tail->next = new_node;
		_tail = _current = new_node;
	}
	else
	{
		// list is empty
		new_node->prev = NULL;
		new_node->next = NULL;
		_head = _tail = _current = new_node;
	}
	return(1);
}

//---------------------//
// List::RemoveCurrent //
//---------------------//
// Remove the current node and return its data.  The next node
// becomes the current node.

template <class T>
T*
List<T>::RemoveCurrent()
{
	Node<T>* node = _current;
	if (_current == NULL)
		return (NULL);
	if (_current->prev)
		_current->prev->next = _current->next;
	if (_current->next)
		_current->next->prev = _current->prev;
	if (_current == _head)
		_head = _head->next;
	if (_current == _tail)
		_tail = _tail->prev;
	_current = _current->next;
	if (node)
	{
		T* data = node->data;
		delete node;
		return(data);
	}
	else
	{
		return(NULL);
	}
}

//---------------//
// List::GetHead //
//---------------//
// The head node becomes the current node.
// Returns the data from the current node on success, 0 on failure

template <class T>
T*
List<T>::GetHead()
{
	_current = _head;
	return (GetCurrent());
}

//---------------//
// List::GetTail //
//---------------//
// The tail node becomes the current node.
// Returns the data from the current node on success, 0 on failure

template <class T>
T*
List<T>::GetTail()
{
	_current = _tail;
	return (GetCurrent());
}

//------------------//
// List::GetCurrent //
//------------------//
// Returns the data from the current node on success, 0 on failure.

template <class T>
T*
List<T>::GetCurrent()
{
	if (_current)
		return (_current->data);
	else
		return (0);
}

//---------------//
// List::GetNext //
//---------------//
// The next node becomes the current node.
// Returns the data from the current node on success, 0 on failure.

template <class T>
T*
List<T>::GetNext()
{
	if (_current)
		_current = _current->next;
	return (GetCurrent());
}

//---------------//
// List::GetPrev //
//---------------//
// The previous node becomes the current node.
// Returns the data from the current node on success, 0 on failure.

template <class T>
T*
List<T>::GetPrev()
{
	if (_current)
		_current = _current->prev;
	return (GetCurrent());
}

//------//
// Find //
//------//

template <class T>
T*
List<T>::Find(
	T*	data)
{
	for (T* contents = GetHead(); contents; contents = GetNext())
	{
		if (*contents == *data)
			return(contents);
	}
	return(NULL);
}

//-----------//
// NodeCount //
//-----------//
// Returns the number of nodes in the list

template <class T>
int
List<T>::NodeCount()
{
	int count = 0;
	for (Node<T>* node = _head; node; node = node->next)
		count++;

	return(count);
}



/*
//------------//
// AppendList //
//------------//
// adds a list to the end of another

void
ListBase::AppendList(
	ListBase*	added_list)
{
	if (_tail)
	{
		// end of list exists, just append
		_tail->next = added_list->_head;
		added_list->_head->prev = _tail;
		_tail = added_list->_tail;
	}
	else
	{
		// list is empty
		_head = added_list->_head;
		_tail = added_list->_tail;
	}
	// unattach from added_list
	added_list->_head = NULL;
	added_list->_tail = NULL;
	return;
}

//---------//
// IsEmpty //
//---------//
// return 1 if the list is empty, 0 otherwise

int
ListBase::IsEmpty()
{
	if (_head)
		return(0);
	else
		return(1);
}

//---------------//
// GotoNodeIndex //
//---------------//
// set the current node to be the node with the specified index
// return 1 on success, 0 on failure

int
ListBase::GotoNodeIndex(
	int		node_index)
{
	int index = 0;
	for (NodeBase* node = _head; node; node = node->next, index++)
	{
		if (index == node_index)
		{
			_current = node;
			return(1);
		}
	}
	return(0);
}

//----------//
// _Prepend //
//----------//
// add node to the front of the list
// if the list is not empty, the current node remains unchanged
// if the list is empty, the new node becomes the current node

void
ListBase::_Prepend(
	NodeBase*	node)
{
	if (_head)
	{
		// front of list exists, just prepend
		node->next = _head;
		node->prev = 0;
		_head->prev = node;
		_head = node;
	}
	else
	{
		// list is empty
		node->prev = NULL;
		node->next = NULL;
		_head = _tail = _current = node;
	}
	return;
}

//---------------//
// _InsertBefore //
//---------------//
// insert node before the current node
// the new node becomes the current node
// if the current node is NULL, insert at the head and
// set current to be the head node

void
ListBase::_InsertBefore(
	NodeBase*	node)
{
	if (_current == NULL)
		_current = _head;

	if (_head == NULL)
	{
		// empty list
		_head = node;
		_tail = node;
		node->prev = NULL;
		node->next = NULL;
	}
	else if (_current == _head)
	{
		// node goes before head
		node->prev = _current->prev;
		node->next = _current;
		_current->prev = node;
		_head = node;
	}
	else
	{
		// node inserts
		node->prev = _current->prev;
		node->next = _current;
		_current->prev->next = node;
		_current->prev = node;
	}
	_current = node;
	return;
}

//--------------//
// _InsertAfter //
//--------------//
// insert node after the current node
// the new node becomes the current node
// if the current node is NULL, insert at the tail and
// set current to be the tail node

void
ListBase::_InsertAfter(
	NodeBase*	node)
{
	if (_current == NULL)
		_current = _tail;

	if (_head == NULL)
	{
		// empty list
		_head = node;
		_tail = node;
		node->prev = NULL;
		node->next = NULL;
	}
	else if (_current == _tail)
	{
		// node goes after tail
		node->next = NULL;
		node->prev = _tail;
		_tail->next = node;
		_tail = node;
	}
	else
	{
		// node inserts
		node->prev = _current;
		node->next = _current->next;
		_current->next = node;
		node->next->prev = node;
	}
	_current = node;
	return;
}

//----------------//
// _RemoveCurrent //
//----------------//
// remove the current node and return it
// the next node becomes the current node

NodeBase*
ListBase::_RemoveCurrent()
{
	NodeBase* current = _current;
	if (_current == NULL)
		return (NULL);
	if (_current->prev)
		_current->prev->next = _current->next;
	if (_current->next)
		_current->next->prev = _current->prev;
	if (_current == _head)
		_head = _head->next;
	if (_current == _tail)
		_tail = _tail->prev;
	_current = _current->next;
	return (current);
}

//==============//
// List methods //
//==============//

//--------//
// IsHead //
//--------//

template <class T>
int
List<T>::IsHead(
	T*	data)
{
	if (_head)
		return (data == ((Node<T>*)_head)->data ? 1 : 0);
	else
		return (0);

}

//--------//
// IsTail //
//--------//

template <class T>
int
List<T>::IsTail(
	T*	data)
{
	if (_tail)
		return (data == ((Node<T>*)_tail)->data ? 1 : 0);
	else
		return (0);

}

//----------//
// GetIndex //
//----------//

template <class T>
T*
List<T>::GetIndex(
	int		node_index)
{
	if (! GotoNodeIndex(node_index))
		return(0);

	return(GetCurrent());
}

//---------------//
// RemoveCurrent //
//---------------//
// remove the current node
// the next node becomes the current node
// return the data from the removed node

template <class T>
T*
List<T>::RemoveCurrent()
{
	// remove the current node and get it
	Node<T>* node = (Node<T>*)ListBase::_RemoveCurrent();
	if (node)
	{
		T* data = node->data;
		delete node;
		return (data);
	}
	else
	{
		return (NULL);
	}
}

//---------//
// Prepend //
//---------//
// create a node with data in it and prepend it to the front of the list
// if the list is not empty, the current node remains unchanged
// if the list is empty, the new node becomes the current node
// return 1 on success, 0 on failure

template <class T>
int
List<T>::Prepend(
	T*	new_data)
{
	Node<T>* new_node = new Node<T>(new_data);
	if (new_node == NULL)
		return(0);
	_Prepend((NodeBase*)new_node);
	return(1);
}


//--------------//
// InsertBefore //
//--------------//
// create a node with data in it and insert it before the current node
// the new node becomes the current node
// return 1 on success, 0 on failure

template <class T>
int
List<T>::InsertBefore(
	T*	new_data)
{
	Node<T>* new_node = new Node<T>(new_data);
	if (new_node == NULL)
		return(0);
	_InsertBefore((NodeBase*)new_node);
	return(1);
}

//--------------//
// InsertAfter //
//--------------//
// create a node with data in it and insert it after the current node
// the new node becomes the current node
// return 1 on success, 0 on failure

template <class T>
int
List<T>::InsertAfter(
	T*	new_data)
{
	Node<T>* new_node = new Node<T>(new_data);
	if (new_node == NULL)
		return(0);
	_InsertAfter((NodeBase*)new_node);
	return(1);
}

//-------------------//
// InsertBeforeIndex //
//-------------------//
// create a node with data in it and insert it before the node with the
// given index (indicies start at 0)
// return 1 on success, 0 on new failure or index failure

template <class T>
int
List<T>::InsertBeforeIndex(
	T*		new_data,
	int		node_index)
{
	if (! GotoNodeIndex(node_index))
		return(0);
	Node<T>* new_node = new Node<T>(new_data);
	if (new_node == NULL)
		return(0);
	_InsertBefore((NodeBase*)new_node);
	return(1);
}

//------------------//
// InsertAfterIndex //
//------------------//
// create a node with data in it and insert it after the node with the
// given index (indicies start at 0)
// return 1 on success, 0 on new failure or index failure

template <class T>
int
List<T>::InsertAfterIndex(
	T*		new_data,
	int		node_index)
{
	if (! GotoNodeIndex(node_index))
		return(0);
	Node<T>* new_node = new Node<T>(new_data);
	if (new_node == NULL)
		return(0);
	_InsertAfter((NodeBase*)new_node);
	return(1);
}

//-----------//
// AddSorted //
//-----------//
// create a node with data in it and insert it sorted
// the new node becomes the current node
// adds *after* equal nodes so that equal nodes are sorted by insertion
// time

template <class T>
int
List<T>::AddSorted(
	T*	new_data)
{
	// make sure there is a current node
	if (! _current)
		_current = _head;

	T* currentdata = GetCurrent();

	// search backward
	while (currentdata != 0 && *new_data < *currentdata)
		currentdata = GetPrev();

	if (currentdata == 0)	// went all the way back past the head
	{						// or the list was empty
		// prepend
		return(Prepend(new_data));
	}

	// search forward
	while (currentdata != 0 && *new_data >= *currentdata)
		currentdata = GetNext();

	if (currentdata == 0)	// went all the way past the tail
	{
		// append
		return(Append(new_data));
	}

	// insert before
	return(InsertBefore(new_data));
}

//-----------------//
// AddUniqueSorted //
//-----------------//
// create a node with data in it and insert it sorted
// the new node becomes the current node
// if a node containing == date exists in the list, the new T is deleted
//   and *not* added
// returns 1 on success, 0 on failure
// adds *after* equal nodes so that equal nodes are sorted by insertion time

template <class T>
int
List<T>::AddUniqueSorted(
	T*	new_data)
{
	// try to find the node (blind search -- boy, am I lazy!)
	if (Find(new_data))
	{
		delete new_data;		// found, no need to add it
		return(1);
	}
	else
	{
		return(AddSorted(new_data));	// not found, add it sorted
	}
}

//------//
// Sort //
//------//

template <class T>
int
List<T>::Sort()
{
	if (! _head)
		return;

	for (NodeBase* node = _head->next; node; node = node->next)
	{
		T* node_data = ((Node<T>*)node)->data;
		T* prev_data = ((Node<T>*)(node->prev))->data;
		if (*node_data < *prev_data)
		{
			// node needs to be sorted
			_current = node;	// go to the unsorted node
			node = node->prev;	// back up before current
			T* data = RemoveCurrent();
			_current = _head;	// go to beginning of list
			if (! AddSorted(data))
				return(0);
		}
	}
	return(1);
}
*/
