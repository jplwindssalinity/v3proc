//=========================================================//
// Copyright  (C)1996, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

// This file does the instantiations for all template files
// for GNU C++ compiler
// use "-fno-implicit-templates"

static const char TempInstances_C_rcsid[] =
    "@(#) $Id$";

#ifdef __SCAT_GNUCPP__

#include <mfhdf.h>

#include "EAList.h"
#include "EAList.C"

#include "Command.h"
#include "EAConfigList.h"
#include "Eqx.h"
#include "Filter.h"
#include "LimitChecker.h"
#include "Parameter.h"
#include "Polynomial.h"
#include "TlmHdfFile.h"
#include "ToDo.h"

template class EANode<int32>;
template class EAList<int32>;
template class EANode<DueFiles>;
template class EAList<DueFiles>;
template class EANode<Filter>;
template class EAList<Filter>;
template class EANode<FilterList>;
template class EAList<FilterList>;
template class EANode<LimitChecker>;
template class EAList<LimitChecker>;
template class EANode<Parameter>;
template class EAList<Parameter>;
template class EANode<Polynomial>;
template class EAList<Polynomial>;
template class EANode<EAStringPair>;
template class EAList<EAStringPair>;
template class EANode<ToDo>;
template class EAList<ToDo>;

template class EANode<Command>;
template class EAList<Command>;
template class SortedList<Command>;
template class EANode<Eqx>;
template class EAList<Eqx>;
template class SortedList<Eqx>;
template class EANode<TlmHdfFile>;
template class EAList<TlmHdfFile>;
template class SortedList<TlmHdfFile>;

#endif //__SCAT_GNUCPP__

