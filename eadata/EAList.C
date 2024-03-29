//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// Revision 1.9  1999/09/30 23:01:39  sally
// update 9/30/99
//
// 
//    Rev 1.2   07 Sep 1999 13:32:46   sally
// add interface for Global Attributes
// 
//    Rev 1.1   21 Apr 1998 16:39:34   sally
// for L2B
// 
//    Rev 1.0   20 Apr 1998 15:18:32   sally
// Initial revision.
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

#include "EAList.h"

//==============
// EANode methods 
//==============

template<class T>
EANode<T>::EANode(
    T*  newdata)
:   NodeBase(), data(newdata)
{
    return;
}

template<class T>
EANode<T>::~EANode()
{
    return;
}

//==============
// List methods 
//==============

template<class T>
EAList<T>::EAList()
:   ListBase()
{
    return;
}

template<class T>
EAList<T>::~EAList()
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
EAList<T>::GetHead()
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
EAList<T>::GetTail()
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
EAList<T>::GetNext()
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
EAList<T>::GetPrev()
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
EAList<T>::GetCurrent()
{
    if (_current)
        return (((EANode<T>*)_current)->data);
    else
        return (0);
}

//----------
// GetIndex 
//----------

template<class T>
T*
EAList<T>::GetIndex(
int     node_index)
{
    if (! GotoNodeIndex(node_index))
        return(0);

    return(GetCurrent());

}//EAList<T>::GetIndex

//---------------
// RemoveCurrent 
//---------------
// remove the current node
// the next node becomes the current node
// return the data from the removed node

template<class T>
T*
EAList<T>::RemoveCurrent()
{
    // remove the current node and get it
    EANode<T>* node = (EANode<T>*)ListBase::_RemoveCurrent();
    if (node)
    {
        T* data = node->data;
        delete node;
        return (data);
    }
    else
    {
        return (0);
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
EAList<T>::Prepend(
    T*  newdata)
{
    EANode<T>* newnode = new EANode<T>(newdata);
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
EAList<T>::Append(
    T*  newdata)
{
    EANode<T>* newnode = new EANode<T>(newdata);
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
EAList<T>::InsertBefore(
    T*  newdata)
{
    EANode<T>* newnode = new EANode<T>(newdata);
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
EAList<T>::InsertAfter(
    T*  newdata)
{
    EANode<T>* newnode = new EANode<T>(newdata);
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
EAList<T>::InsertBeforeIndex(
    T*      newdata,
    int     node_index)
{
    if (! GotoNodeIndex(node_index))
        return(0);
    EANode<T>* newnode = new EANode<T>(newdata);
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
EAList<T>::InsertAfterIndex(
    T*      newdata,
    int     node_index)
{
    if (! GotoNodeIndex(node_index))
        return(0);
    EANode<T>* newnode = new EANode<T>(newdata);
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
    if (! EAList<T>::_current)
        EAList<T>::_current = EAList<T>::_head;

    T* currentdata = EAList<T>::GetCurrent();

    // search backward
    while (currentdata != 0 && *newdata < *currentdata)
        currentdata = EAList<T>::GetPrev();

    if (currentdata == 0)   // went all the way back past the head
    {                       // or the list was empty
        // prepend
        Prepend(newdata);
        return;
    }

    // search forward
    while (currentdata != 0 && *newdata >= *currentdata)
        currentdata = EAList<T>::GetNext();

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
    if (! EAList<T>::_head)
        return;

    for (NodeBase* node = EAList<T>::_head->next; node; node = node->next)
    {
        T* node_data = ((EANode<T>*)node)->data;
        T* prev_data = ((EANode<T>*)(node->prev))->data;
        if (*node_data < *prev_data)
        {
            // node needs to be sorted
            EAList<T>::_current = node;    // go to the unsorted node
            node = node->prev;  // back up before current
            T* data = EAList<T>::RemoveCurrent();
            EAList<T>::_current = EAList<T>::_head;   // go to beginning of list
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
EAList<T>::Find(
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
EAList<T>::IsHead(
T*  data)
{
    if (_head)
    {
        return (data == ((EANode<T>*)_head)->data ? 1 : 0);
    }
    else
        return (0);

}//EAList<T>::IsHead

//--------
// IsTail 
//--------

template<class T>
int
EAList<T>::IsTail(
T*  data)
{
    if (_tail)
    {
        return (data == ((EANode<T>*)_tail)->data ? 1 : 0);
    }
    else
        return (0);

}//EAList<T>::IsHead

template<class T>
SortedList<T>::SortedList()
: EAList<T>()
{
    // empty
}

template<class T>
SortedList<T>::~SortedList()
{
    // empty
}

#endif
