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
    int   AllocateMask(uchar fill_value);

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

    int  WritePltr(const char* filename, Image* x_image, Image* y_image);
    int  WriteAscii(FILE* ofp);

    //--------------//
    // initializing //
    //--------------//

    int  FillMask(uchar fill_value);
    int  Step(float angle);

    //------------//
    // operations //
    //------------//

    int  LinearMap(float old_1, float new_1, float old_2, float new_2,
             float new_min, float new_max);
    int  Convolve(Image* image_1, Image* image_2);
    int  Gradient(int operator_size, Image* gx, Image* gy);
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
