//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef IMAGE_H
#define IMAGE_H

static const char rcs_id_image_h[] =
    "@(#) $Id$";

#include <stdio.h>

//======================================================================
// CLASSES
//    Image
//======================================================================

//======================================================================
// CLASS
//    Image
//
// DESCRIPTION
//    The Image object is used to manipulate images.
//======================================================================

class Image
{
public:

    typedef unsigned char uchar;

    //--------------//
	// construction //
	//--------------//

	Image();
	~Image();

    int   Allocate(const int x_size, const int y_size);
    int   Allocate(Image* image);
    void  Free();
    void  FreeImage();
    void  FreeMask();

    //----------//
    // checking //
    //----------//

    int  SizeMatch(Image* image);
    int  MinMax(float* min_value, float* max_value);

    //-----------------//
    // setting/getting //
    //-----------------//

    int  Set(int x, int y, float image_value);
    int  Set(int x, int y, float image_value, uchar mask_value);

    int  Get(int x, int y, float* image_value);

    //--------------//
    // input/output //
    //--------------//

    int  WritePtim(const char* filename, Image* x_image, Image* y_image,
             float x_min, float x_max, float x_res, float y_min, float y_max,
             float y_res);

    //--------------//
    // initializing //
    //--------------//

    int  Step(float angle);

    //------------//
    // operations //
    //------------//

    int  Convolve(Image* image_1, Image* image_2);
    int  Gradient(int operator_size);
    int  Magnitude(Image* image_1, Image* image_2);

protected:

    //-----------//
    // variables //
    //-----------//

    int      _xSize;
    int      _ySize;
    float**  _image;
    uchar**  _mask;
};

#endif
