//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef LIST_H
#define LIST_H

static const char rcs_id_list_h[] =
	"@(#) $Id$";

//======================================================================
// CLASSES
//		Node, List, SortableList
//======================================================================

//======================================================================
// CLASS
//		Node
//
// DESCRIPTION
//		The Node object is a node for a doubly linked list.  It
//		contains a pointer to a data object and pointers to the
//		previous and next nodes in the list.
//======================================================================

template <class T>
class Node
{
public:

	//--------------//
	// construction //
	//--------------//

	Node(T* new_data);
	~Node();

	//-----------//
	// variables //
	//-----------//

	Node*	prev;
	Node*	next;
	T*		data;
};

//======================================================================
// CLASS
//		List
//
// DESCRIPTION
//		The List object is a doubly linked list of Nodes.  The list
//		can be traversed forwards and backwards.
//======================================================================

template <class T>
class List
{
public:

	//--------------//
	// construction //
	//--------------//

	List();
	~List();

	//----------------//
	// adding to list //
	//----------------//

	int		Append(T* new_data);			// append to end of list
	int		Prepend(T* new_data);			// prepend to beginning of list
	void	AppendList(List<T>* list);
	int		InsertBefore(T* new_data);

	//--------------------//
	// removing from list //
	//--------------------//

	T*		RemoveCurrent();	// remove current, next becomes current

	//----------------------//
	// retrieving from list //
	//----------------------//

	T*		GetHead();			// current = head, return T* of current
	T*		GetTail();			// current = tail, return T* of current
	T*		GetCurrent();		// return T* of current
	T*		GetNext();			// current = next, return T* of current
	T*		GetPrev();			// current = prev, return T* of current
	T*		GetByIndex(int index);	// current = node with given index

	//--------//
	// moving //
	//--------//

	void	GotoHead() { _current = _head; return; };
	void	GotoTail() { _current = _tail; return; };
	int		GotoNext();
	int		GotoPrev();
	int		SwapCurrentAndNext();
	int		Find(T* data);

	//-------------------//
	// hacking into list //
	//-------------------//

	Node<T>*	GetCurrentNode() { return(_current); };
	void		SetCurrentNode(Node<T>* node) { _current = node; return; };

	//-------------//
	// information //
	//-------------//

	int		NodeCount();		// returns the number of nodes

protected:

	//-----------//
	// variables //
	//-----------//

	Node<T>*		_head;
	Node<T>*		_tail;
	Node<T>*		_current;
};

//======================================================================
// CLASS
//		SortableList
//
// DESCRIPTION
//		The SortableList object is a List which has sorting methods.
//		It requires the node object to have the following operators:
//		<, >=
//======================================================================

template <class T>
class SortableList : public List
{
public:

	//----------------//
	// adding to list //
	//----------------//

	int		AddSorted(T* new_data);
	int		AddUniqueSorted(T* new_data);

	//-----------------//
	// organizing list //
	//-----------------//

	int		Sort();
};

#endif
