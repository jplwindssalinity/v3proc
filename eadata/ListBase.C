//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.0   04 Feb 1998 14:16:20   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:29:17  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

static const char ListBase_C_rcsid[] =
    "@(#) $Header$";

#include <stdio.h>

#include "ListBase.h"

//==================
// NodeBase methods 
//==================

NodeBase::NodeBase()
:   prev(NULL), next(NULL)
{
    return;
}

NodeBase::~NodeBase()
{
    return;
}


//==================
// ListBase methods 
//==================

ListBase::ListBase()
:   _head(NULL), _tail(NULL), _current(NULL)
{
    return;
}

ListBase::~ListBase()
{
    return;
}

//------------
// AppendList 
//------------
// adds a list to the end of another

void
ListBase::AppendList(
    ListBase*   added_list)
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

//---------
// IsEmpty 
//---------
// return 1 if the list is empty, 0 otherwise

int
ListBase::IsEmpty()
{
    if (_head)
        return(0);
    else
        return(1);
}

//-----------
// NodeCount 
//-----------
// returns the number of nodes in the list

int
ListBase::NodeCount()
{
    int count = 0;
    for (NodeBase* node = _head; node; node = node->next)
    {
        count++;
    }
    return(count);
}

//---------------
// GotoNodeIndex 
//---------------
// set the current node to be the node with the specified index
// return 1 on success, 0 on failure

int
ListBase::GotoNodeIndex(
    int     node_index)
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

//----------
// _Prepend 
//----------
// add node to the front of the list
// if the list is not empty, the current node remains unchanged
// if the list is empty, the new node becomes the current node

void
ListBase::_Prepend(
    NodeBase*   node)
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

//---------
// _Append 
//---------
// add node to the end of the list
// if the list is not empty, the current node remains unchanged
// if the list is empty, the new node becomes the current node

void
ListBase::_Append(
    NodeBase*   node)
{
    if (_tail)
    {
        // end of list exists, just append
        node->next = NULL;
        node->prev = _tail;
        _tail->next = node;
        _tail = _current = node;
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

//---------------
// _InsertBefore 
//---------------
// insert node before the current node
// the new node becomes the current node
// if the current node is NULL, insert at the head and
// set current to be the head node

void
ListBase::_InsertBefore(
    NodeBase*   node)
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

//--------------
// _InsertAfter 
//--------------
// insert node after the current node
// the new node becomes the current node
// if the current node is NULL, insert at the tail and
// set current to be the tail node

void
ListBase::_InsertAfter(
    NodeBase*   node)
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

//----------------
// _RemoveCurrent 
//----------------
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

