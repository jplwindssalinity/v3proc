//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.0   04 Feb 1998 14:16:22   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:28:30  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef LISTBASE_H
#define LISTBASE_H

static const char ListBase_H_rcsid[] =
    "@(#) $Header$";


//==========
// NodeBase 
//==========

struct NodeBase
{
    NodeBase*   prev;
    NodeBase*   next;

    NodeBase();
    virtual ~NodeBase();
};


//==========
// ListBase 
//==========

class ListBase
{
public:
    ListBase();
    virtual ~ListBase();

    virtual void      AppendList(ListBase* added_list);
    virtual int       IsEmpty();      // 1 if the list is empty, 0 otherwise
    virtual int       NodeCount();    // returns the number of nodes in the list
    virtual int       GotoNodeIndex(int node_index);

protected:
    virtual void      _Prepend(NodeBase* node);
    virtual void      _Append(NodeBase* node);
    virtual void      _InsertBefore(NodeBase* node);
    virtual void      _InsertAfter(NodeBase* node);
    virtual NodeBase* _RemoveCurrent();

    NodeBase*   _head;
    NodeBase*   _tail;
    NodeBase*   _current;
};

#endif // LISTBASE_H
