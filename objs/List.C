//==============================================================//
// Copyright (C) 1997-2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef LIST_C
#define LIST_C

static const char rcs_id_list_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include "List.h"

//======//
// Node //
//======//

template <class T>
Node<T>::Node(
    T*    new_data)
:   prev(NULL), next(NULL), data(new_data)
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
:   _head(NULL), _tail(NULL), _current(NULL)
{
    return;
}

template <class T>
List<T>::~List()
{
    if (_head != NULL)
    {
        fprintf(stderr, "List destroyed without being deallocated!\n");
    }
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
    T*  new_data)
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

//---------------//
// List::Prepend //
//---------------//
// Creates a node with data in it and prepends it to the beginning of the
// list.  The new node becomes the current node.
// Returns 1 on success, 0 on failure

template <class T>
int
List<T>::Prepend(
    T*    new_data)
{
    Node<T>* new_node = new Node<T>(new_data);
    if (new_node == NULL)
        return(0);

    if (_head)
    {
        // beginning of list exists, just prepend
        new_node->next = _head;
        new_node->prev = NULL;
        _head->prev = new_node;
        _head = new_node;
    }
    else
    {
        // list is empty
        new_node->prev = NULL;
        new_node->next = NULL;
        _head = _tail = new_node;
    }

    _current = new_node;

    return(1);
}

//------------------//
// List::AppendList //
//------------------//
// Appends the passed list to the end of the list.  The entire passed
// list is disconnected and added to the new list.  The passed list
// is emptied.  The current node is unchanged.

template <class T>
void
List<T>::AppendList(
    List<T>*    list)
{
    //-------------------//
    // patch list at end //
    //-------------------//

    if (_tail)
    {
        // list is not empty
        _tail->next = list->_head;
        list->_head->prev = _tail;
        _tail = list->_tail;
    }
    else
    {
        // list is empty
        _head = list->_head;
        _tail = list->_tail;
    }

    //----------------------//
    // disconnect from list //
    //----------------------//

    // unattach from added_list
    list->_head = NULL;
    list->_tail = NULL;

    return;
}

//--------------------//
// List::InsertBefore //
//--------------------//
// Creates a node with data in it and inserts it before the current node.
// The new node becomes the current node.
// Returns 1 on success, 0 on failure

template <class T>
int
List<T>::InsertBefore(
    T*    new_data)
{
    // create a new node
    Node<T>* new_node = new Node<T>(new_data);
    if (new_node == NULL)
        return(0);

    // define the current node
    if (_current == NULL)
        _current = _head;

    if (_head == NULL)
    {
        // empty list
        _head = new_node;
        _tail = new_node;
        new_node->prev = NULL;
        new_node->next = NULL;
    }
    else if (_current == _head)
    {
        // node goes before head
        new_node->prev = _current->prev;
        new_node->next = _current;
        _current->prev = new_node;
        _head = new_node;
    }
    else
    {
        // node inserts
        new_node->prev = _current->prev;
        new_node->next = _current;
        _current->prev->next = new_node;
        _current->prev = new_node;
    }
    _current = new_node;
    return(1);
}

//-------------------//
// List::InsertAfter //
//-------------------//
// Creates a node with data in it and inserts it after the current node.
// The new node becomes the current node.
// Returns 1 on success, 0 on failure

template <class T>
int
List<T>::InsertAfter(
    T*    new_data)
{
    // create a new node
    Node<T>* new_node = new Node<T>(new_data);
    if (new_node == NULL)
        return(0);

    // define the current node
    if (_current == NULL)
        _current = _tail;

    if (_head == NULL)
    {
        // empty list
        _head = new_node;
        _tail = new_node;
        new_node->prev = NULL;
        new_node->next = NULL;
    }
    else if (_current == _tail)
    {
        // node goes after tail
        new_node->prev = _tail;
        new_node->next = NULL;
        _tail->next = new_node;
        _tail = new_node;
    }
    else
    {
        // node inserts
        new_node->prev = _current;
        new_node->next = _current->next;
        _current->next->prev = new_node;
        _current->next = new_node;
    }
    _current = new_node;
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

//------------------//
// List::GetByIndex //
//------------------//
// Returns the data from the node with the given index.  Returns
// NULL on failure.

template <class T>
T*
List<T>::GetByIndex(
    int        index)
{
    GotoHead();
    for (int i = 0; i < index; i++)
        GotoNext();

    return(GetCurrent());
}

//------------------//
// List::GetIndexOf //
//------------------//
// Returns the index of the node containing the given data. Returns
// -1 on failure. The current node is unchanged. This is done using
// pointer equality, NOT content equals.

template <class T>
int
List<T>::GetIndexOf(
    T*  data)
{
    int index = 0;
    for (Node<T>* node = _head; node; node = node->next)
    {
        if (node->data == data)
            return(index);
        index++;
    }
    return(-1);
}

//----------------//
// List::GotoNext //
//----------------//
// Sets the current node to be the next node.
// Returns 1 on success.  If the current node or the node following
// the current node does not exist (is NULL), GotoNext fails and
// returns 0.

template <class T>
int
List<T>::GotoNext()
{
    if (! _current)
        return(0);
    _current = _current->next;
    if (! _current)
        return(0);
    return(1);
}

//----------------//
// List::GotoPrev //
//----------------//
// Sets the current node to be the prev node.
// Returns 1 on success.  If the current node or the node preceeding
// the current node does not exist (is NULL), GotoPrev fails and
// returns 0.

template <class T>
int
List<T>::GotoPrev()
{
    if (! _current)
        return(0);
    _current = _current->prev;
    if (! _current)
        return(0);
    return(1);
}

//--------------------------//
// List::SwapCurrentAndNext //
//--------------------------//
// Swaps the data in the current and next node (if they both exist)
// The current node is unchanged.
// Returns 1 on success, 0 if either node doesn't exist

template <class T>
int
List<T>::SwapCurrentAndNext()
{
    // make sure current and next exist
    if (! _current || ! _current->next)
        return(0);

    // swap
    T* tmp_data = _current->data;
    _current->data = _current->next->data;
    _current->next->data = tmp_data;

    return(1);
}

//-----------------//
// List::NodeCount //
//-----------------//
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

//--------------//
// List::IsHead //
//--------------//

template <class T>
int
List<T>::IsHead(
    T*    data)
{
    if (_head && data == _head->data)
        return(1);
    else
        return(0);
}

//--------------//
// List::IsTail //
//--------------//

template <class T>
int
List<T>::IsTail(
    T*    data)
{
    if (_tail && data == _tail->data)
        return(1);
    else
        return(0);
}

//---------------//
// List::IsEmpty //
//---------------//

template <class T>
int
List<T>::IsEmpty()
{
    if (_head)
        return(0);
    else
        return(1);
}

//==============//
// SortableList //
//==============//

//-------------------------//
// SortableList::AddSorted //
//-------------------------//
// Creates a node with data in it and inserts it into the list so that
// the list is sorted.  Inserts the node after nodes containing equal
// data so that equal nodes are sorted by insertion order.
// Returns 1 on success, 0 on failure

template <class T>
int
SortableList<T>::AddSorted(
    T*    new_data)
{
    // make sure there is a current node
    if (! _current)
        _current = _head;

    T* current_data = GetCurrent();

    // search backwards
    while (current_data && *new_data < *current_data)
        current_data = GetPrev();

    if (! current_data)
        return(Prepend(new_data));

    // search forward
    while (current_data && *new_data >= *current_data)
        current_data = GetNext();

    if (! current_data)
        return(Append(new_data));

    // insert before
    return(InsertBefore(new_data));
}

//-------------------------------//
// SortableList::AddUniqueSorted //
//-------------------------------//
// Creates a node with data in it and inserts it into the list so that
// the list is sorted.  Inserts the node after nodes containing equal
// data so that equal nodes are sorted by insertion order.  If an == node
// exists, the new data is deleted.
// Returns 1 on success, 0 on failure

template <class T>
int
SortableList<T>::AddUniqueSorted(
    T*    new_data)
{
    // check for an == node
    if (Find(new_data))
    {
        delete new_data;
        return(1);
    }
    else
    {
        return(AddSorted(new_data));
    }
}

//--------------------//
// SortableList::Find //
//--------------------//
// Searches for the node containing data equal to the target data.
// If such a node is found, returns 1 and sets the current node.
// Otherwise, returns 0.

template<class T>
int
SortableList<T>::Find(
    T*    data)
{
    for (T* contents = GetHead(); contents; contents = GetNext())
    {
        if (*contents == *data)
            return(1);
    }
    return(0);
}

//--------------------//
// SortableList::Sort //
//--------------------//

template <class T>
int
SortableList<T>::Sort()
{
    if (! _head)
        return(0);

    for (Node<T>* node = _head->next; node; node = node->next)
    {
        T* node_data = ((Node<T>*)node)->data;
        T* prev_data = ((Node<T>*)(node->prev))->data;
        if (*node_data < *prev_data)
        {
            // node needs to be sorted
            _current = node;    // go to the unsorted node
            node = node->prev;    // back up before current
            T* data = RemoveCurrent();
            _current = _head;    // go to beginning of list
            if (! AddSorted(data))
                return(0);
        }
    }
    return(1);
}

#endif
