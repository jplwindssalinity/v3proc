//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.1   21 Apr 1998 16:39:36   sally
// for L2B
// 
//    Rev 1.0   20 Apr 1998 15:18:28   sally
// Initial revision.
// 
//    Rev 1.2   14 Apr 1998 16:28:46   sally
// move back to old List
// 
//    Rev 1.0   04 Feb 1998 14:16:18   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:28:29  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

// This file contains a double linked list class.  The contents of the
// list are templated (not void*).

#ifndef EALIST_H
#define EALIST_H

static const char rcs_id_ealist_h[] =
    "@(#) $Header$";

#include "ListBase.h"

//======
// EANode 
//======

template<class T>
struct EANode : public NodeBase
{
    T*      data;

    EANode(T* newdata);
    virtual ~EANode();
};

//======
// List 
//======

template<class T>
class EAList : public ListBase
{
public:
    EAList();
    virtual ~EAList();

    // query
    //-----------
    virtual int     IsHead(T* data);    // return 1 if true, else 0
    virtual int     IsTail(T* data);    // return 1 if true, else 0

    // navigation
    //-----------
    virtual T*      GetHead();          // current = head, return T* of current
    virtual T*      GetTail();          // current = tail, return T* of current
    virtual T*      GetPrev();          // current = prev, return T* of current
    virtual T*      GetNext();          // current = next, return T* of current
    virtual T*      GetCurrent();       // return T* of current
    virtual T*      GetIndex(int node_index);// current = index, return T* of current
    virtual T*      RemoveCurrent();    // remove current, next becomes current

    // adding data
    //------------
    virtual void    Prepend(T* newdata);       // prepend to front of list
    virtual void    Append(T* newdata);        // append to end of list
    virtual void    InsertBefore(T* newdata);  // insert before the current node
    virtual void    InsertAfter(T* newdata);   // insert after the current node
    virtual int     InsertBeforeIndex(T* newdata, int node_index);
    virtual int     InsertAfterIndex(T* newdata, int node_index);

    // operations
    //-----------
    virtual int     Find(T* data);// return 1 if found, else 0 (current is set)
};

template<class T>
class SortedList : public EAList<T>
{
public:
    SortedList();
    virtual ~SortedList();

    // adding data
    //-----------
    virtual void  AddSorted(T* newdata);       // add to the list sorted
    virtual int   AddUniqueSorted(T* newdata); // add if unique to the list sorted

    // operations
    //-----------
    virtual void  Sort();             // sort the list
};


#endif // EALIST_H
