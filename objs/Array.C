//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_array_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <stdarg.h>
#include "Array.h"

//=======//
// Array //
//=======//

template <class T>
Array<T>::Array(int i1)
{
	dims = 1;
	ptr1 = (T*)malloc((size_t)(i1*sizeof(T)));
	ptr2 = NULL;
	return;
}

template <class T>
Array<T>::Array(int i1, int i2)
{
	dims = 2;
	ptr1 = NULL;
	ptr2 = (T**)malloc((size_t)(i1*sizeof(T*)));
	if (!ptr2)
	{
		ptr2 = NULL;
		return;
	}

	ptr2[0] = (T*)malloc((size_t)(i1*i2*sizeof(T)));
	if (!(ptr2[0]))
	{
		free(ptr2);	// deallocate the row pointers
		ptr2 = NULL;
		return;
	}

	for (int i=1; i < i1; i++) ptr2[i] = ptr2[i-1] + i2;

	return;
}

template <class T>
Array<T>::~Array()
{
	if (dims == 1)
	{
		free(ptr1);
	}
	else if (dims == 2)
	{
		free(ptr2[0]);
		free(ptr2);
	}
	else
	{
		printf("Error: illegal dims value = %d in Array object\n",dims);
	}
	return;
}
