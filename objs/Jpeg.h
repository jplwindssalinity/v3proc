//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef JPEG_H
#define JPEG_H

static const char rcs_id_jpeg_h[] =
    "@(#) $Id$";

#include <stdio.h>
#include "ColorMap.h"

//================//
// JPEG functions //
//================//

int  write_jpeg(float** array, int x_size, int y_size, ColorMap* colormap,
         int quality, const char* output_file);

#endif
