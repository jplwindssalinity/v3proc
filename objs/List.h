//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef LIST_H
#define LIST_H

static const char rcs_id_list_h[] =
	"@(#) $Id$";

//======================================================================
// CLASSES
//		Node, List
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
	T*		GetNodeWithIndex(int index);	// current = node with given index

	//--------//
	// moving //
	//--------//

	void	GotoHead() { _current = _head; };
	void	GotoTail() { _current = _tail; };
	int		GotoNext();
	int		GotoPrev();
	int		SwapCurrentAndNext();

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

/*

	//----------------//
	// accessing list //
	//----------------//

	void		AppendList(ListBase* added_list);
	int			IsEmpty();		// 1 if the list is empty, 0 otherwise

protected:
	void		_Prepend(NodeBase* node);
	void		_Append(NodeBase* node);
	void		_InsertBefore(NodeBase* node);
	void		_InsertAfter(NodeBase* node);
	NodeBase*	_RemoveCurrent();

	NodeBase*	_head;
	NodeBase*	_tail;
	NodeBase*	_current;
};

//======//
// List //
//======//

template<class T>
class List : public ListBase
{
public:
	List();
	~List();

	// query
	//-----------
	int		IsHead(T* data);	// return 1 if true, else 0
	int		IsTail(T* data);	// return 1 if true, else 0

	// navigation
	//-----------
	T*		GetTail();			// current = tail, return T* of current
	T*		GetPrev();			// current = prev, return T* of current
	T*		GetIndex(int node_index);// current = index, return T* of current

	// adding data
	//------------
	int		Prepend(T* newdata);			// prepend to front of list
	int		InsertBefore(T* newdata);		// insert before the current node
	int		InsertAfter(T* newdata);		// insert after the current node
	int		InsertBeforeIndex(T* newdata, int node_index);
	int		InsertAfterIndex(T* newdata, int node_index);
	int		AddSorted(T* newdata);			// add to the list sorted
	int		AddUniqueSorted(T* newdata);	// add if unique to the list sorted

	// operations
	//-----------
	T*		Find(T* data);		// return T* or NULL (current is set)
	int		Sort();				// sort the list
};
*/

#endif
