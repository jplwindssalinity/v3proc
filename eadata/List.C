//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.3   14 Apr 1998 16:29:02   sally
// move back to old List
// 
//    Rev 1.1   26 Feb 1998 09:59:18   sally
// add missing return statement
// 
//    Rev 1.0   04 Feb 1998 14:16:16   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:29:16  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef LIST_C
#define LIST_C

#ifndef __SCAT_GNUCPP__
static const char List_C_rcsid[] =
    "@(#) $Header$";
#endif

#include <stdio.h>

#include "List.h"

//==============
// Node methods 
//==============

template<class T>
Node<T>::Node(
    T*  newdata)
:   NodeBase(), data(newdata)
{
    return;
}

template<class T>
Node<T>::~Node()
{
    return;
}

//==============
// List methods 
//==============

template<class T>
List<T>::List()
:   ListBase()
{
    return;
}

template<class T>
List<T>::~List()
{
    GetHead();
    T* node;
    while ((node=RemoveCurrent()) != NULL)
        delete node;
    return;
}

//---------
// GetHead 
//---------
// the head node becomes the current node
// return the data from the current node.

template<class T>
T*
List<T>::GetHead()
{
    _current = _head;
    return (GetCurrent());
}

//---------
// GetTail 
//---------
// the tail node becomes the current node
// return the data from the current node.

template<class T>
T*
List<T>::GetTail()
{
    _current = _tail;
    return (GetCurrent());
}

//---------
// GetNext 
//---------
// the next node becomes the current node
// return the data from the current node.

template<class T>
T*
List<T>::GetNext()
{
    if (_current)
        _current = _current->next;
    return (GetCurrent());
}

//---------
// GetPrev 
//---------
// the prev node becomes the current node
// return the data from the current node.

template<class T>
T*
List<T>::GetPrev()
{
    if (_current)
        _current = _current->prev;
    return (GetCurrent());
}

//------------
// GetCurrent 
//------------
// return the data from the current node

template<class T>
T*
List<T>::GetCurrent()
{
    if (_current)
        return (((Node<T>*)_current)->data);
    else
        return (0);
}

//----------
// GetIndex 
//----------

template<class T>
T*
List<T>::GetIndex(
int     node_index)
{
    if (! GotoNodeIndex(node_index))
        return(0);

    return(GetCurrent());

}//List<T>::GetIndex

//---------------
// RemoveCurrent 
//---------------
// remove the current node
// the next node becomes the current node
// return the data from the removed node

template<class T>
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

//---------
// Prepend 
//---------
// create a node with data in it and prepend it to the front of the list
// if the list is not empty, the current node remains unchanged
// if the list is empty, the new node becomes the current node

template<class T>
void
List<T>::Prepend(
    T*  newdata)
{
    Node<T>* newnode = new Node<T>(newdata);
    _Prepend((NodeBase*)newnode);
    return;
}

//--------
// Append 
//--------
// create a node with data in it and append it to the end of the list
// if the list is not empty, the current node remains unchanged
// if the list is empty, the new node becomes the current node

template<class T>
void
List<T>::Append(
    T*  newdata)
{
    Node<T>* newnode = new Node<T>(newdata);
    _Append((NodeBase*)newnode);
    return;
}

//--------------
// InsertBefore 
//--------------
// create a node with data in it and insert it before the current node
// the new node becomes the current node

template<class T>
void
List<T>::InsertBefore(
    T*  newdata)
{
    Node<T>* newnode = new Node<T>(newdata);
    _InsertBefore((NodeBase*)newnode);
    return;
}

//--------------
// InsertAfter 
//--------------
// create a node with data in it and insert it after the current node
// the new node becomes the current node

template<class T>
void
List<T>::InsertAfter(
    T*  newdata)
{
    Node<T>* newnode = new Node<T>(newdata);
    _InsertAfter((NodeBase*)newnode);
    return;
}

//-------------------
// InsertBeforeIndex 
//-------------------
// create a node with data in it and insert it before the node with the
// given index (indicies start at 0)

template<class T>
int
List<T>::InsertBeforeIndex(
    T*      newdata,
    int     node_index)
{
    if (! GotoNodeIndex(node_index))
        return(0);
    Node<T>* newnode = new Node<T>(newdata);
    _InsertBefore((NodeBase*)newnode);
    return(1);
}

//------------------
// InsertAfterIndex 
//------------------
// create a node with data in it and insert it after the node with the
// given index (indicies start at 0)

template<class T>
int
List<T>::InsertAfterIndex(
    T*      newdata,
    int     node_index)
{
    if (! GotoNodeIndex(node_index))
        return(0);
    Node<T>* newnode = new Node<T>(newdata);
    _InsertAfter((NodeBase*)newnode);
    return(1);
}

//-----------
// AddSorted 
//-----------
// create a node with data in it and insert it sorted
// the new node becomes the current node
// adds *after* equal nodes so that equal nodes are sorted by insertion
// time

template<class T>
void
SortedList<T>::AddSorted(
    T*  newdata)
{
    // make sure there is a current node
    if (! _current)
        _current = _head;

    T* currentdata = GetCurrent();

    // search backward
    while (currentdata != 0 && *newdata < *currentdata)
        currentdata = GetPrev();

    if (currentdata == 0)   // went all the way back past the head
    {                       // or the list was empty
        // prepend
        Prepend(newdata);
        return;
    }

    // search forward
    while (currentdata != 0 && *newdata >= *currentdata)
        currentdata = GetNext();

    if (currentdata == 0)   // went all the way past the tail
    {
        // append
        Append(newdata);
        return;
    }

    // insert before
    InsertBefore(newdata);
}

//-----------------
// AddUniqueSorted 
//-----------------
// create a node with data in it and insert it sorted
// the new node becomes the current node
// if a node containing == date exists in the list, the new T is deleted
//   and *not* added
// returns 1 if the node was added, returns 0 if the data was deleted
// adds *after* equal nodes so that equal nodes are sorted by insertion time

template<class T>
int
SortedList<T>::AddUniqueSorted(
    T*  newdata)
{
    // try to find the node (blind search -- boy, am I lazy)
    if (Find(newdata))
    {
        delete newdata;     // found, no need to add it
        return(0);
    }
    else
    {
        AddSorted(newdata); // not found, add it sorted
        return(1);
    }
}

//------
// Sort 
//------

template<class T>
void
SortedList<T>::Sort()
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
            _current = node;    // go to the unsorted node
            node = node->prev;  // back up before current
            T* data = RemoveCurrent();
            _current = _head;   // go to beginning of list
            AddSorted(data);
        }
    }
    return;
}

//------
// Find 
//------

template<class T>
int
List<T>::Find(
    T*  data)
{
    for (T* contents = GetHead(); contents; contents = GetNext())
    {
        if (*contents == *data)
            return(1);
    }
    return(0);
}

//--------
// IsHead 
//--------

template<class T>
int
List<T>::IsHead(
T*  data)
{
    if (_head)
    {
        return (data == ((Node<T>*)_head)->data ? 1 : 0);
    }
    else
        return (0);

}//List<T>::IsHead

//--------
// IsTail 
//--------

template<class T>
int
List<T>::IsTail(
T*  data)
{
    if (_tail)
    {
        return (data == ((Node<T>*)_tail)->data ? 1 : 0);
    }
    else
        return (0);

}//List<T>::IsHead

template<class T>
SortedList<T>::SortedList()
: List<T>()
{
    // empty
}

template<class T>
SortedList<T>::~SortedList()
{
    // empty
}

#endif
