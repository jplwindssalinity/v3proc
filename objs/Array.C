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
	ptr = (void*)malloc((size_t)(i1*sizeof(T)));
	return;
}

template <class T>
Array<T>::Array(int i1, int i2)
{
	dims = 2;
	ptr = (void*)malloc((size_t)(i1*sizeof(T*)));
	if (!ptr)
	{
		ptr = NULL;
		return;
	}

	ptr[0] = (void*)malloc((size_t)(i1*i2*sizeof(T)));
	if (!(ptr[0]))
	{
		free(ptr);	// deallocate the row pointers
		ptr = NULL;
		return;
	}

	for (int i=1; i < i1; i++) ptr[i] = ptr[i-1] + i2;

	return;
}

template <class T>
Array<T>::~Array()
{
	if (dims == 1)
	{
		free(ptr);
	}
	else if (dims == 2)
	{
		free(ptr[0]);
		free(ptr);
	}
	else
	{
		printf("Error: illegal dims value = %d in Array object\n",dims);
	}
	return;
}
